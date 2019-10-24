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
#include "main.h"
#include "db.h"
#include "wallet.h"


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
    if (fHelp || params.size() < 1 || params.size() > 2)
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
  

    //if (!fGood) throw JSONRPCError(-5, "Invalid private key");
    if (pwalletMain->IsLocked())
        throw JSONRPCError(-13, "Error: Please enter the wallet passphrase with walletpassphrase first.");
    if (fWalletUnlockMintOnly) // slimcoin: no importprivkey in mint-only mode
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

        if (!pwalletMain->AddKey(key))
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

// Return average network hashes per second based on the last 'lookup' blocks,
// or from the last difficulty change if 'lookup' is nonpositive.
// If 'height' is nonnegative, compute the estimate at the time when a given block was found.
/*
Value GetNetworkHashPS(int lookup, int height) {
    CBlockIndex *pb = pindexBest;

    if (height >= 0 && height < nBestHeight)
        pb = FindBlockByHeight(height);

    if (pb == NULL || !pb->nHeight)
        return 0;

    // If lookup is -1, then use blocks since last difficulty change.
    if (lookup <= 0)
        lookup = pb->nHeight % 2016 + 1;

    // If lookup is larger than chain, then set it to chain length.
    if (lookup > pb->nHeight)
        lookup = pb->nHeight;

    CBlockIndex *pb0 = pb;
    int64 minTime = pb0->GetBlockTime();
    int64 maxTime = minTime;
    for (int i = 0; i < lookup; i++) {
        pb0 = pb0->pprev;
        int64 time = pb0->GetBlockTime();
        minTime = std::min(time, minTime);
        maxTime = std::max(time, maxTime);
    }

    // In case there's a situation where minTime == maxTime, we don't want a divide by zero exception.
    if (minTime == maxTime)
        return 0;

    // uint256 workDiff = pb->nChainWork - pb0->nChainWork;
    CBigNum workDiff = pb->bnChainTrust - pb0->bnChainTrust;
    int64 timeDiff = maxTime - minTime;

    return (boost::int64_t)(workDiff.getuint64() / timeDiff);
}

Value getnetworkhashps(const Array& params, bool fHelp)
{
    if (fHelp || params.size() > 2)
        throw runtime_error(
            "getnetworkhashps [blocks] [height]\n"
            "Returns the estimated network hashes per second based on the last 120 blocks.\n"
            "Pass in [blocks] to override # of blocks, -1 specifies since last difficulty change.\n"
            "Pass in [height] to estimate the network speed at the time when a certain block was found.");

    return GetNetworkHashPS(params.size() > 0 ? params[0].get_int() : 120, params.size() > 1 ? params[1].get_int() : -1);
}
*/
