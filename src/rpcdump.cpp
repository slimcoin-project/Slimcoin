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
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid Bitcoin address or script");
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

        // add to address book or update label
        if (address.IsValid())
            pwalletMain->SetAddressBook(address.Get(), strLabel, "receive");

        // Don't throw error in case an address is already there
        if (pwalletMain->HaveWatchOnly(script))
            return Value::null;

        pwalletMain->MarkDirty();
        pwalletMain->SetAddressBookName(dest, strLabel);
        // pwalletMain->SetAddressBook(dest, strLabel, "receive");

        if (!pwalletMain->AddWatchOnly(script))
            throw JSONRPCError(RPC_WALLET_ERROR, "Error adding address to wallet");

        if (fRescan)
        {
            pwalletMain->ScanForWalletTransactions(pindexGenesisBlock, true);
            pwalletMain->ReacceptWalletTransactions();
        }
    }

    return Value::null;
}
