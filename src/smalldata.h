// Copyright (c) 2014 The Fusioncoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef H_SMALL_DATA_FUSIONCOIN
#define H_SMALL_DATA_FUSIONCOIN

#include <string>
#include <vector>

#include <boost/foreach.hpp>
#include <boost/variant.hpp>

#include "keystore.h"
#include "bignum.h"

class CTransaction;

enum{
    SMALLDATA_TYPE_NULL,
    SMALLDATA_TYPE_PLAINTEXT,
    SMALLDATA_TYPE_BROADCAST,
};

const unsigned char *GetSmallDataHeader(int type);

bool GetTxMessage(CTransaction &tx, std::string &msg, bool &isBroadcast);

class CAdTx : public CTransaction
{
public:
    uint256 hashBlock;
    int nHeight;
    int64 nFee;

    // memory
    std::string adText;
    
    IMPLEMENT_SERIALIZE
    (
        nSerSize += SerReadWrite(s, *(CTransaction*)this, nType, nVersion, ser_action);
        READWRITE(hashBlock);
        READWRITE(nHeight);
        READWRITE(nFee);
    )

    CAdTx();
    CAdTx(const CTransaction& txIn);
    void GetText();
    void GetFee();
    int64 GetFeeCur();
};

class CAdManager{
private:
    mutable CCriticalSection cs;
public:
    uint256 lastBlockHash;
    int nLastHeight;
    std::vector<CAdTx> vAdList;

    IMPLEMENT_SERIALIZE
    (
        READWRITE(lastBlockHash);
        READWRITE(nLastHeight);
        READWRITE(vAdList);
    )

    CAdManager();
    void load();
    void save();
    bool getAdList(std::vector<CAdTx> &adTxList, int max = 15);
    void processBlock(CBlock* pblock, int height = 0);
    void addToList(CAdTx &adTx);
};

/** Access to the advertise database (ad.dat) */
class CAdDB
{
private:
    boost::filesystem::path pathAddr;
public:
    CAdDB();
    bool Write(const CAdManager& addr);
    bool Read(CAdManager& addr);
};

extern bool fAdEnabled;
extern CAdManager adManager;

#endif // H_SMALL_DATA_FUSIONCOIN

