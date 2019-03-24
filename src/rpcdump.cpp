// Copyright (c) 2009-2012 Bitcoin Developers
// Copyright (c) 2012-2013 The Peercoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

//#include <openssl/ecdsa.h>
#include <openssl/obj_mac.h>

#include "init.h" // for pwalletMain
#include "bitcoinrpc.h"
#include "ui_interface.h"
#include "base58.h"

#include <boost/lexical_cast.hpp>

#include "json/json_spirit_reader_template.h"
#include "json/json_spirit_writer_template.h"
#include "json/json_spirit_utils.h"


#define printf OutputDebugStringF

// using namespace boost::asio;
using namespace json_spirit;
using namespace std;

extern Object JSONRPCError(int code, const string& message);

class CTxDump
{
public:
  CBlockIndex *pindex;
  int64 nValue;
  bool fSpent;
  CWalletTx* ptx;
  int nOut;
  CTxDump(CWalletTx* ptx = NULL, int nOut = -1)
  {
    pindex = NULL;
    nValue = 0;
    fSpent = false;
    this->ptx = ptx;
    this->nOut = nOut;
  }
};

Value importpassphrase(const Array& params, bool fHelp)
{
  if(fHelp || params.size() < 1 || params.size() > 2)
    throw runtime_error(
      "importpassphrase \"<passphrase>\" [label]\n"
      "Adds a private key into your wallet.");
  string strSecret = params[0].get_str();
  uint256 pass = sha256((const u8int*)strSecret.c_str(), strSecret.length());
  CSecret passSecret;
  passSecret.resize(32);
  for (int i = 0; i < 32; i++) {
   passSecret[i] = (pass.begin())[i];
  }
  
  string strLabel = "";
  if(params.size() > 1)
    strLabel = params[1].get_str();

  CBitcoinSecret vchSecret;
  bool fCompressed = true;
  vchSecret.SetSecret( passSecret , fCompressed );
  

  //if(!fGood) throw JSONRPCError(-5, "Invalid private key");
  if(pwalletMain->IsLocked())
    throw JSONRPCError(-13, "Error: Please enter the wallet passphrase with walletpassphrase first.");
  if(fWalletUnlockMintOnly) // slimcoin: no importprivkey in mint-only mode
    throw JSONRPCError(-102, "Wallet is unlocked for minting only.");

  CKey key;
  CSecret secret = vchSecret.GetSecret(fCompressed);
  key.SetSecret(secret, fCompressed);
  /* FIXME: sanity check required */
  CBitcoinAddress vchAddress = CBitcoinAddress(key.GetPubKey().GetID());

  {
    LOCK2(cs_main, pwalletMain->cs_wallet);

    pwalletMain->MarkDirty();
    pwalletMain->SetAddressBookName(CTxDestination(vchAddress.Get()), strLabel);

    if(!pwalletMain->AddKey(key))
      throw JSONRPCError(-4,"Error adding key to wallet");

    pwalletMain->ScanForWalletTransactions(pindexGenesisBlock, true);
    pwalletMain->ReacceptWalletTransactions();
  }

  MainFrameRepaint();
  Object obj;
  obj.push_back(Pair("Secret",        CBitcoinSecret( passSecret, fCompressed ).ToString() ));
  obj.push_back(Pair("Address",       vchAddress.ToString()));
  obj.push_back(Pair("Hash",          pass.GetHex()));
  obj.push_back(Pair("Phrase",        strSecret));
  obj.push_back(Pair("Length",   (int)strSecret.length()));
  return obj;
  // return Value::null;
}

Value importprivkey(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 1 || params.size() > 3)
        throw runtime_error(
            "importprivkey <slimcoinprivkey> [label] [rescan=false]\n"
            "Adds a private key (as returned by dumpprivkey) to your wallet.");

    string strSecret = params[0].get_str();
    string strLabel = "";
    if (params.size() > 1)
        strLabel = params[1].get_str();
    CBitcoinSecret vchSecret;
    bool fGood = vchSecret.SetString(strSecret);

    // Whether to perform rescan after import
    bool fRescan = false;
    if (params.size() > 2)
        fRescan = params[2].get_bool();

    if (!fGood) throw JSONRPCError(-5,"Invalid private key");
    if (pwalletMain->IsLocked())
        throw JSONRPCError(-13, "Error: Please enter the wallet passphrase with walletpassphrase first.");
    if (fWalletUnlockMintOnly) // slimcoin: no importprivkey in mint-only mode
        throw JSONRPCError(-102, "Wallet is unlocked for minting only.");

    CKey key;
    bool fCompressed;
    CSecret secret = vchSecret.GetSecret(fCompressed);
    key.SetSecret(secret, fCompressed);
    CKeyID vchAddress = key.GetPubKey().GetID();
    {
        LOCK2(cs_main, pwalletMain->cs_wallet);

        pwalletMain->MarkDirty();
        pwalletMain->SetAddressBookName(vchAddress, strLabel);

        if (!pwalletMain->AddKey(key))
            throw JSONRPCError(-4,"Error adding key to wallet");

        if (fRescan) {
          pwalletMain->ScanForWalletTransactions(pindexGenesisBlock, true);
          pwalletMain->ReacceptWalletTransactions();
        }
    }

    MainFrameRepaint();

    return Value::null;
}

Value dumpprivkey(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 1)
        throw runtime_error(
      "dumpprivkey <slimcoinaddress>\n"
      "Reveals the private key corresponding to <slimcoinaddress>.");

    string strAddress = params[0].get_str();
    CBitcoinAddress address;
    if (!address.SetString(strAddress))
        throw JSONRPCError(-5, "Invalid slimcoin address");
    if (pwalletMain->IsLocked())
        throw JSONRPCError(-13, "Error: Please enter the wallet passphrase with walletpassphrase first.");
    if (fWalletUnlockMintOnly) // slimcoin: no dumpprivkey in mint-only mode
        throw JSONRPCError(-102, "Wallet is unlocked for minting only.");
    CKeyID keyID;
    if (!address.GetKeyID(keyID))
        throw JSONRPCError(-3, "Address does not refer to a key");
    CSecret vchSecret;
    bool fCompressed;
    if (!pwalletMain->GetSecret(keyID, vchSecret, fCompressed))
        throw JSONRPCError(-4,"Private key for address " + strAddress + " is not known");
    return CBitcoinSecret(vchSecret, fCompressed).ToString();
}

Value importaddress(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 1 || params.size() > 3)
        throw runtime_error(
            "importaddress <address> [label] [rescan=true]\n"
            "Adds an address or script (in hex) that can be watched as if it were in your wallet but cannot be used to spend.");

    CScript script;
    CBitcoinAddress address(params[0].get_str());

    if (address.IsValid()) {
        script.SetDestination(address.Get());
    } else if (IsHex(params[0].get_str())) {
        std::vector<unsigned char> data(ParseHex(params[0].get_str()));
        script = CScript(data.begin(), data.end());
    } else {
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid Slimcoin address or script");
    }

    CTxDestination dest;
    dest = address.Get();

    string strLabel = "";
    if (params.size() > 1)
        strLabel = params[1].get_str();

    // Whether to perform rescan after import
    bool fRescan = true;
    if (params.size() > 2)
        fRescan = params[2].get_bool();

    {
        LOCK2(cs_main, pwalletMain->cs_wallet);

        // Don't throw error in case an address is already there
        if (pwalletMain->HaveWatchOnly(dest))
            return Value::null;

        pwalletMain->MarkDirty();

         // add to address book or update label
        if (address.IsValid())
            // pwalletMain->SetAddressBook(address.Get(), strLabel, "receive");
            pwalletMain->SetAddressBookName(address.Get(), strLabel);

       if (!pwalletMain->AddWatchOnly(dest))
            throw JSONRPCError(RPC_WALLET_ERROR, "Error adding address to wallet");

        if (fRescan)
        {
            pwalletMain->ScanForWalletTransactions(pindexGenesisBlock, true);
            pwalletMain->ReacceptWalletTransactions();
        }
    }

    return Value::null;
}

Value ignoreaddress(const Array& params, bool fHelp)
{
    if(fHelp || (params.size() < 1) || (params.size() > 2))
        throw runtime_error(
            "ignoreaddress <address> [rescan]\n"
            "Removes an imported watch-only address or script (in hex)."
            "Block chain re-scanning is off (false) by default.\n");

    bool fRescan = false;
    if(params.size() > 1)
      fRescan = params[1].get_bool();

    CScript script;
    CBitcoinAddress address(params[0].get_str());

    if (address.IsValid()) {
        script.SetDestination(address.Get());
    } else if (IsHex(params[0].get_str())) {
        std::vector<unsigned char> data(ParseHex(params[0].get_str()));
        script = CScript(data.begin(), data.end());
    } else {
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid Slimcoin address or script");
    }

    CTxDestination dest;
    dest = address.Get();

    {
        LOCK2(cs_main, pwalletMain->cs_wallet);

        if (pwalletMain->HaveWatchOnly(dest))
        {
            if (!pwalletMain->RemoveWatchOnly(dest))
                throw JSONRPCError(RPC_WALLET_ERROR, "Error removing watch-only address from wallet");
            pwalletMain->MarkDirty();

            if (fRescan)
            {
                pwalletMain->ScanForWalletTransactions(pindexGenesisBlock, true);
                pwalletMain->ReacceptWalletTransactions();
            }
        }
    }

    return Value::null;
}

/*
Value importaddress(const Array &params, bool fHelp) {

    if(fHelp || (params.size() < 1) || (params.size() > 3))
      throw(runtime_error(
        "importaddress <address> [label] [rescan]\n"
        "Adds a watch only (unspendable) address to your wallet.\n"
        "P2PKH pubkey script in hex may be specified instead of the address.\n"
        "Block chain re-scanning is off (false) by default.\n"));

    string strLabel = "";
    if(params.size() > 1)
      strLabel = params[1].get_str();

    bool fRescan = false;
    if(params.size() > 2)
      fRescan = params[2].get_bool();

    CScript script;
    CBitcoinAddress addr;

    if(IsHex(params[0].get_str())) {
        std::vector<unsigned char> vchScriptPubKey(ParseHex(params[0].get_str()));
        if(vchScriptPubKey.size() != 25)
          throw(JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid P2PKH pubkey script"));
        script = CScript(vchScriptPubKey.begin(), vchScriptPubKey.end());
        // Copy the public key hash
        std::string strTemp = params[0].get_str().substr(6, 40);
        // Insert the Base58 prefix
        char prefix[1];
        sprintf(prefix, "%x", fTestNet ? 0x6F : 0x73);
        strTemp.insert(0, prefix);
        // Convert and encode
        std::vector<unsigned char> vchTemp(ParseHex(strTemp));
        addr = CBitcoinAddress(EncodeBase58Check(vchTemp));
    } else {
        CKeyID keyID;
        addr = CBitcoinAddress(params[0].get_str());
        if(!addr.GetKeyID(keyID))
          throw(JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid address"));
        script = GetScriptForPubKeyHash(keyID);
    }

    {
        LOCK2(cs_main, pwalletMain->cs_wallet);

        if(::IsMine(*pwalletMain, script) == MINE_SPENDABLE)
          throw(JSONRPCError(RPC_WALLET_ERROR, "The private key is already in the wallet"));

        if(pwalletMain->HaveWatchOnly(script))
          throw(JSONRPCError(RPC_WALLET_ERROR, "The address is being watched already"));

        pwalletMain->MarkDirty();

        if(addr.IsValid())
          pwalletMain->SetAddressBookName(addr.Get(), strLabel);

        if(!pwalletMain->AddWatchOnly(script))
          throw(JSONRPCError(RPC_WALLET_ERROR, "Failed to add the address to the wallet"));

        if(fRescan) {
            pwalletMain->ScanForWalletTransactions(pindexGenesisBlock, true);
            pwalletMain->ReacceptWalletTransactions();
        }

    }

    return(Value::null);
}

Value importpubkey(const Array &params, bool fHelp) {

    if(fHelp || (params.size() < 1) || (params.size() > 3))
      throw(runtime_error(
        "importpubkey <public key> [label] [rescan]\n"
        "Adds a watch only (unspendable) public key in hex to your wallet.\n"
        "Block chain re-scanning is off (false) by default.\n"));

    string strLabel = "";
    if(params.size() > 1)
      strLabel = params[1].get_str();

    bool fRescan = false;
    if(params.size() > 2)
      fRescan = params[2].get_bool();

    CScript script;
    CBitcoinAddress addr(params[0].get_str());

    if(!IsHex(params[0].get_str()))
      throw(JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Hex string expected for public key"));

    CPubKey pubKey(std::vector<unsigned char> (ParseHex(params[0].get_str())));
    if(!pubKey.IsValid())
      throw(JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid public key"));

    CKeyID keyID = pubKey.GetID();
    addr = CBitcoinAddress(keyID);
    script = GetScriptForPubKeyHash(keyID);

    {
        LOCK2(cs_main, pwalletMain->cs_wallet);

        if(::IsMine(*pwalletMain, script) == MINE_SPENDABLE)
          throw(JSONRPCError(RPC_WALLET_ERROR, "The private key is already in the wallet"));

        if(pwalletMain->HaveWatchOnly(script))
          throw(JSONRPCError(RPC_WALLET_ERROR, "The public key is being watched already"));

        pwalletMain->MarkDirty();

        if(addr.IsValid())
          pwalletMain->SetAddressBookName(addr.Get(), strLabel);

        if(!pwalletMain->AddWatchOnly(script))
          throw(JSONRPCError(RPC_WALLET_ERROR, "Failed to add the public key to the wallet"));

        if(fRescan) {
            pwalletMain->ScanForWalletTransactions(pindexGenesisBlock, true);
            pwalletMain->ReacceptWalletTransactions();
        }

    }

    return(Value::null);
}
*/

