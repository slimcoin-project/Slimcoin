// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2012 The Bitcoin developers
// Copyright (c) 2011-2013 The Peercoin developers
// Copyright (c) 2013-2014 The Slimcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_MAIN_H
#define BITCOIN_MAIN_H

#include "bignum.h"
#include "net.h"
#include "script.h"
#include <math.h>       /* pow */

#ifdef WIN32
#include <io.h> /* for _commit */
#endif

#include <list>

//the size of the block to hash, from the nVersion to the nNonce
#define HASH_PBLOCK_SIZE(pblock)  UEND(pblock->nNonce) - UBEGIN(pblock->nVersion)
#define HASH_BLOCK_SIZE(block)    UEND(block.nNonce) - UBEGIN(block.nVersion)

class CWallet;
class CBlock;
class CBlockIndex;
class CKeyItem;
class CReserveKey;
class COutPoint;
class CWalletTx;
class CTransaction;
class CTxOut;


class CAddress;
class CInv;
class CRequestTracker;
class CNode;

static const unsigned int MAX_BLOCK_SIZE = 1000000;
static const unsigned int MAX_BLOCK_SIZE_GEN = MAX_BLOCK_SIZE / 2;
static const unsigned int MAX_BLOCK_SIGOPS = MAX_BLOCK_SIZE / 50;
static const unsigned int MAX_ORPHAN_TRANSACTIONS = MAX_BLOCK_SIZE / 100;
static const int64 MIN_TX_FEE = CENT;
static const int64 MIN_RELAY_TX_FEE = CENT;
static const int64 MAX_MONEY = 250000000 * COIN;
static const int64 MAX_MINT_PROOF_OF_WORK = 50 * COIN;
static const int64 MAX_MINT_PROOF_OF_BURN = 250 * COIN;
static const int64 MIN_TXOUT_AMOUNT = MIN_TX_FEE;
inline bool MoneyRange(int64 nValue) { return (nValue >= 0 && nValue <= MAX_MONEY); }
static const int COINBASE_MATURITY_SLM = 500;
// Threshold for nLockTime: below this value it is interpreted as block number, otherwise as UNIX timestamp.
static const int LOCKTIME_THRESHOLD = 500000000; // Tue Nov  5 00:53:20 1985 UTC
static const int STAKE_TARGET_SPACING = 90; // 90 second block spacing 
static const int POB_TARGET_SPACING = 3;    // 3 PoW block spacing target between each PoB block
static const int STAKE_MIN_AGE = 60 * 60 * 24 * 7; // minimum age for coin age
static const int STAKE_MAX_AGE = 60 * 60 * 24 * 90; // stake age of full weight

#ifdef USE_UPNP
static const int fHaveUPnP = true;
#else
static const int fHaveUPnP = false;
#endif

static const uint256 hashGenesisBlockOfficial("0x00000766be5a4bb74c040b85a98d2ba2b433c5f4c673912b3331ea6f18d61bea");
static const uint256 hashGenesisBlockTestNet("0x00000d7e8a80fec4057cb6d560822705596040bf41f0ebb2465dcdf46e4c517e");

static const int64 nMaxClockDrift = 2 * 60 * 60;        // two hours

extern CScript COINBASE_FLAGS;






extern CCriticalSection cs_main;
extern std::map<uint256, CBlockIndex*> mapBlockIndex;
extern std::set<std::pair<COutPoint, unsigned int> > setStakeSeen;
extern std::set<std::pair<uint256, uint256> > setBurnSeen;
extern uint256 hashGenesisBlock;
extern unsigned int nStakeMinAge;
extern int nCoinbaseMaturity;
extern CBlockIndex* pindexGenesisBlock;
extern int nBestHeight;
extern CBigNum bnBestChainTrust;
extern CBigNum bnBestInvalidTrust;
extern uint256 hashBestChain;
extern CBlockIndex* pindexBest;
extern unsigned int nTransactionsUpdated;
extern uint64 nLastBlockTx;
extern uint64 nLastBlockSize;
extern int64 nLastCoinStakeSearchInterval;
extern const std::string strMessageMagic;
extern double dHashesPerSec;
extern int64 nHPSTimerStart;
extern int64 nTimeBestReceived;
extern CCriticalSection cs_setpwalletRegistered;
extern std::set<CWallet*> setpwalletRegistered;
extern std::map<uint256, CBlock*> mapOrphanBlocks;

// Settings
extern int64 nTransactionFee;

//////////////////////////////////////////////////////////////////////////////
/*                              Proof Of Burn                               */
//////////////////////////////////////////////////////////////////////////////

//Burn addresses
const CBitcoinAddress burnOfficialAddress("SfSLMCoinMainNetworkBurnAddr1DeTK5");
const CBitcoinAddress burnTestnetAddress("mmSLiMCoinTestnetBurnAddress1XU5fu");

#define BURN_CONSTANT      .01 * CENT

//The hash of a burnt tx doubles smoothly over the course of 350000.0 blocks, 
// do not change this without changing BURN_DECAY_RATE (keep floating point)
#define BURN_HASH_DOUBLE   350000.0 

//The growth rate of the hash (2 ** (1 / BURN_HASH_DOUBLE)) rounded up, 
// do not change this without changing BURN_HASH_DOUBLE
#define BURN_DECAY_RATE    1.00000198

//a burn tx requires atleast x >= 6 confirmations between it and the best block, BURN_MIN_CONFIMS must be > 0
#define BURN_MIN_CONFIRMS  6       
#define BURN_HARDER_TARGET 0.5     //make the burn target 0.5 times the intermediate calculated target

//keeps things safe
#if BURN_MIN_CONFIRMS < 1
#error BURN_MIN_CONFIRMS must be greater than or equal to 1
#endif

//PATCHES

//Rounds down the burn hash for all hashes after block 10500, not really needed though
// has became a legacy thing
#define BURN_ROUND_DOWN 10500

//at block 15936 and above, when checking burn hash equality in
// CBlock::CheckProofOfBurn, use the intermediate hash
#define BURN_INTERMEDIATE_HEIGHT 15936

inline bool use_burn_hash_intermediate(s32int nHeight)
{
  return nHeight >= BURN_INTERMEDIATE_HEIGHT ? true : false;
}

//Adjusts the trust values for PoW and PoB blocks
#define CHAINCHECKS_SWITCH_TIME          2403654400 //Sometime in the future

//Adjusts PoB and PoS targets
#define POB_POS_TARGET_SWITCH_TIME       2403654400 //Sometime in the future

//PATCHES

void SlimCoinAfterBurner(CWallet *pwallet);
bool HashBurnData(uint256 burnBlockHash, uint256 hashPrevBlock, uint256 burnTxHash,
                  s32int burnBlkHeight, int64 burnValue, uint256 &smallestHashRet, bool fRetIntermediate);
bool GetBurnHash(uint256 hashPrevBlock, s32int burnBlkHeight, s32int burnCTx,
                 s32int burnCTxOut, uint256 &smallestHashRet, bool fRetIntermediate);
bool GetAllTxClassesByIndex(s32int blkHeight, s32int txDepth, s32int txOutDepth, 
                            CBlock &blockRet, CTransaction &txRet, CTxOut &txOutRet);

//Scans all of the hashes of this transaction and returns the smallest one
bool ScanBurnHashes(const CWalletTx &burnWTx, uint256 &smallestHashRet);

//Applies ScanBurnHashes to all of the burnt hashes stored in the setBurnHashes
void HashAllBurntTx(uint256 &smallestHashRet, CWalletTx &smallestWTxRet);

//Given a number of burnt coins and their depth in the chain, returns the number of effectivly burnt coins
inline int64 BurnCalcEffectiveCoins(int64 nCoins, s32int depthInChain)
{
  return nCoins / pow(BURN_DECAY_RATE, depthInChain);
}

inline void GetBurnAddress(CBitcoinAddress &addressRet)
{
  addressRet = fTestNet ? burnTestnetAddress : burnOfficialAddress;
  return;
}

//the burn address, when created, the constuctor automatically assigns its value
class CBurnAddress : public CBitcoinAddress
{
public:

  CBurnAddress()
  {
    GetBurnAddress(*this);
  }
};

//if any == true, then compare address with both testnet and realnet burn addresses
// else compare only with the address that corresponds to which network the client is connected to
inline bool IsBurnAddress(const CBitcoinAddress &address, bool any=false)
{
  if(any)
    return address == burnTestnetAddress || address == burnOfficialAddress;
  else{
    CBurnAddress burnAddress;
    return address == burnAddress;
  }
    
}

//makes a uint256 number into its compact form and returns it as a uint256 again
inline const uint256 becomeCompact(const uint256 &num)
{
  //+ 1 at the end so that hashes that are originally above target, when
  // made compact, will still not make the limit, 
  //ex: hash   0x000000221312312 -> 0x00000022131000...01 //still does not make the target
  //    target 0x000000221310000...000
  return CBigNum().SetCompact(CBigNum(num).GetCompact()).getuint256() + 1;
}

//////////////////////////////////////////////////////////////////////////////
/*                              Proof Of Burn                               */
//////////////////////////////////////////////////////////////////////////////


class CReserveKey;
class CTxDB;
class CTxIndex;

void RegisterWallet(CWallet* pwalletIn);
void UnregisterWallet(CWallet* pwalletIn);
void SyncWithWallets(const CTransaction& tx, const CBlock* pblock=NULL, bool fUpdate=false, bool fConnect=true);
bool ProcessBlock(CNode* pfrom, CBlock* pblock);
bool CheckDiskSpace(uint64 nAdditionalBytes=0);
FILE* OpenBlockFile(unsigned int nFile, unsigned int nBlockPos, const char* pszMode="rb");
FILE* AppendBlockFile(unsigned int& nFileRet);
bool LoadBlockIndex(bool fAllowNew=true);
void PrintBlockTree();
bool ProcessMessages(CNode* pfrom);
bool SendMessages(CNode* pto, bool fSendTrickle);
void GenerateSlimcoins(bool fGenerate, CWallet* pwallet);
CBlock *CreateNewBlock(CWallet* pwallet, bool fProofOfStake=false, 
                       const CWalletTx *burnWalletTx=NULL, CReserveKey *resKey=NULL);
void IncrementExtraNonce(CBlock* pblock, CBlockIndex* pindexPrev, unsigned int& nExtraNonce);
void FormatHashBuffers(CBlock* pblock, char* pmidstate, char* pdata, char* phash1);
bool CheckWork(CBlock* pblock, CWallet& wallet, CReserveKey& reservekey);
bool CheckProofOfWork(uint256 hash, u32int nBits);
bool CheckProofOfBurnHash(uint256 hash, u32int nBurnBits);
int64 GetProofOfWorkReward(u32int nBits, bool fProofOfBurn=false);
int64 GetProofOfStakeReward(int64 nCoinAge, u32int nTime);
unsigned int ComputeMinWork(unsigned int nBase, int64 nTime);
int GetNumBlocksOfPeers();
bool IsInitialBlockDownload();
std::string GetWarnings(std::string strFor);
bool GetTransaction(const uint256 &hash, CTransaction &tx, uint256 &hashBlock);
uint256 WantedByOrphan(const CBlock* pblockOrphan);
const CBlockIndex* GetLastBlockIndex(const CBlockIndex* pindex, bool fProofOfStake);
void SlimCoinMiner(CWallet *pwallet, bool fProofOfStake);
CBlockIndex *pindexByHeight(s32int nHeight);

//Returns the number of proof of work blocks between (not including) the
// blocks with heights startHeight and endHeight
s32int nPoWBlocksBetween(s32int startHeight, s32int endHeight);






bool GetWalletFile(CWallet* pwallet, std::string &strWalletFileOut);

/** Position on disk for a particular transaction. */
class CDiskTxPos
{
public:
  unsigned int nFile;
  unsigned int nBlockPos;
  unsigned int nTxPos;

  CDiskTxPos()
  {
    SetNull();
  }

  CDiskTxPos(unsigned int nFileIn, unsigned int nBlockPosIn, unsigned int nTxPosIn)
  {
    nFile = nFileIn;
    nBlockPos = nBlockPosIn;
    nTxPos = nTxPosIn;
  }

  IMPLEMENT_SERIALIZE( READWRITE(FLATDATA(*this)); )
    void SetNull() { nFile = -1; nBlockPos = 0; nTxPos = 0; }
  bool IsNull() const { return (nFile == -1); }

  friend bool operator==(const CDiskTxPos& a, const CDiskTxPos& b)
  {
    return (a.nFile     == b.nFile &&
            a.nBlockPos == b.nBlockPos &&
            a.nTxPos    == b.nTxPos);
  }

  friend bool operator!=(const CDiskTxPos& a, const CDiskTxPos& b)
  {
    return !(a == b);
  }

  std::string ToString() const
  {
    if (IsNull())
      return "null";
    else
      return strprintf("(nFile=%d, nBlockPos=%d, nTxPos=%d)", nFile, nBlockPos, nTxPos);
  }

  void print() const
  {
    printf("%s", ToString().c_str());
  }
};



/** An inpoint - a combination of a transaction and an index n into its vin */
class CInPoint
{
public:
  CTransaction* ptx;
  unsigned int n;

  CInPoint() { SetNull(); }
  CInPoint(CTransaction* ptxIn, unsigned int nIn) { ptx = ptxIn; n = nIn; }
  void SetNull() { ptx = NULL; n = -1; }
  bool IsNull() const { return (ptx == NULL && n == -1); }
};



/** An outpoint - a combination of a transaction hash and an index n into its vout */
class COutPoint
{
public:
  uint256 hash;
  unsigned int n;

  COutPoint() { SetNull(); }
  COutPoint(uint256 hashIn, unsigned int nIn) { hash = hashIn; n = nIn; }
  IMPLEMENT_SERIALIZE( READWRITE(FLATDATA(*this)); )
    void SetNull() { hash = 0; n = -1; }
  bool IsNull() const { return (hash == 0 && n == -1); }

  friend bool operator<(const COutPoint& a, const COutPoint& b)
  {
    return (a.hash < b.hash || (a.hash == b.hash && a.n < b.n));
  }

  friend bool operator==(const COutPoint& a, const COutPoint& b)
  {
    return (a.hash == b.hash && a.n == b.n);
  }

  friend bool operator!=(const COutPoint& a, const COutPoint& b)
  {
    return !(a == b);
  }

  std::string ToString() const
  {
    //~ return strprintf("COutPoint(%s, %d)", hash.ToString().substr(0,10).c_str(), n);
    return strprintf("COutPoint(%s, %d)", hash.ToString().c_str(), n);
  }

  void print() const
  {
    printf("%s\n", ToString().c_str());
  }
};




/** An input of a transaction.  It contains the location of the previous
 * transaction's output that it claims and a signature that matches the
 * output's public key.
 */
class CTxIn
{
public:
  COutPoint prevout;
  CScript scriptSig;
  unsigned int nSequence;

  CTxIn()
  {
    nSequence = std::numeric_limits<unsigned int>::max();
  }

  explicit CTxIn(COutPoint prevoutIn, CScript scriptSigIn=CScript(), 
                 unsigned int nSequenceIn=std::numeric_limits<unsigned int>::max())
  {
    prevout = prevoutIn;
    scriptSig = scriptSigIn;
    nSequence = nSequenceIn;
  }

  CTxIn(uint256 hashPrevTx, unsigned int nOut, CScript scriptSigIn=CScript(), 
        unsigned int nSequenceIn=std::numeric_limits<unsigned int>::max())
  {
    prevout = COutPoint(hashPrevTx, nOut);
    scriptSig = scriptSigIn;
    nSequence = nSequenceIn;
  }

  IMPLEMENT_SERIALIZE
    (
      READWRITE(prevout);
      READWRITE(scriptSig);
      READWRITE(nSequence);
      )

    bool IsFinal() const
  {
    return (nSequence == std::numeric_limits<unsigned int>::max());
  }

  friend bool operator==(const CTxIn& a, const CTxIn& b)
  {
    return (a.prevout   == b.prevout &&
            a.scriptSig == b.scriptSig &&
            a.nSequence == b.nSequence);
  }

  friend bool operator!=(const CTxIn& a, const CTxIn& b)
  {
    return !(a == b);
  }

  std::string ToStringShort() const
  {
    return strprintf(" %s %d", prevout.hash.ToString().c_str(), prevout.n);
  }

  std::string ToString() const
  {
    std::string str;
    str += "CTxIn(";
    str += prevout.ToString();
    if (prevout.IsNull())
      str += strprintf(", coinbase %s", HexStr(scriptSig).c_str());
    else
      str += strprintf(", scriptSig=%s", scriptSig.ToString().substr(0,24).c_str());
    if (nSequence != std::numeric_limits<unsigned int>::max())
      str += strprintf(", nSequence=%u", nSequence);
    str += ")";
    return str;
  }

  void print() const
  {
    printf("%s\n", ToString().c_str());
  }
};




/** An output of a transaction.  It contains the public key that the next input
 * must be able to sign with to claim it.
 */
class CTxOut
{
public:
  int64 nValue;
  CScript scriptPubKey;

  CTxOut()
  {
    SetNull();
  }

  CTxOut(int64 nValueIn, CScript scriptPubKeyIn)
  {
    nValue = nValueIn;
    scriptPubKey = scriptPubKeyIn;
  }

  IMPLEMENT_SERIALIZE
    (
      READWRITE(nValue);
      READWRITE(scriptPubKey);
      )

    void SetNull()
  {
    nValue = -1;
    scriptPubKey.clear();
  }

  bool IsNull()
  {
    return (nValue == -1);
  }

  void SetEmpty()
  {
    nValue = 0;
    scriptPubKey.clear();
  }

  bool IsEmpty() const
  {
    return (nValue == 0 && scriptPubKey.empty());
  }

  uint256 GetHash() const
  {
    return SerializeHash(*this);
  }

  friend bool operator==(const CTxOut& a, const CTxOut& b)
  {
    return (a.nValue       == b.nValue &&
            a.scriptPubKey == b.scriptPubKey);
  }

  friend bool operator!=(const CTxOut& a, const CTxOut& b)
  {
    return !(a == b);
  }

  std::string ToStringShort() const
  {
    return strprintf(" out %s %s", FormatMoney(nValue).c_str(), scriptPubKey.ToString(true).c_str());
  }

  std::string ToString() const
  {
    if (IsEmpty()) return "CTxOut(empty)";
    if (scriptPubKey.size() < 6)
      return "CTxOut(error)";

    return strprintf("CTxOut(nValue=%s, scriptPubKey=%s)", FormatMoney(nValue).c_str(), scriptPubKey.ToString().c_str());
  }

  void print() const
  {
    printf("%s\n", ToString().c_str());
  }
};




enum GetMinFee_mode
{
  GMF_BLOCK,
  GMF_RELAY,
  GMF_SEND,
};

typedef std::map<uint256, std::pair<CTxIndex, CTransaction> > MapPrevTx;

/** The basic transaction that is broadcasted on the network and contained in
 * blocks.  A transaction can contain multiple inputs and outputs.
 */
class CTransaction
{
public:
  int nVersion;
  unsigned int nTime;
  std::vector<CTxIn> vin;
  std::vector<CTxOut> vout;
  unsigned int nLockTime;

  // Denial-of-service detection:
  mutable int nDoS;
  bool DoS(int nDoSIn, bool fIn) const { nDoS += nDoSIn; return fIn; }

  CTransaction()
  {
    SetNull();
  }

  IMPLEMENT_SERIALIZE
    (
      READWRITE(this->nVersion);
      nVersion = this->nVersion;
      READWRITE(nTime);
      READWRITE(vin);
      READWRITE(vout);
      READWRITE(nLockTime);
      )

    void SetNull()
  {
    nVersion = 1;
    nTime = GetAdjustedTime();
    vin.clear();
    vout.clear();
    nLockTime = 0;
    nDoS = 0;  // Denial-of-service prevention
  }

  bool IsNull() const
  {
    return (vin.empty() && vout.empty());
  }

  uint256 GetHash() const
  {
    return SerializeHash(*this);
  }

  bool IsFinal(int nBlockHeight=0, int64 nBlockTime=0) const
  {
    // Time based nLockTime implemented in 0.1.6
    if(!nLockTime)
      return true;

    if(!nBlockHeight)
      nBlockHeight = nBestHeight;

    if(!nBlockTime)
      nBlockTime = GetAdjustedTime();

    if((int64)nLockTime < ((int64)nLockTime < LOCKTIME_THRESHOLD ? (int64)nBlockHeight : nBlockTime))
      return true;

    BOOST_FOREACH(const CTxIn& txin, vin)
      if(!txin.IsFinal())
        return false;

    return true;
  }

  bool IsNewerThan(const CTransaction& old) const
  {
    if (vin.size() != old.vin.size())
      return false;
    for (unsigned int i = 0; i < vin.size(); i++)
      if (vin[i].prevout != old.vin[i].prevout)
        return false;

    bool fNewer = false;
    unsigned int nLowest = std::numeric_limits<unsigned int>::max();
    for (unsigned int i = 0; i < vin.size(); i++)
    {
      if (vin[i].nSequence != old.vin[i].nSequence)
      {
        if (vin[i].nSequence <= nLowest)
        {
          fNewer = false;
          nLowest = vin[i].nSequence;
        }
        if (old.vin[i].nSequence < nLowest)
        {
          fNewer = true;
          nLowest = old.vin[i].nSequence;
        }
      }
    }
    return fNewer;
  }

  bool IsCoinBase() const
  {
    return (vin.size() == 1 && vin[0].prevout.IsNull() && vout.size() >= 1);
  }

  bool IsCoinStake() const
  {
    // slimcoin: the coin stake transaction is marked with the first output empty
    return (vin.size() > 0 && (!vin[0].prevout.IsNull()) && vout.size() >= 2 && vout[0].IsEmpty());
  }

  /** Check for standard transaction types
      @return True if all outputs (scriptPubKeys) use only standard transaction forms
  */
  bool IsStandard() const;

  /** Check for standard transaction types
      @param[in] mapInputs	Map of previous transactions that have outputs we're spending
      @return True if all inputs (scriptSigs) use only standard transaction forms
      @see CTransaction::FetchInputs
  */
  bool AreInputsStandard(const MapPrevTx& mapInputs) const;

  /** Count ECDSA signature operations the old-fashioned (pre-0.6) way
      @return number of sigops this transaction's outputs will produce when spent
      @see CTransaction::FetchInputs
  */
  unsigned int GetLegacySigOpCount() const;

  /** Count ECDSA signature operations in pay-to-script-hash inputs.

      @param[in] mapInputs	Map of previous transactions that have outputs we're spending
      @return maximum number of sigops required to validate this transaction's inputs
      @see CTransaction::FetchInputs
  */
  unsigned int GetP2SHSigOpCount(const MapPrevTx& mapInputs) const;

  /** Amount of bitcoins spent by this transaction.
      @return sum of all outputs (note: does not include fees)
  */
  int64 GetValueOut() const
  {
    int64 nValueOut = 0;
    BOOST_FOREACH(const CTxOut& txout, vout)
    {
      nValueOut += txout.nValue;
      if (!MoneyRange(txout.nValue) || !MoneyRange(nValueOut))
        throw std::runtime_error("CTransaction::GetValueOut() : value out of range");
    }
    return nValueOut;
  }

  //Returns the pubKet of the first txIn of this tx
  bool GetSendersPubKey(CScript &scriptPubKeyRet, bool fOurPubKey=false) const;

  //return the index of a burn transaction in vout, -1 if not found
  s32int GetBurnOutTxIndex() const
  {
    //find the burnt transaction
    CBurnAddress burnAddress;

    u32int i;
    for(i = 0; i < vout.size(); i++)
    {
      CBitcoinAddress address;
      if(!ExtractAddress(vout[i].scriptPubKey, address))
        continue;

      if(address == burnAddress)
        break;
    }

    //if i hit the end for the for loop, it means it found nothing so return -1
    return i == vout.size() ? -1 : i;
  }
  
  bool IsBurnTx() const
  {
    return GetBurnOutTxIndex() == -1 ? false : true;
  }

  //if (return value).IsNull() == true, then some error occured
  CTxOut GetBurnOutTx() const
  {
    s32int burnTxOutIndex = GetBurnOutTxIndex();
    if(burnTxOutIndex == -1)
      return CTxOut(); //error

    return vout[burnTxOutIndex];
  }

  /** Amount of bitcoins coming in to this transaction
      Note that lightweight clients may not know anything besides the hash of previous transactions,
      so may not be able to calculate this.

      @param[in] mapInputs	Map of previous transactions that have outputs we're spending
      @return	Sum of value of all inputs (scriptSigs)
      @see CTransaction::FetchInputs
  */
  int64 GetValueIn(const MapPrevTx& mapInputs) const;

  static bool AllowFree(double dPriority)
  {
    // Large (in bytes) low-priority (new, small-coin) transactions
    // need a fee.
    return dPriority > COIN * 960 / 250; //960 blocks per day
  }

  int64 GetMinFee(unsigned int nBlockSize=1, bool fAllowFree=false, enum GetMinFee_mode mode=GMF_BLOCK) const
  {
    // Base fee is either MIN_TX_FEE or MIN_RELAY_TX_FEE
    int64 nBaseFee = (mode == GMF_RELAY) ? MIN_RELAY_TX_FEE : MIN_TX_FEE;

    unsigned int nBytes = ::GetSerializeSize(*this, SER_NETWORK, PROTOCOL_VERSION);
    unsigned int nNewBlockSize = nBlockSize + nBytes;
    int64 nMinFee = (1 + (int64)nBytes / 1000) * nBaseFee;

    if (fAllowFree)
    {
      if (nBlockSize == 1)
      {
        // Transactions under 10K are free
        // (about 4500bc if made of 50bc inputs)
        if (nBytes < 10000)
          nMinFee = 0;
      }
      else
      {
        // Free transaction area
        if (nNewBlockSize < 27000)
          nMinFee = 0;
      }
    }

    // To limit dust spam, require MIN_TX_FEE/MIN_RELAY_TX_FEE if any output is less than 0.01
    if (nMinFee < nBaseFee)
    {
      BOOST_FOREACH(const CTxOut& txout, vout)
        if (txout.nValue < CENT)
          nMinFee = nBaseFee;
    }

    // Raise the price as the block approaches full
    if (nBlockSize != 1 && nNewBlockSize >= MAX_BLOCK_SIZE_GEN/2)
    {
      if (nNewBlockSize >= MAX_BLOCK_SIZE_GEN)
        return MAX_MONEY;
      nMinFee *= MAX_BLOCK_SIZE_GEN / (MAX_BLOCK_SIZE_GEN - nNewBlockSize);
    }

    if (!MoneyRange(nMinFee))
      nMinFee = MAX_MONEY;
    return nMinFee;
  }


  bool ReadFromDisk(CDiskTxPos pos, FILE** pfileRet=NULL)
  {
    CAutoFile filein = CAutoFile(OpenBlockFile(pos.nFile, 0, pfileRet ? "rb+" : "rb"), SER_DISK, CLIENT_VERSION);
    if (!filein)
      return error("CTransaction::ReadFromDisk() : OpenBlockFile failed");

    // Read transaction
    if (fseek(filein, pos.nTxPos, SEEK_SET) != 0)
      return error("CTransaction::ReadFromDisk() : fseek failed");

    try {
      filein >> *this;
    }
    catch (std::exception &e) {
      return error("%s() : deserialize or I/O error", __PRETTY_FUNCTION__);
    }

    // Return file pointer
    if (pfileRet)
    {
      if (fseek(filein, pos.nTxPos, SEEK_SET) != 0)
        return error("CTransaction::ReadFromDisk() : second fseek failed");
      *pfileRet = filein.release();
    }
    return true;
  }

  friend bool operator==(const CTransaction& a, const CTransaction& b)
  {
    return (a.nVersion  == b.nVersion &&
            a.nTime     == b.nTime &&
            a.vin       == b.vin &&
            a.vout      == b.vout &&
            a.nLockTime == b.nLockTime);
  }

  friend bool operator!=(const CTransaction& a, const CTransaction& b)
  {
    return !(a == b);
  }


  std::string ToStringShort() const
  {
    std::string str;
    str += strprintf("%s %s", GetHash().ToString().c_str(), IsCoinBase()? "base" : (IsCoinStake()? "stake" : "user"));
    return str;
  }

  std::string ToString() const
  {
    std::string str;
    str += IsCoinBase()? "Coinbase" : (IsCoinStake()? "Coinstake" : "CTransaction");
    str += strprintf("(hash=%s, nTime=%d, ver=%d, vin.size=%d, vout.size=%d, nLockTime=%d)\n",
                     GetHash().ToString().substr(0,10).c_str(),
                     nTime,
                     nVersion,
                     vin.size(),
                     vout.size(),
                     nLockTime);

    for (unsigned int i = 0; i < vin.size(); i++)
      str += "    " + vin[i].ToString() + "\n";
    for (unsigned int i = 0; i < vout.size(); i++)
      str += "    " + vout[i].ToString() + "\n";
    return str;
  }

  void print() const
  {
    printf("%s", ToString().c_str());
  }


  bool ReadFromDisk(CTxDB& txdb, COutPoint prevout, CTxIndex& txindexRet);
  bool ReadFromDisk(CTxDB& txdb, COutPoint prevout);
  bool ReadFromDisk(COutPoint prevout);
  bool DisconnectInputs(CTxDB& txdb);

  /** Fetch from memory and/or disk. inputsRet keys are transaction hashes.

      @param[in] txdb	Transaction database
      @param[in] mapTestPool	List of pending changes to the transaction index database
      @param[in] fBlock	True if being called to add a new best-block to the chain
      @param[in] fMiner	True if being called by CreateNewBlock
      @param[out] inputsRet	Pointers to this transaction's inputs
      @param[out] fInvalid	returns true if transaction is invalid
      @return	Returns true if all inputs are in txdb or mapTestPool
  */
  bool FetchInputs(CTxDB& txdb, const std::map<uint256, CTxIndex>& mapTestPool,
                   bool fBlock, bool fMiner, MapPrevTx& inputsRet, bool& fInvalid);

  /** Sanity check previous transactions, then, if all checks succeed,
      mark them as spent by this transaction.

      @param[in] inputs	Previous transactions (from FetchInputs)
      @param[out] mapTestPool	Keeps track of inputs that need to be updated on disk
      @param[in] posThisTx	Position of this transaction on disk
      @param[in] pindexBlock
      @param[in] fBlock	true if called from ConnectBlock
      @param[in] fMiner	true if called from CreateNewBlock
      @param[in] fStrictPayToScriptHash	true if fully validating p2sh transactions
      @return Returns true if all checks succeed
  */
  bool ConnectInputs(CTxDB& txdb, MapPrevTx inputs,
                     std::map<uint256, CTxIndex>& mapTestPool, const CDiskTxPos& posThisTx,
                     const CBlockIndex* pindexBlock, bool fBlock, bool fMiner, bool fStrictPayToScriptHash=true);
  bool ClientConnectInputs();
  bool CheckTransaction() const;
  bool AcceptToMemoryPool(CTxDB& txdb, bool fCheckInputs=true, bool* pfMissingInputs=NULL);
  bool GetCoinAge(CTxDB& txdb, uint64& nCoinAge) const;  // slimcoin: get transaction coin age

protected:
  const CTxOut& GetOutputFor(const CTxIn& input, const MapPrevTx& inputs) const;
};





/** A transaction with a merkle branch linking it to the block chain. */
class CMerkleTx : public CTransaction
{
public:
  uint256 hashBlock;
  std::vector<uint256> vMerkleBranch;
  int nIndex;

  // memory only
  mutable bool fMerkleVerified;


  CMerkleTx()
  {
    Init();
  }

CMerkleTx(const CTransaction& txIn) : CTransaction(txIn)
  {
    Init();
  }

  void Init()
  {
    hashBlock = 0;
    nIndex = -1;
    fMerkleVerified = false;
  }


  IMPLEMENT_SERIALIZE
    (
      nSerSize += SerReadWrite(s, *(CTransaction*)this, nType, nVersion, ser_action);
      nVersion = this->nVersion;
      READWRITE(hashBlock);
      READWRITE(vMerkleBranch);
      READWRITE(nIndex);
      )


    int SetMerkleBranch(const CBlock* pblock=NULL);
  int GetDepthInMainChain(CBlockIndex* &pindexRet) const;
  int GetDepthInMainChain() const { CBlockIndex *pindexRet; return GetDepthInMainChain(pindexRet); }
  bool IsInMainChain() const { return GetDepthInMainChain() > 0; }
  int GetBlocksToMaturity() const;
  bool AcceptToMemoryPool(CTxDB& txdb, bool fCheckInputs=true);
  bool AcceptToMemoryPool();

  bool IsBurnTxMature() const;
  s32int GetBurnDepthInMainChain() const;
};




/**  A txdb record that contains the disk location of a transaction and the
 * locations of transactions that spend its outputs.  vSpent is really only
 * used as a flag, but having the location is very helpful for debugging.
 */
class CTxIndex
{
public:
  CDiskTxPos pos;
  std::vector<CDiskTxPos> vSpent;

  CTxIndex()
  {
    SetNull();
  }

  CTxIndex(const CDiskTxPos& posIn, unsigned int nOutputs)
  {
    pos = posIn;
    vSpent.resize(nOutputs);
  }

  IMPLEMENT_SERIALIZE
    (
      if (!(nType & SER_GETHASH))
        READWRITE(nVersion);
      READWRITE(pos);
      READWRITE(vSpent);
      )

    void SetNull()
  {
    pos.SetNull();
    vSpent.clear();
  }

  bool IsNull()
  {
    return pos.IsNull();
  }

  friend bool operator==(const CTxIndex& a, const CTxIndex& b)
  {
    return (a.pos    == b.pos &&
            a.vSpent == b.vSpent);
  }

  friend bool operator!=(const CTxIndex& a, const CTxIndex& b)
  {
    return !(a == b);
  }
  int GetDepthInMainChain() const;
 
};





/** Nodes collect new transactions into a block, hash them into a hash tree,
 * and scan through nonce values to make the block's hash satisfy proof-of-work
 * requirements.  When they solve the proof-of-work, they broadcast the block
 * to everyone and the block is added to the block chain.  The first transaction
 * in the block is a special one that creates a new coin owned by the creator
 * of the block.
 *
 * Blocks are appended to blk0001.dat files on disk.  Their location on disk
 * is indexed by CBlockIndex objects in memory.
 */
class CBlock
{
public:
  // header
  int nVersion;
  uint256 hashPrevBlock;
  uint256 hashMerkleRoot;
  unsigned int nTime;
  unsigned int nBits;
  unsigned int nNonce;

  // Proof-of-Burn switch, indexes, and values
  bool fProofOfBurn;
  uint256 hashBurnBlock;//in case there was a fork, used to check if the burn coords point to the block intended
  uint256 burnHash;     //used for DoS detection
  s32int burnBlkHeight; //the height the block containing the burn tx is found
  s32int burnCTx;       //the index in vtx of the burn tx
  s32int burnCTxOut;    //the index in the burn tx's vout to the burnt coins output
  int64 nEffectiveBurnCoins;
  u32int nBurnBits;

  // network and disk
  std::vector<CTransaction> vtx;

  // slimcoin: block signature - signed by coin base txout[0]'s owner
  std::vector<unsigned char> vchBlockSig;

  // memory only
  mutable std::vector<uint256> vMerkleTree;

  // Denial-of-service detection:
  mutable int nDoS;
  bool DoS(int nDoSIn, bool fIn) const { nDoS += nDoSIn; return fIn; }

  CBlock()
  {
    SetNull();
  }

  IMPLEMENT_SERIALIZE
    (
      //The block header
      READWRITE(this->nVersion);
      nVersion = this->nVersion;
      READWRITE(hashPrevBlock);
      READWRITE(hashMerkleRoot);
      READWRITE(nTime);
      READWRITE(nBits);
      READWRITE(nNonce);

      //PoB data
      READWRITE(fProofOfBurn);
      READWRITE(hashBurnBlock);
      READWRITE(burnHash);
      READWRITE(burnBlkHeight);
      READWRITE(burnCTx);
      READWRITE(burnCTxOut);
      READWRITE(nEffectiveBurnCoins);
      READWRITE(nBurnBits);

      // ConnectBlock depends on vtx following header to generate CDiskTxPos
      if(!(nType & (SER_GETHASH | SER_BLOCKHEADERONLY)))
      {
        READWRITE(vtx);
        READWRITE(vchBlockSig);
      }else if(fRead)
      {
        const_cast<CBlock*>(this)->vtx.clear();
        const_cast<CBlock*>(this)->vchBlockSig.clear();
      }
      
      )

    void SetNull()
  {
    nVersion = 1;
    hashPrevBlock = 0;
    hashMerkleRoot = 0;
    nTime = 0;
    nBits = 0;
    nNonce = 0;

    //proof-of-burn defaults
    fProofOfBurn = false;
    hashBurnBlock = 0;
    burnHash = 0;
    burnBlkHeight = burnCTx = burnCTxOut = -1; //set indexes to negative
    nEffectiveBurnCoins = 0;
    nBurnBits = 0;

    vtx.clear();
    vchBlockSig.clear();
    vMerkleTree.clear();
    nDoS = 0;
  }

  bool IsNull() const
  {
    return (!nBits);
  }

  uint256 GetHash() const
  {
    return DcryptHash(BEGIN(nVersion), END(nNonce));
  }

  //PoB
  uint256 GetBurnHash(bool fRetIntermediate) const
  {
    if(!IsProofOfBurn())
      return ~uint256(0);

    uint256 hashRet;
    if(!::GetBurnHash(hashPrevBlock, burnBlkHeight, burnCTx, burnCTxOut, hashRet, fRetIntermediate))
      return ~uint256(0);

    return hashRet;
  }
  
  //check this block's coinbase public key signature with that of the given transaction index
  bool BurnCheckPubKeys(s32int blkHeight, s32int txDepth, s32int txOutDepth) const
  {
    CBlock indexBlock;
    CTransaction indexTx;
    CTxOut indexTxOut;
    if(!GetAllTxClassesByIndex(blkHeight, txDepth, txOutDepth, indexBlock, indexTx, indexTxOut))
      return false;

    CScript indexTxScript;
    if(!indexTx.GetSendersPubKey(indexTxScript))
      return false;

    //compare the block's coinbase's script with the burn transaction's sender's script
    return vtx[0].vout[0].scriptPubKey.comparePubKeySignature(indexTxScript);
  }

  bool CheckBurnEffectiveCoins(int64 *calcEffCoinsRet = NULL) const;

  bool CheckProofOfBurn() const;

  //PoB

  int64 GetBlockTime() const
  {
    return (int64)nTime;
  }

  void UpdateTime(const CBlockIndex* pindexPrev);

  // slimcoin: entropy bit for stake modifier if chosen by modifier
  unsigned int GetStakeEntropyBit() const
  {
    uint160 hashSig = Hash160(vchBlockSig);

    if(fDebug && GetBoolArg("-printstakemodifier"))
      printf("GetStakeEntropyBit: hashSig=%s", hashSig.ToString().c_str());

    hashSig >>= 159; // take the first bit of the hash

    if(fDebug && GetBoolArg("-printstakemodifier"))
      printf(" entropybit=%d\n", hashSig.Get64());

    return hashSig.Get64();
  }

  // slimcoin: three types of block: proof-of-work, proof-of-stake, or proof-of-burn
  bool IsProofOfStake() const
  {
    return (vtx.size() > 1 && vtx[1].IsCoinStake());
  }

  bool IsProofOfBurn() const
  {
    return fProofOfBurn && burnBlkHeight >= 0 && burnCTx >= 0 && burnCTxOut >= 0;
  }

  bool IsProofOfWork() const
  {
    return !IsProofOfBurn() && !IsProofOfStake();
  }

  std::pair<COutPoint, unsigned int> GetProofOfStake() const
  {
    return IsProofOfStake() ? std::make_pair(vtx[1].vin[0].prevout, vtx[1].nTime) : 
      std::make_pair(COutPoint(), (unsigned int)0);
  }

  std::pair<uint256, uint256> GetProofOfBurn() const
  {
    return std::make_pair(burnHash, hashPrevBlock);
  }

  // slimcoin: get max transaction timestamp
  int64 GetMaxTransactionTime() const
  {
    int64 maxTransactionTime = 0;
    BOOST_FOREACH(const CTransaction& tx, vtx)
      maxTransactionTime = std::max(maxTransactionTime, (int64)tx.nTime);
    return maxTransactionTime;
  }

  uint256 BuildMerkleTree() const
  {
    vMerkleTree.clear();
    BOOST_FOREACH(const CTransaction& tx, vtx)
      vMerkleTree.push_back(tx.GetHash());
    int j = 0;
    for (int nSize = vtx.size(); nSize > 1; nSize = (nSize + 1) / 2)
    {
      for (int i = 0; i < nSize; i += 2)
      {
        int i2 = std::min(i+1, nSize-1);
        vMerkleTree.push_back(Hash(BEGIN(vMerkleTree[j+i]),  END(vMerkleTree[j+i]),
                                   BEGIN(vMerkleTree[j+i2]), END(vMerkleTree[j+i2])));
      }
      j += nSize;
    }
    return (vMerkleTree.empty() ? 0 : vMerkleTree.back());
  }

  std::vector<uint256> GetMerkleBranch(int nIndex) const
  {
    if (vMerkleTree.empty())
      BuildMerkleTree();
    std::vector<uint256> vMerkleBranch;
    int j = 0;
    for (int nSize = vtx.size(); nSize > 1; nSize = (nSize + 1) / 2)
    {
      int i = std::min(nIndex^1, nSize-1);
      vMerkleBranch.push_back(vMerkleTree[j+i]);
      nIndex >>= 1;
      j += nSize;
    }
    return vMerkleBranch;
  }

  static uint256 CheckMerkleBranch(uint256 hash, const std::vector<uint256>& vMerkleBranch, int nIndex)
  {
    if (nIndex == -1)
      return 0;
    BOOST_FOREACH(const uint256& otherside, vMerkleBranch)
    {
      if (nIndex & 1)
        hash = Hash(BEGIN(otherside), END(otherside), BEGIN(hash), END(hash));
      else
        hash = Hash(BEGIN(hash), END(hash), BEGIN(otherside), END(otherside));
      nIndex >>= 1;
    }
    return hash;
  }


  bool WriteToDisk(unsigned int& nFileRet, unsigned int& nBlockPosRet)
  {
    // Open history file to append
    CAutoFile fileout = CAutoFile(AppendBlockFile(nFileRet), SER_DISK, CLIENT_VERSION);
    if(!fileout)
      return error("CBlock::WriteToDisk() : AppendBlockFile failed");

    // Write index header
    unsigned char pchMessageStart[4];
    GetMessageStart(pchMessageStart, true);
    unsigned int nSize = fileout.GetSerializeSize(*this);
    fileout << FLATDATA(pchMessageStart) << nSize;

    // Write block
    long fileOutPos = ftell(fileout);
    if(fileOutPos < 0)
      return error("CBlock::WriteToDisk() : ftell failed");

    nBlockPosRet = fileOutPos;
    fileout << *this;

    // Flush stdio buffers and commit to disk before returning
    fflush(fileout);

    if(!IsInitialBlockDownload() || (nBestHeight+1) % 500 == 0)
    {
#ifdef WIN32
      _commit(_fileno(fileout));
#else
      fsync(fileno(fileout));
#endif
    }

    return true;
  }

  bool ReadFromDisk(unsigned int nFile, unsigned int nBlockPos, bool fReadTransactions=true)
  {
    SetNull();

    // Open history file to read
    CAutoFile filein = CAutoFile(OpenBlockFile(nFile, nBlockPos, "rb"), SER_DISK, CLIENT_VERSION);
    if (!filein)
      return error("CBlock::ReadFromDisk() : OpenBlockFile failed");
    if(!fReadTransactions)
      filein.nType |= SER_BLOCKHEADERONLY;

    // Read block
    try {
      filein >> *this;
    }
    catch (std::exception &e) {
      return error("%s() : deserialize or I/O error", __PRETTY_FUNCTION__);
    }

    // Check the header
    if(fReadTransactions && IsProofOfWork() && !CheckProofOfWork(GetHash(), nBits))
      return error("CBlock::ReadFromDisk() : errors in block header");

    return true;
  }



  void print() const
  {
    printf("CBlock(hash=%s, ver=%d, hashPrevBlock=%s, hashMerkleRoot=%s, nTime=%u, nBits=%08x, nNonce=%u, vtx=%d, vchBlockSig=%s)\n",
           GetHash().ToString().substr(0,20).c_str(),
           nVersion,
           hashPrevBlock.ToString().substr(0,20).c_str(),
           hashMerkleRoot.ToString().substr(0,10).c_str(),
           nTime, nBits, nNonce,
           vtx.size(),
           HexStr(vchBlockSig.begin(), vchBlockSig.end()).c_str());

    printf("CBlock General PoB(nBurnBits=%08x nEffectiveBurnCoins=%"PRI64u" (formatted %s))\n", 
           nBurnBits, nEffectiveBurnCoins, FormatMoney(nEffectiveBurnCoins).c_str());
    
    if(IsProofOfBurn())
      printf("CBlock Specific PoB(fProofOfBurn %s, burnBlkHeight %d, burnCTx %d, burnCTxOut %d)\n",
             fProofOfBurn ? "true" : "false", burnBlkHeight, burnCTx, burnCTxOut);

    for (unsigned int i = 0; i < vtx.size(); i++)
    {
      printf("  ");
      vtx[i].print();
    }
    printf("  vMerkleTree: ");
    for (unsigned int i = 0; i < vMerkleTree.size(); i++)
      printf("%s ", vMerkleTree[i].ToString().substr(0,10).c_str());
    printf("\n");
  }


  bool DisconnectBlock(CTxDB& txdb, CBlockIndex* pindex);
  bool ConnectBlock(CTxDB& txdb, CBlockIndex* pindex);
  bool ReadFromDisk(const CBlockIndex* pindex, bool fReadTransactions=true);
  bool SetBestChain(CTxDB& txdb, CBlockIndex* pindexNew);
  bool AddToBlockIndex(unsigned int nFile, unsigned int nBlockPos);
  bool CheckBlock() const;
  bool AcceptBlock();
  bool GetCoinAge(uint64& nCoinAge) const; // slimcoin: calculate total coin age spent in block
  bool SignBlock(const CKeyStore& keystore);
  bool CheckBlockSignature() const;

private:
  bool SetBestChainInner(CTxDB& txdb, CBlockIndex *pindexNew);
};






/** The block chain is a tree shaped structure starting with the
 * genesis block at the root, with each block potentially having multiple
 * candidates to be the next block.  pprev and pnext link a path through the
 * main/longest chain.  A blockindex may have multiple pprev pointing back
 * to it, but pnext will only point forward to the longest branch, or will
 * be null if the block is not part of the longest chain.
 */
class CBlockIndex
{
public:
  const uint256* phashBlock;
  CBlockIndex* pprev;
  CBlockIndex* pnext;
  unsigned int nFile;
  unsigned int nBlockPos;
  CBigNum bnChainTrust; // slimcoin: trust score of block chain
  int nHeight;
  int64 nMint;
  int64 nMoneySupply;

  unsigned int nFlags;  // slimcoin: block index flags

  enum  
  {
    BLOCK_PROOF_OF_STAKE = (1 << 0), // is proof-of-stake block
    BLOCK_STAKE_ENTROPY  = (1 << 1), // entropy bit for stake modifier
    BLOCK_STAKE_MODIFIER = (1 << 2), // regenerated stake modifier
  };

  uint64 nStakeModifier; // hash modifier for proof-of-stake
  unsigned int nStakeModifierChecksum; // checksum of index; in-memeory only

  // proof-of-stake specific fields
  COutPoint prevoutStake;
  unsigned int nStakeTime;
  uint256 hashProofOfStake;

  // block header
  int nVersion;
  uint256 hashMerkleRoot;
  u32int nTime;
  u32int nBits;
  u32int nNonce;

  // Proof-of-Burn switch, indexes, and values
  bool fProofOfBurn;
  uint256 burnHash; //the recorded burn hash used for DoS detection
  s32int burnBlkHeight;
  s32int burnCTx;
  s32int burnCTxOut;
  int64 nEffectiveBurnCoins;
  u32int nBurnBits;

  CBlockIndex()
  {
    phashBlock = NULL;
    pprev = NULL;
    pnext = NULL;
    nFile = 0;
    nBlockPos = 0;
    nHeight = 0;
    bnChainTrust = 0;
    nMint = 0;
    nMoneySupply = 0;
    nFlags = 0;
    nStakeModifier = 0;
    nStakeModifierChecksum = 0;
    hashProofOfStake = 0;
    prevoutStake.SetNull();
    nStakeTime = 0;

    nVersion       = 0;
    hashMerkleRoot = 0;
    nTime          = 0;
    nBits          = 0;
    nNonce         = 0;

    //PoB
    fProofOfBurn   = false;
    burnHash       = 0;
    burnBlkHeight  = -1;
    burnCTx        = -1;
    burnCTxOut     = -1;
    nEffectiveBurnCoins = 0;
    nBurnBits      = 0;
  }

  CBlockIndex(unsigned int nFileIn, unsigned int nBlockPosIn, CBlock &block)
  {
    phashBlock = NULL;
    pprev = NULL;
    pnext = NULL;
    nFile = nFileIn;
    nBlockPos = nBlockPosIn;
    nHeight = 0;
    bnChainTrust = 0;
    nMint = 0;
    nMoneySupply = 0;
    nFlags = 0;
    nStakeModifier = 0;
    nStakeModifierChecksum = 0;
    hashProofOfStake = 0;

    if(block.IsProofOfStake())
    {
      SetProofOfStake();
      prevoutStake = block.vtx[1].vin[0].prevout;
      nStakeTime = block.vtx[1].nTime;
    }else{
      prevoutStake.SetNull();
      nStakeTime = 0;
    }

    nVersion       = block.nVersion;
    hashMerkleRoot = block.hashMerkleRoot;
    nTime          = block.nTime;
    nBits          = block.nBits;
    nNonce         = block.nNonce;

    //PoB
    fProofOfBurn   = block.fProofOfBurn;
    burnHash       = block.burnHash;
    burnBlkHeight  = block.burnBlkHeight;
    burnCTx        = block.burnCTx;
    burnCTxOut     = block.burnCTxOut;
    nEffectiveBurnCoins = block.nEffectiveBurnCoins;
    nBurnBits      = block.nBurnBits;

  }

  CBlock GetBlockHeader() const
  {
    CBlock block;
    block.nVersion       = nVersion;

    if(pprev)
      block.hashPrevBlock = pprev->GetBlockHash();

    block.hashMerkleRoot = hashMerkleRoot;
    block.nTime          = nTime;
    block.nBits          = nBits;
    block.nNonce         = nNonce;
    return block;
  }

  uint256 GetBlockHash() const
  {
    return *phashBlock;
  }

  int64 GetBlockTime() const
  {
    return (int64)nTime;
  }

  CBigNum GetBlockTrust() const;

  bool IsInMainChain() const
  {
    return (pnext || this == pindexBest);
  }

  bool CheckIndex() const
  {
    /*printf("%5d ------------------------------- %d %d %d %d %d %d %d\n",
           nHeight, IsProofOfWork(), IsProofOfBurn(), IsProofOfStake(), 
           fProofOfBurn, burnBlkHeight, burnCTx, burnCTxOut);*/

    //The burn and stake data will be checked after all block indexes have been loaded

    if(IsProofOfWork())
      return CheckProofOfWork(GetBlockHash(), nBits);
    else if(IsProofOfBurn() || IsProofOfStake()) 
      return true;
    else
      return false;
  }

  bool EraseBlockFromDisk()
  {
    // Open history file
    CAutoFile fileout = CAutoFile(OpenBlockFile(nFile, nBlockPos, "rb+"), SER_DISK, CLIENT_VERSION);
    if (!fileout)
      return false;

    // Overwrite with empty null block
    CBlock block;
    block.SetNull();
    fileout << block;

    return true;
  }

  enum { nMedianTimeSpan=11 };

  int64 GetMedianTimePast() const
  {
    int64 pmedian[nMedianTimeSpan];
    int64* pbegin = &pmedian[nMedianTimeSpan];
    int64* pend = &pmedian[nMedianTimeSpan];

    const CBlockIndex* pindex = this;
    for (int i = 0; i < nMedianTimeSpan && pindex; i++, pindex = pindex->pprev)
      *(--pbegin) = pindex->GetBlockTime();

    std::sort(pbegin, pend);
    return pbegin[(pend - pbegin)/2];
  }

  int64 GetMedianTime() const
  {
    const CBlockIndex* pindex = this;
    for (int i = 0; i < nMedianTimeSpan/2; i++)
    {
      if (!pindex->pnext)
        return GetBlockTime();
      pindex = pindex->pnext;
    }
    return pindex->GetMedianTimePast();
  }

  bool IsProofOfBurn() const
  {
    return fProofOfBurn && burnBlkHeight >= 0 && burnCTx >= 0 && burnCTxOut >= 0;
  }

  bool IsProofOfWork() const
  {
    return !(nFlags & BLOCK_PROOF_OF_STAKE) && !IsProofOfBurn();
  }

  bool IsProofOfStake() const
  {
    return (nFlags & BLOCK_PROOF_OF_STAKE);
  }

  void SetProofOfStake()
  {
    nFlags |= BLOCK_PROOF_OF_STAKE;
  }

  unsigned int GetStakeEntropyBit() const
  {
    return ((nFlags & BLOCK_STAKE_ENTROPY) >> 1);
  }

  bool SetStakeEntropyBit(unsigned int nEntropyBit)
  {
    if (nEntropyBit > 1)
      return false;
    nFlags |= (nEntropyBit? BLOCK_STAKE_ENTROPY : 0);
    return true;
  }

  bool GeneratedStakeModifier() const
  {
    return (nFlags & BLOCK_STAKE_MODIFIER);
  }

  void SetStakeModifier(uint64 nModifier, bool fGeneratedStakeModifier)
  {
    nStakeModifier = nModifier;
    if (fGeneratedStakeModifier)
      nFlags |= BLOCK_STAKE_MODIFIER;
  }

  std::pair<COutPoint, u32int> GetProofOfStake() const
  {
    return std::make_pair(prevoutStake, nStakeTime);
  }

  std::pair<uint256, uint256> GetProofOfBurn() const
  {
    return std::make_pair(burnHash, pprev->GetBlockHash());
  }


  std::string ToString() const
  {
    return strprintf("CBlockIndex(nprev=%08x, pnext=%08x, nFile=%d, nBlockPos=%-6d nHeight=%d, nMint=%s, nMoneySupply=%s, nFlags=(%s)(%d)(%s), nStakeModifier=%016"PRI64x", nStakeModifierChecksum=%08x, hashProofOfStake=%s, prevoutStake=(%s), nStakeTime=%d merkle=%s, hashBlock=%s, nBurnBits=%08x nEffectiveBurnCoins=%"PRI64u" (formatted %s))",
                     pprev, pnext, nFile, nBlockPos, nHeight,
                     FormatMoney(nMint).c_str(), FormatMoney(nMoneySupply).c_str(),
                     GeneratedStakeModifier() ? "MOD" : "-", GetStakeEntropyBit(), IsProofOfStake()? "PoS" : "PoW",
                     nStakeModifier, nStakeModifierChecksum, 
                     hashProofOfStake.ToString().c_str(),
                     prevoutStake.ToString().c_str(), nStakeTime,
                     hashMerkleRoot.ToString().substr(0,10).c_str(),
                     GetBlockHash().ToString().substr(0,20).c_str(),
                     nBurnBits, nEffectiveBurnCoins, FormatMoney(nEffectiveBurnCoins).c_str());
  }

  void print() const
  {
    printf("%s\n", ToString().c_str());
  }
};



/** Used to marshal pointers into hashes for db storage. */
class CDiskBlockIndex : public CBlockIndex
{
private:
  uint256 blockHash;

public:
  uint256 hashPrev;
  uint256 hashNext;

  CDiskBlockIndex()
  {
    hashPrev = 0;
    hashNext = 0;
    blockHash = 0;

  }

  explicit CDiskBlockIndex(CBlockIndex* pindex) : CBlockIndex(*pindex)
  {
    hashPrev = (pprev ? pprev->GetBlockHash() : 0);
    hashNext = (pnext ? pnext->GetBlockHash() : 0);
  }

  IMPLEMENT_SERIALIZE
    (
      if (!(nType & SER_GETHASH))
        READWRITE(nVersion);

      READWRITE(hashNext);
      READWRITE(nFile);
      READWRITE(nBlockPos);
      READWRITE(nHeight);
      READWRITE(nMint);
      READWRITE(nMoneySupply);
      READWRITE(nFlags);
      READWRITE(nStakeModifier);

      if(IsProofOfStake())
      {
        READWRITE(prevoutStake);
        READWRITE(nStakeTime);
        READWRITE(hashProofOfStake);
      }else if(fRead)
      {
        const_cast<CDiskBlockIndex*>(this)->prevoutStake.SetNull();
        const_cast<CDiskBlockIndex*>(this)->nStakeTime = 0;
        const_cast<CDiskBlockIndex*>(this)->hashProofOfStake = 0;
      }

      // block header
      READWRITE(this->nVersion);
      READWRITE(hashPrev);
      READWRITE(hashMerkleRoot);
      READWRITE(nTime);
      READWRITE(nBits);
      READWRITE(nNonce);

      // PoB
      READWRITE(fProofOfBurn);
      READWRITE(burnHash);
      READWRITE(burnBlkHeight);
      READWRITE(burnCTx);
      READWRITE(burnCTxOut);
      READWRITE(nEffectiveBurnCoins);
      READWRITE(nBurnBits);

      //cached hash of the block
      READWRITE(blockHash);
      )

    uint256 GetBlockHash() const
  {
    if(fUseFastIndex && (nTime < GetAdjustedTime() - 24 * 60 * 60) && blockHash != 0)
      return blockHash;

    CBlock block;
    block.nVersion        = nVersion;
    block.hashPrevBlock   = hashPrev;
    block.hashMerkleRoot  = hashMerkleRoot;
    block.nTime           = nTime;
    block.nBits           = nBits;
    block.nNonce          = nNonce;

    //assign the cached value to be written a value
    const_cast<CDiskBlockIndex*>(this)->blockHash = block.GetHash();

    return blockHash;
  }


  std::string ToString() const
  {
    std::string str = "CDiskBlockIndex(";
    str += CBlockIndex::ToString();
    str += strprintf("\n                hashBlock=%s, hashPrev=%s, hashNext=%s)",
                     GetBlockHash().ToString().c_str(),
                     hashPrev.ToString().substr(0,20).c_str(),
                     hashNext.ToString().substr(0,20).c_str());
    return str;
  }

  void print() const
  {
    printf("%s\n", ToString().c_str());
  }
};








/** Describes a place in the block chain to another node such that if the
 * other node doesn't have the same branch, it can find a recent common trunk.
 * The further back it is, the further before the fork it may be.
 */
class CBlockLocator
{
protected:
  std::vector<uint256> vHave;
public:

  CBlockLocator()
  {
  }

  explicit CBlockLocator(const CBlockIndex* pindex)
  {
    Set(pindex);
  }

  explicit CBlockLocator(uint256 hashBlock)
  {
    std::map<uint256, CBlockIndex*>::iterator mi = mapBlockIndex.find(hashBlock);
    if (mi != mapBlockIndex.end())
      Set((*mi).second);
  }

  CBlockLocator(const std::vector<uint256>& vHaveIn)
  {
    vHave = vHaveIn;
  }

  IMPLEMENT_SERIALIZE
    (
      if (!(nType & SER_GETHASH))
        READWRITE(nVersion);
      READWRITE(vHave);
      )

    void SetNull()
  {
    vHave.clear();
  }

  bool IsNull()
  {
    return vHave.empty();
  }

  void Set(const CBlockIndex* pindex)
  {
    vHave.clear();
    int nStep = 1;
    while (pindex)
    {
      vHave.push_back(pindex->GetBlockHash());

      // Exponentially larger steps back
      for (int i = 0; pindex && i < nStep; i++)
        pindex = pindex->pprev;
      if (vHave.size() > 10)
        nStep *= 2;
    }
    vHave.push_back(hashGenesisBlock);
  }

  int GetDistanceBack()
  {
    // Retrace how far back it was in the sender's branch
    int nDistance = 0;
    int nStep = 1;
    BOOST_FOREACH(const uint256& hash, vHave)
    {
      std::map<uint256, CBlockIndex*>::iterator mi = mapBlockIndex.find(hash);
      if (mi != mapBlockIndex.end())
      {
        CBlockIndex* pindex = (*mi).second;
        if (pindex->IsInMainChain())
          return nDistance;
      }
      nDistance += nStep;
      if (nDistance > 10)
        nStep *= 2;
    }
    return nDistance;
  }

  CBlockIndex* GetBlockIndex()
  {
    // Find the first block the caller has in the main chain
    BOOST_FOREACH(const uint256& hash, vHave)
    {
      std::map<uint256, CBlockIndex*>::iterator mi = mapBlockIndex.find(hash);
      if (mi != mapBlockIndex.end())
      {
        CBlockIndex* pindex = (*mi).second;
        if (pindex->IsInMainChain())
          return pindex;
      }
    }
    return pindexGenesisBlock;
  }

  uint256 GetBlockHash()
  {
    // Find the first block the caller has in the main chain
    BOOST_FOREACH(const uint256& hash, vHave)
    {
      std::map<uint256, CBlockIndex*>::iterator mi = mapBlockIndex.find(hash);
      if (mi != mapBlockIndex.end())
      {
        CBlockIndex* pindex = (*mi).second;
        if (pindex->IsInMainChain())
          return hash;
      }
    }
    return hashGenesisBlock;
  }

  int GetHeight()
  {
    CBlockIndex* pindex = GetBlockIndex();
    if (!pindex)
      return 0;
    return pindex->nHeight;
  }
};









/** Alerts are for notifying old versions if they become too obsolete and
 * need to upgrade.  The message is displayed in the status bar.
 * Alert messages are broadcast as a vector of signed data.  Unserializing may
 * not read the entire buffer if the alert is for a newer version, but older
 * versions can still relay the original data.
 */
class CUnsignedAlert
{
public:
  int nVersion;
  int64 nRelayUntil;      // when newer nodes stop relaying to newer nodes
  int64 nExpiration;
  int nID;
  int nCancel;
  std::set<int> setCancel;
  int nMinVer;            // lowest version inclusive
  int nMaxVer;            // highest version inclusive
  std::set<std::string> setSubVer;  // empty matches all
  int nPriority;

  // Actions
  std::string strComment;
  std::string strStatusBar;
  std::string strReserved;

  IMPLEMENT_SERIALIZE
    (
      READWRITE(this->nVersion);
      nVersion = this->nVersion;
      READWRITE(nRelayUntil);
      READWRITE(nExpiration);
      READWRITE(nID);
      READWRITE(nCancel);
      READWRITE(setCancel);
      READWRITE(nMinVer);
      READWRITE(nMaxVer);
      READWRITE(setSubVer);
      READWRITE(nPriority);

      READWRITE(strComment);
      READWRITE(strStatusBar);
      READWRITE(strReserved);
      )

    void SetNull()
  {
    nVersion = 1;
    nRelayUntil = 0;
    nExpiration = 0;
    nID = 0;
    nCancel = 0;
    setCancel.clear();
    nMinVer = 0;
    nMaxVer = 0;
    setSubVer.clear();
    nPriority = 0;

    strComment.clear();
    strStatusBar.clear();
    strReserved.clear();
  }

  std::string ToString() const
  {
    std::string strSetCancel;
    BOOST_FOREACH(int n, setCancel)
      strSetCancel += strprintf("%d ", n);
    std::string strSetSubVer;
    BOOST_FOREACH(std::string str, setSubVer)
      strSetSubVer += "\"" + str + "\" ";
    return strprintf(
      "CAlert(\n"
      "    nVersion     = %d\n"
      "    nRelayUntil  = %"PRI64d"\n"
      "    nExpiration  = %"PRI64d"\n"
      "    nID          = %d\n"
      "    nCancel      = %d\n"
      "    setCancel    = %s\n"
      "    nMinVer      = %d\n"
      "    nMaxVer      = %d\n"
      "    setSubVer    = %s\n"
      "    nPriority    = %d\n"
      "    strComment   = \"%s\"\n"
      "    strStatusBar = \"%s\"\n"
      ")\n",
      nVersion,
      nRelayUntil,
      nExpiration,
      nID,
      nCancel,
      strSetCancel.c_str(),
      nMinVer,
      nMaxVer,
      strSetSubVer.c_str(),
      nPriority,
      strComment.c_str(),
      strStatusBar.c_str());
  }

  void print() const
  {
    printf("%s", ToString().c_str());
  }
};

/** An alert is a combination of a serialized CUnsignedAlert and a signature. */
class CAlert : public CUnsignedAlert
{
public:
  std::vector<unsigned char> vchMsg;
  std::vector<unsigned char> vchSig;

  CAlert()
  {
    SetNull();
  }

  IMPLEMENT_SERIALIZE
    (
      READWRITE(vchMsg);
      READWRITE(vchSig);
      )

    void SetNull()
  {
    CUnsignedAlert::SetNull();
    vchMsg.clear();
    vchSig.clear();
  }

  bool IsNull() const
  {
    return (nExpiration == 0);
  }

  uint256 GetHash() const
  {
    return SerializeHash(*this);
  }

  bool IsInEffect() const
  {
    return (GetAdjustedTime() < nExpiration);
  }

  bool Cancels(const CAlert& alert) const
  {
    if (!IsInEffect())
      return false; // this was a no-op before 31403
    return (alert.nID <= nCancel || setCancel.count(alert.nID));
  }

  bool AppliesTo(int nVersion, std::string strSubVerIn) const
  {
    // TODO: rework for client-version-embedded-in-strSubVer ?
    return (IsInEffect() &&
            nMinVer <= nVersion && nVersion <= nMaxVer &&
            (setSubVer.empty() || setSubVer.count(strSubVerIn)));
  }

  bool AppliesToMe() const
  {
    return AppliesTo(PROTOCOL_VERSION, FormatSubVersion(CLIENT_NAME, CLIENT_VERSION, std::vector<std::string>()));
  }

  bool RelayTo(CNode* pnode) const
  {
    if (!IsInEffect())
      return false;
    // returns true if wasn't already contained in the set
    if (pnode->setKnown.insert(GetHash()).second)
    {
      if (AppliesTo(pnode->nVersion, pnode->strSubVer) ||
          AppliesToMe() ||
          GetAdjustedTime() < nRelayUntil)
      {
        pnode->PushMessage("alert", *this);
        return true;
      }
    }
    return false;
  }

  bool CheckSignature()
  {
    CKey key;
    if (!key.SetPubKey(ParseHex("04a0a849dd49b113d3179a332dd77715c43be4d0076e2f19e66de23dd707e56630f792f298dfd209bf042bb3561f4af6983f3d81e439737ab0bf7f898fecd21aab")))
      return error("CAlert::CheckSignature() : SetPubKey failed");
    if (!key.Verify(Hash(vchMsg.begin(), vchMsg.end()), vchSig))
      return error("CAlert::CheckSignature() : verify signature failed");

    // Now unserialize the data
    CDataStream sMsg(vchMsg, SER_NETWORK, PROTOCOL_VERSION);
    sMsg >> *(CUnsignedAlert*)this;
    return true;
  }

  bool ProcessAlert();
};

class CTxMemPool
{
public:
  mutable CCriticalSection cs;
  std::map<uint256, CTransaction> mapTx;
  std::map<COutPoint, CInPoint> mapNextTx;

  bool accept(CTxDB& txdb, CTransaction &tx,
              bool fCheckInputs, bool* pfMissingInputs);
  bool addUnchecked(CTransaction &tx);
  bool remove(CTransaction &tx);

  unsigned long size()
  {
    LOCK(cs);
    return mapTx.size();
  }

  bool exists(uint256 hash)
  {
    return (mapTx.count(hash) != 0);
  }

  CTransaction& lookup(uint256 hash)
  {
    return mapTx[hash];
  }
};

extern CTxMemPool mempool;

#endif
