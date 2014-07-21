// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2012 The Bitcoin developers
// Copyright (c) 2011-2013 The Peercoin developers
// Copyright (c) 2013-2014 The Slimcoin Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "checkpoints.h"
#include "db.h"
#include "net.h"
#include "init.h"
#include "ui_interface.h"
#include "kernel.h"
#include <boost/algorithm/string/replace.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <math.h>       /* pow */

#include "dcrypt.h"

using namespace std;
using namespace boost;

//
// Global state
//

CCriticalSection cs_setpwalletRegistered;
set<CWallet*> setpwalletRegistered;

CCriticalSection cs_main;

CTxMemPool mempool;
unsigned int nTransactionsUpdated = 0;

map<uint256, CBlockIndex*> mapBlockIndex;
uint256 hashGenesisBlock = hashGenesisBlockOfficial;
static CBigNum bnProofOfWorkLimit(~uint256(0) >> 20); //5 preceding 0s, 20/4 since every hex = 4 bits
static CBigNum bnProofOfBurnLimit(~uint256(0) >> 16); //4 preceding 0s, 16/4 since every hex = 4 bits
static CBigNum bnProofOfStakeLimit(~uint256(0) >> 27); //0x0000001ffff....fff
static CBigNum bnInitialHashTarget(~uint256(0) >> 21); //0x000007fffff....fff
static uint256 nPoWBase(~uint256(0) >> 24); //6 preceding 0s, 24/4 since every hex = 4 bits
static uint256 nPoBBase(~uint256(0) >> 20); //5 preceding 0s, 20/4 since every hex = 4 bits
unsigned int nStakeMinAge = STAKE_MIN_AGE;
int nCoinbaseMaturity = COINBASE_MATURITY_SLM;
CBlockIndex *pindexGenesisBlock = NULL;
int nBestHeight = -1;
CBigNum bnBestChainTrust = 0;
CBigNum bnBestInvalidTrust = 0;
uint256 hashBestChain = 0;
CBlockIndex *pindexBest = NULL;
int64 nTimeBestReceived = 0;

CMedianFilter<int> cPeerBlockCounts(5, 0); // Amount of blocks that other nodes claim to have

map<uint256, CBlock*> mapOrphanBlocks;
multimap<uint256, CBlock*> mapOrphanBlocksByPrev;
set<pair<COutPoint, unsigned int> > setStakeSeenOrphan;
map<uint256, uint256> mapProofOfStake;
set<pair<COutPoint, unsigned int> > setStakeSeen;
set<pair<uint256, uint256> > setBurnSeen;
set<uint256> setBurnSeenOrphan;

map<uint256, CDataStream*> mapOrphanTransactions;
map<uint256, map<uint256, CDataStream*> > mapOrphanTransactionsByPrev;

// Constant stuff for coinbase transactions we create:
CScript COINBASE_FLAGS;

const string strMessageMagic = "SLIMCoin Signed Message:\n";

double dHashesPerSec;
int64 nHPSTimerStart;

// Settings
int64 nTransactionFee = MIN_TX_FEE;

// Dcrypt hash scanner
static u32int ScanDcryptHash(CBlock *pblock, u32int *nHashesDone, uint256 *phash);

//////////////////////////////////////////////////////////////////////////////
//
// dispatching functions
//

// These functions dispatch to one or all registered wallets


void RegisterWallet(CWallet* pwalletIn)
{
  {
    LOCK(cs_setpwalletRegistered);
    setpwalletRegistered.insert(pwalletIn);
  }
}

void UnregisterWallet(CWallet* pwalletIn)
{
  {
    LOCK(cs_setpwalletRegistered);
    setpwalletRegistered.erase(pwalletIn);
  }
}

// check whether the passed transaction is from us
bool static IsFromMe(CTransaction& tx)
{
  BOOST_FOREACH(CWallet* pwallet, setpwalletRegistered)
    if(pwallet->IsFromMe(tx))
      return true;
  return false;
}

// get the wallet transaction with the given hash (if it exists)
bool static GetTransaction(const uint256& hashTx, CWalletTx& wtx)
{
  BOOST_FOREACH(CWallet* pwallet, setpwalletRegistered)
    if(pwallet->GetTransaction(hashTx, wtx))
      return true;
  return false;
}

// Return transaction in tx, and if it was found inside a block, its hash is placed in hashBlock
bool GetTransaction(const uint256 &hash, CTransaction &tx, uint256 &hashBlock)
{
  {
    LOCK(cs_main);
    {
      LOCK(mempool.cs);
      if(mempool.exists(hash))
      {
        tx = mempool.lookup(hash);
        return true;
      }
    }

    CTxDB txdb("r");
    CTxIndex txindex;
    if (tx.ReadFromDisk(txdb, COutPoint(hash, 0), txindex))
    {
      CBlock block;
      if (block.ReadFromDisk(txindex.pos.nFile, txindex.pos.nBlockPos, false))
        hashBlock = block.GetHash();
      return true;
    }
  }
  return false;
}

// erases transaction with the given hash from all wallets
void static EraseFromWallets(uint256 hash)
{
  BOOST_FOREACH(CWallet* pwallet, setpwalletRegistered)
    pwallet->EraseFromWallet(hash);
}

// make sure all wallets know about the given transaction, in the given block
void SyncWithWallets(const CTransaction& tx, const CBlock* pblock, bool fUpdate, bool fConnect)
{
  if(!fConnect)
  {
    // slimcoin: wallets need to refund inputs when disconnecting coinstake
    if(tx.IsCoinStake())
    {
      BOOST_FOREACH(CWallet* pwallet, setpwalletRegistered)
        if(pwallet->IsFromMe(tx))
          pwallet->DisableTransaction(tx);
    }
    return;
  }

  BOOST_FOREACH(CWallet* pwallet, setpwalletRegistered)
    pwallet->AddToWalletIfInvolvingMe(tx, pblock, fUpdate);
}

// notify wallets about a new best chain
void static SetBestChain(const CBlockLocator& loc)
{
  BOOST_FOREACH(CWallet* pwallet, setpwalletRegistered)
    pwallet->SetBestChain(loc);
}

// notify wallets about an updated transaction
void static UpdatedTransaction(const uint256& hashTx)
{
  BOOST_FOREACH(CWallet* pwallet, setpwalletRegistered)
    pwallet->UpdatedTransaction(hashTx);
}

// dump all wallets
void static PrintWallets(const CBlock& block)
{
  BOOST_FOREACH(CWallet* pwallet, setpwalletRegistered)
    pwallet->PrintWallet(block);
}

// notify wallets about an incoming inventory (for request counts)
void static Inventory(const uint256& hash)
{
  BOOST_FOREACH(CWallet* pwallet, setpwalletRegistered)
    pwallet->Inventory(hash);
}

// ask wallets to resend their transactions
void static ResendWalletTransactions()
{
  BOOST_FOREACH(CWallet* pwallet, setpwalletRegistered)
    pwallet->ResendWalletTransactions();
}







//////////////////////////////////////////////////////////////////////////////
//
// mapOrphanTransactions
//

bool AddOrphanTx(const CDataStream &vMsg)
{
  CTransaction tx;
  CDataStream(vMsg) >> tx;
  uint256 hash = tx.GetHash();
  if(mapOrphanTransactions.count(hash))
    return false;

  CDataStream* pvMsg = new CDataStream(vMsg);

  // Ignore big transactions, to avoid a
  // send-big-orphans memory exhaustion attack. If a peer has a legitimate
  // large transaction with a missing parent then we assume
  // it will rebroadcast it later, after the parent transaction(s)
  // have been mined or received.
  // 10,000 orphans, each of which is at most 5,000 bytes big is
  // at most 500 megabytes of orphans:
  if(pvMsg->size() > 5000)
  {
    printf("ignoring large orphan tx (size: %u, hash: %s)\n", pvMsg->size(), hash.ToString().substr(0,10).c_str());
    delete pvMsg;
    return false;
  }

  mapOrphanTransactions[hash] = pvMsg;
  BOOST_FOREACH(const CTxIn& txin, tx.vin)
    mapOrphanTransactionsByPrev[txin.prevout.hash].insert(make_pair(hash, pvMsg));

  printf("stored orphan tx %s (mapsz %u)\n", hash.ToString().substr(0,10).c_str(),
         mapOrphanTransactions.size());
  return true;
}

void static EraseOrphanTx(uint256 hash)
{
  if(!mapOrphanTransactions.count(hash))
    return;
  const CDataStream* pvMsg = mapOrphanTransactions[hash];
  CTransaction tx;
  CDataStream(*pvMsg) >> tx;
  BOOST_FOREACH(const CTxIn& txin, tx.vin)
  {
    mapOrphanTransactionsByPrev[txin.prevout.hash].erase(hash);
    if(mapOrphanTransactionsByPrev[txin.prevout.hash].empty())
      mapOrphanTransactionsByPrev.erase(txin.prevout.hash);
  }
  delete pvMsg;
  mapOrphanTransactions.erase(hash);
}

unsigned int LimitOrphanTxSize(unsigned int nMaxOrphans)
{
  unsigned int nEvicted = 0;
  while(mapOrphanTransactions.size() > nMaxOrphans)
  {
    // Evict a random orphan:
    uint256 randomhash = GetRandHash();
    map<uint256, CDataStream*>::iterator it = mapOrphanTransactions.lower_bound(randomhash);
    if(it == mapOrphanTransactions.end())
      it = mapOrphanTransactions.begin();
    EraseOrphanTx(it->first);
    ++nEvicted;
  }
  return nEvicted;
}







//////////////////////////////////////////////////////////////////////////////
//
// CTransaction and CTxIndex
//

bool CTransaction::ReadFromDisk(CTxDB& txdb, COutPoint prevout, CTxIndex& txindexRet)
{
  SetNull();
  if(!txdb.ReadTxIndex(prevout.hash, txindexRet))
    return false;
  if(!ReadFromDisk(txindexRet.pos))
    return false;
  if(prevout.n >= vout.size())
  {
    SetNull();
    return false;
  }
  return true;
}

bool CTransaction::ReadFromDisk(CTxDB& txdb, COutPoint prevout)
{
  CTxIndex txindex;
  return ReadFromDisk(txdb, prevout, txindex);
}

bool CTransaction::ReadFromDisk(COutPoint prevout)
{
  CTxDB txdb("r");
  CTxIndex txindex;
  return ReadFromDisk(txdb, prevout, txindex);
}


//Returns the pubKet of the first txIn of this tx
// fOurPubKey is a bool stating, if the outTx pointed to by the outpoint of the first 
//     inTx of this transaction has a CScript that is type TX_PUBKEYHASH, if fOurPubKey
//     is true, then it will look the hash up in pwalletMain's keystore and return
//     a CScript of type TX_PUBKEY in scriptPubKeyRet. 
//
//     If the sender of this transaction is not us and the outTx's CScript is type TX_PUBKEYHASH, 
//     it is impossible for us to return a CScript of type TX_PUBKEY
bool CTransaction::GetSendersPubKey(CScript &scriptPubKeyRet, bool fOurPubKey) const
{
  if(vin.empty())
    return false;
    
  const CTxIn input = vin[0];

  CTransaction prevTx;

  // First try finding the previous transaction in database
  CTxDB txdb("r");
  CTxIndex txindex;

  //read the transaction of the prevout
  if(!prevTx.ReadFromDisk(txdb, input.prevout, txindex))
    return false; //if the read disk failed

  txdb.Close();

  //get the script key
  scriptPubKeyRet = prevTx.vout[input.prevout.n].scriptPubKey;

  //Check what type the scriptPubKeyRet is
  vector<valtype> vSolutions;
  txnouttype whichType;
  if(!Solver(scriptPubKeyRet, whichType, vSolutions))
    return error("GetSendersPubKey() : Solver failed");

  //if the scriptPubKeyRet is a pubkey hash, get the raw public key form
  if(whichType == TX_PUBKEYHASH && fOurPubKey == true) // pay to address type
  {
    // convert to pay to public key type
    CKey key;
    if(!pwalletMain->GetKey(uint160(vSolutions[0]), key))
      return error("GetSendersPubKey() : failed to get key for burn tx type=%d", whichType);

    //clear the old data and give it the new data
    scriptPubKeyRet.clear();
    scriptPubKeyRet << key.GetPubKey() << OP_CHECKSIG;
  }

  //sucess!
  return true;
}

bool CTransaction::IsStandard() const
{
  BOOST_FOREACH(const CTxIn& txin, vin)
  {
    // Biggest 'standard' txin is a 3-signature 3-of-3 CHECKMULTISIG
    // pay-to-script-hash, which is 3 ~80-byte signatures, 3
    // ~65-byte public keys, plus a few script ops.
    if(txin.scriptSig.size() > 500)
      return false;
    if(!txin.scriptSig.IsPushOnly())
      return false;
  }
  BOOST_FOREACH(const CTxOut& txout, vout)
    if(!::IsStandard(txout.scriptPubKey))
      return false;
  return true;
}

//
// Check transaction inputs, and make sure any
// pay-to-script-hash transactions are evaluating IsStandard scripts
//
// Why bother? To avoid denial-of-service attacks; an attacker
// can submit a standard HASH... OP_EQUAL transaction,
// which will get accepted into blocks. The redemption
// script can be anything; an attacker could use a very
// expensive-to-check-upon-redemption script like:
//   DUP CHECKSIG DROP ... repeated 100 times... OP_1
//
bool CTransaction::AreInputsStandard(const MapPrevTx& mapInputs) const
{
  if(IsCoinBase())
    return true; // Coinbases don't use vin normally

  for(unsigned int i = 0; i < vin.size(); i++)
  {
    const CTxOut& prev = GetOutputFor(vin[i], mapInputs);

    vector<vector<unsigned char> > vSolutions;
    txnouttype whichType;
    // get the scriptPubKey corresponding to this input:
    const CScript& prevScript = prev.scriptPubKey;
    if(!Solver(prevScript, whichType, vSolutions))
      return false;
    int nArgsExpected = ScriptSigArgsExpected(whichType, vSolutions);
    if(nArgsExpected < 0)
      return false;

    // Transactions with extra stuff in their scriptSigs are
    // non-standard. Note that this EvalScript() call will
    // be quick, because if there are any operations
    // beside "push data" in the scriptSig the
    // IsStandard() call returns false
    vector<vector<unsigned char> > stack;
    if(!EvalScript(stack, vin[i].scriptSig, *this, i, 0))
      return false;

    if(whichType == TX_SCRIPTHASH)
    {
      if(stack.empty())
        return false;
      CScript subscript(stack.back().begin(), stack.back().end());
      vector<vector<unsigned char> > vSolutions2;
      txnouttype whichType2;
      if(!Solver(subscript, whichType2, vSolutions2))
        return false;
      if(whichType2 == TX_SCRIPTHASH)
        return false;

      int tmpExpected;
      tmpExpected = ScriptSigArgsExpected(whichType2, vSolutions2);
      if(tmpExpected < 0)
        return false;
      nArgsExpected += tmpExpected;
    }

    if(stack.size() != (unsigned int)nArgsExpected)
      return false;
  }

  return true;
}

unsigned int CTransaction::GetLegacySigOpCount() const
{
  unsigned int nSigOps = 0;
  BOOST_FOREACH(const CTxIn& txin, vin)
  {
    nSigOps += txin.scriptSig.GetSigOpCount(false);
  }

  BOOST_FOREACH(const CTxOut& txout, vout)
  {
    nSigOps += txout.scriptPubKey.GetSigOpCount(false);
  }
  return nSigOps;
}


int CMerkleTx::SetMerkleBranch(const CBlock* pblock)
{
  if(fClient)
  {
    if(hashBlock == 0)
      return 0;
  }
  else
  {
    CBlock blockTmp;
    if(pblock == NULL)
    {
      // Load the block this tx is in
      CTxIndex txindex;
      if(!CTxDB("r").ReadTxIndex(GetHash(), txindex))
        return 0;
      if(!blockTmp.ReadFromDisk(txindex.pos.nFile, txindex.pos.nBlockPos))
        return 0;
      pblock = &blockTmp;
    }

    // Update the tx's hashBlock
    hashBlock = pblock->GetHash();

    // Locate the transaction
    for(nIndex = 0; nIndex < (int)pblock->vtx.size(); nIndex++)
      if(pblock->vtx[nIndex] == *(CTransaction*)this)
        break;
    if(nIndex == (int)pblock->vtx.size())
    {
      vMerkleBranch.clear();
      nIndex = -1;
      printf("ERROR: SetMerkleBranch() : couldn't find tx in block\n");
      return 0;
    }

    // Fill in merkle branch
    vMerkleBranch = pblock->GetMerkleBranch(nIndex);
  }

  // Is the tx in a block that's in the main chain
  map<uint256, CBlockIndex*>::iterator mi = mapBlockIndex.find(hashBlock);
  if(mi == mapBlockIndex.end())
    return 0;
  CBlockIndex* pindex = (*mi).second;
  if(!pindex || !pindex->IsInMainChain())
    return 0;

  return pindexBest->nHeight - pindex->nHeight + 1;
}







bool CTransaction::CheckTransaction() const
{
  // Basic checks that don't depend on any context
  if(vin.empty())
    return DoS(10, error("CTransaction::CheckTransaction() : vin empty"));
  if(vout.empty())
    return DoS(10, error("CTransaction::CheckTransaction() : vout empty"));
  // Size limits
  if(::GetSerializeSize(*this, SER_NETWORK, PROTOCOL_VERSION) > MAX_BLOCK_SIZE)
    return DoS(100, error("CTransaction::CheckTransaction() : size limits failed"));

  // Check for negative or overflow output values
  int64 nValueOut = 0;
  for(int i = 0; i < vout.size(); i++)
  {
    const CTxOut& txout = vout[i];

    if(txout.IsEmpty() && (!IsCoinBase()) && (!IsCoinStake()))
      return DoS(100, error("CTransaction::CheckTransaction() : txout empty for user transaction"));
    // slimcoin: enforce minimum output amount
    if((!txout.IsEmpty()) && txout.nValue < MIN_TXOUT_AMOUNT)
      return DoS(100, error("CTransaction::CheckTransaction() : txout.nValue below minimum"));
    if(txout.nValue > MAX_MONEY)
      return DoS(100, error("CTransaction::CheckTransaction() : txout.nValue too high"));
    nValueOut += txout.nValue;
    if(!MoneyRange(nValueOut))
      return DoS(100, error("CTransaction::CheckTransaction() : txout total out of range"));
  }

  // Check for duplicate inputs
  set<COutPoint> vInOutPoints;
  BOOST_FOREACH(const CTxIn& txin, vin)
  {
    if(vInOutPoints.count(txin.prevout))
      return false;
    vInOutPoints.insert(txin.prevout);
  }

  if(IsCoinBase())
  {
    if(vin[0].scriptSig.size() < 2 || vin[0].scriptSig.size() > 100)
    {
      printf("SCRIPT size is %d\n", vin[0].scriptSig.size());
      return DoS(100, error("CTransaction::CheckTransaction() : coinbase script size"));
    }
  }else{
    BOOST_FOREACH(const CTxIn& txin, vin)
      if(txin.prevout.IsNull())
        return DoS(10, error("CTransaction::CheckTransaction() : prevout is null"));
  }

  return true;
}

bool CTxMemPool::accept(CTxDB& txdb, CTransaction &tx, bool fCheckInputs,
                        bool* pfMissingInputs)
{
  if(pfMissingInputs)
    *pfMissingInputs = false;

  if(!tx.CheckTransaction())
    return error("CTxMemPool::accept() : CheckTransaction failed");

  // Coinbase is only valid in a block, not as a loose transaction
  if(tx.IsCoinBase())
    return tx.DoS(100, error("CTxMemPool::accept() : coinbase as individual tx"));
  // slimcoin: coinstake is also only valid in a block, not as a loose transaction
  if(tx.IsCoinStake())
    return tx.DoS(100, error("CTxMemPool::accept() : coinstake as individual tx"));

  // To help v0.1.5 clients who would see it as a negative number
  if((int64)tx.nLockTime > std::numeric_limits<int>::max())
    return error("CTxMemPool::accept() : not accepting nLockTime beyond 2038 yet");

  // Rather not work on nonstandard transactions (unless -testnet)
  if(!fTestNet && !tx.IsStandard())
    return error("CTxMemPool::accept() : nonstandard transaction type");

  // Do we already have it?
  uint256 hash = tx.GetHash();
  {
    LOCK(cs);
    if(mapTx.count(hash))
      return false;
  }
  if(fCheckInputs)
    if(txdb.ContainsTx(hash))
      return false;

  // Check for conflicts with in-memory transactions
  CTransaction* ptxOld = NULL;
  for(unsigned int i = 0; i < tx.vin.size(); i++)
  {
    COutPoint outpoint = tx.vin[i].prevout;
    if(mapNextTx.count(outpoint))
    {
      // Disable replacement feature for now
      return false;

      // Allow replacing with a newer version of the same transaction
      if(i != 0)
        return false;
      ptxOld = mapNextTx[outpoint].ptx;
      if(ptxOld->IsFinal())
        return false;
      if(!tx.IsNewerThan(*ptxOld))
        return false;
      for(unsigned int i = 0; i < tx.vin.size(); i++)
      {
        COutPoint outpoint = tx.vin[i].prevout;
        if(!mapNextTx.count(outpoint) || mapNextTx[outpoint].ptx != ptxOld)
          return false;
      }
      break;
    }
  }

  if(fCheckInputs)
  {
    MapPrevTx mapInputs;
    map<uint256, CTxIndex> mapUnused;
    bool fInvalid = false;
    if(!tx.FetchInputs(txdb, mapUnused, false, false, mapInputs, fInvalid))
    {
      if(fInvalid)
        return error("CTxMemPool::accept() : FetchInputs found invalid tx %s", hash.ToString().substr(0,10).c_str());
      if(pfMissingInputs)
        *pfMissingInputs = true;
      return error("CTxMemPool::accept() : FetchInputs failed %s", hash.ToString().substr(0,10).c_str());
    }

    // Check for non-standard pay-to-script-hash in inputs
    if(!tx.AreInputsStandard(mapInputs) && !fTestNet)
      return error("CTxMemPool::accept() : nonstandard transaction input");

    // Note: if you modify this code to accept non-standard transactions, then
    // you should add code here to check that the transaction does a
    // reasonable number of ECDSA signature verifications.

    int64 nFees = tx.GetValueIn(mapInputs)-tx.GetValueOut();
    unsigned int nSize = ::GetSerializeSize(tx, SER_NETWORK, PROTOCOL_VERSION);

    // Don't accept it if it can't get into a block
    if(nFees < tx.GetMinFee(1000, false, GMF_RELAY))
      return error("CTxMemPool::accept() : not enough fees");

    // Continuously rate-limit free transactions
    // This mitigates 'penny-flooding' -- sending thousands of free transactions just to
    // be annoying or make other's transactions take longer to confirm.
    if(nFees < MIN_RELAY_TX_FEE)
    {
      static CCriticalSection cs;
      static double dFreeCount;
      static int64 nLastTime;
      int64 nNow = GetTime();

      {
        LOCK(cs);
        // Use an exponentially decaying ~10-minute window:
        dFreeCount *= pow(1.0 - 1.0/600.0, (double)(nNow - nLastTime));
        nLastTime = nNow;
        // -limitfreerelay unit is thousand-bytes-per-minute
        // At default rate it would take over a month to fill 1GB
        if(dFreeCount > GetArg("-limitfreerelay", 15)*10*1000 && !IsFromMe(tx))
          return error("CTxMemPool::accept() : free transaction rejected by rate limiter");
        if(fDebug)
          printf("Rate limit dFreeCount: %g => %g\n", dFreeCount, dFreeCount+nSize);
        dFreeCount += nSize;
      }
    }

    // Check against previous transactions
    // This is done last to help prevent CPU exhaustion denial-of-service attacks.
    if(!tx.ConnectInputs(txdb, mapInputs, mapUnused, CDiskTxPos(1,1,1), pindexBest, false, false))
    {
      return error("CTxMemPool::accept() : ConnectInputs failed %s", hash.ToString().substr(0, 10).c_str());
    }
  }

  // Store transaction in memory
  {
    LOCK(cs);
    if(ptxOld)
    {
      printf("CTxMemPool::accept() : replacing tx %s with new version\n", ptxOld->GetHash().ToString().c_str());
      remove(*ptxOld);
    }
    addUnchecked(tx);
  }

  ///// are we sure this is ok when loading transactions or restoring block txes
  // If updated, erase old tx from wallet
  if(ptxOld)
    EraseFromWallets(ptxOld->GetHash());

  printf("CTxMemPool::accept() : accepted %s\n", hash.ToString().substr(0,10).c_str());
  return true;
}

bool CTransaction::AcceptToMemoryPool(CTxDB& txdb, bool fCheckInputs, bool* pfMissingInputs)
{
  return mempool.accept(txdb, *this, fCheckInputs, pfMissingInputs);
}

bool CTxMemPool::addUnchecked(CTransaction &tx)
{
  printf("addUnchecked(): size %lu\n",  mapTx.size());
  // Add to memory pool without checking anything.  Don't call this directly,
  // call CTxMemPool::accept to properly check the transaction first.
  {
    LOCK(cs);
    uint256 hash = tx.GetHash();
    mapTx[hash] = tx;
    for(unsigned int i = 0; i < tx.vin.size(); i++)
      mapNextTx[tx.vin[i].prevout] = CInPoint(&mapTx[hash], i);
    nTransactionsUpdated++;
  }
  return true;
}


bool CTxMemPool::remove(CTransaction &tx)
{
  // Remove transaction from memory pool
  {
    LOCK(cs);
    uint256 hash = tx.GetHash();
    if(mapTx.count(hash))
    {
      BOOST_FOREACH(const CTxIn& txin, tx.vin)
        mapNextTx.erase(txin.prevout);
      mapTx.erase(hash);
      nTransactionsUpdated++;
    }
  }
  return true;
}






int CMerkleTx::GetDepthInMainChain(CBlockIndex* &pindexRet) const
{
  if(hashBlock == 0 || nIndex == -1)
    return 0;

  // Find the block it claims to be in
  map<uint256, CBlockIndex*>::iterator mi = mapBlockIndex.find(hashBlock);
  if(mi == mapBlockIndex.end())
    return 0;
  CBlockIndex* pindex = (*mi).second;
  if(!pindex || !pindex->IsInMainChain())
    return 0;

  // Make sure the merkle branch connects to this block
  if(!fMerkleVerified)
  {
    if(CBlock::CheckMerkleBranch(GetHash(), vMerkleBranch, nIndex) != pindex->hashMerkleRoot)
      return 0;
    fMerkleVerified = true;
  }

  pindexRet = pindex;
  return pindexBest->nHeight - pindex->nHeight + 1;
}

//the burn depth is the amount of PoW blocks between (not including) the transaction and the best block
s32int CMerkleTx::GetBurnDepthInMainChain() const
{
  CBlockIndex *pindex = 0;
  GetDepthInMainChain(pindex);

  if(!pindex)
    return 0;

  return nPoWBlocksBetween(pindex->nHeight, pindexBest->nHeight);
}

bool CMerkleTx::IsBurnTxMature() const
{
  return GetBurnDepthInMainChain() >= BURN_MIN_CONFIRMS;
}

int CMerkleTx::GetBlocksToMaturity() const
{
  if(!(IsCoinBase() || IsCoinStake()))
    return 0;
  return max(0, (nCoinbaseMaturity+20) - GetDepthInMainChain());
}


bool CMerkleTx::AcceptToMemoryPool(CTxDB& txdb, bool fCheckInputs)
{
  if(fClient)
  {
    if(!IsInMainChain() && !ClientConnectInputs())
      return false;
    return CTransaction::AcceptToMemoryPool(txdb, false);
  }
  else
  {
    return CTransaction::AcceptToMemoryPool(txdb, fCheckInputs);
  }
}

bool CMerkleTx::AcceptToMemoryPool()
{
  CTxDB txdb("r");
  return AcceptToMemoryPool(txdb);
}



bool CWalletTx::AcceptWalletTransaction(CTxDB& txdb, bool fCheckInputs)
{

  {
    LOCK(mempool.cs);
    // Add previous supporting transactions first
    BOOST_FOREACH(CMerkleTx& tx, vtxPrev)
    {
      if(!(tx.IsCoinBase() || tx.IsCoinStake()))
      {
        uint256 hash = tx.GetHash();
        if(!mempool.exists(hash) && !txdb.ContainsTx(hash))
          tx.AcceptToMemoryPool(txdb, fCheckInputs);
      }
    }
    return AcceptToMemoryPool(txdb, fCheckInputs);
  }
  return false;
}

bool CWalletTx::AcceptWalletTransaction()
{
  CTxDB txdb("r");
  return AcceptWalletTransaction(txdb);
}

int CTxIndex::GetDepthInMainChain() const
{
  // Read block header
  CBlock block;
  if(!block.ReadFromDisk(pos.nFile, pos.nBlockPos, false))
    return 0;
  // Find the block in the index
  map<uint256, CBlockIndex*>::iterator mi = mapBlockIndex.find(block.GetHash());
  if(mi == mapBlockIndex.end())
    return 0;
  CBlockIndex* pindex = (*mi).second;
  if(!pindex || !pindex->IsInMainChain())
    return 0;
  return 1 + nBestHeight - pindex->nHeight;
}










//////////////////////////////////////////////////////////////////////////////
//
// CBlock and CBlockIndex
//

bool CBlock::ReadFromDisk(const CBlockIndex* pindex, bool fReadTransactions)
{
  if(!fReadTransactions)
  {
    *this = pindex->GetBlockHeader();
    return true;
  }
  if(!ReadFromDisk(pindex->nFile, pindex->nBlockPos, fReadTransactions))
    return false;
  if(GetHash() != pindex->GetBlockHash())
    return error("CBlock::ReadFromDisk() : GetHash() doesn't match index");
  return true;
}

uint256 static GetOrphanRoot(const CBlock *pblock)
{
  // Work back to the first block in the orphan chain
  while(mapOrphanBlocks.count(pblock->hashPrevBlock))
    pblock = mapOrphanBlocks[pblock->hashPrevBlock];
  return pblock->GetHash();
}

// slimcoin: find block wanted by given orphan block
uint256 WantedByOrphan(const CBlock *pblockOrphan)
{
  // Work back to the first block in the orphan chain
  while(mapOrphanBlocks.count(pblockOrphan->hashPrevBlock))
    pblockOrphan = mapOrphanBlocks[pblockOrphan->hashPrevBlock];
  return pblockOrphan->hashPrevBlock;
}

int64 GetProofOfWorkReward(u32int nBits, bool fProofOfBurn)
{
  const int64 maxSubsidy = fProofOfBurn ? MAX_MINT_PROOF_OF_BURN : MAX_MINT_PROOF_OF_WORK;
  CBigNum bnSubsidyLimit = maxSubsidy;
  CBigNum bnTarget;
  bnTarget.SetCompact(nBits); //expand the target hash for the current block
  CBigNum bnTargetLimit = fProofOfBurn ? bnProofOfBurnLimit : bnProofOfWorkLimit;
  bnTargetLimit.SetCompact(bnTargetLimit.GetCompact());

  CBigNum bnDiff = bnTargetLimit / bnTarget;

  int64 nSubsidy = 0;

  // slimcoin: subsidy is cut in half every 16x multiply of difficulty
  // A reasonably continuous curve is used to avoid shock to market
  // (nSubsidyLimit / nSubsidy) ** 4 == bnProofOfWorkLimit / bnTarget
  //
  // Human readable form:
  //
  // difficulty = bnTargetLimit / bnTarget
  //
  // Subsidy = (the max mint reward) / (the forth root of the difficulty)
  // nSubsidy = bnSubsidyLimit / (difficulty ^ 1/4)

  CBigNum bnLowerBound = CENT;
  CBigNum bnUpperBound = bnSubsidyLimit;

  while(bnLowerBound + CENT <= bnUpperBound)
  {
    CBigNum bnMidValue = (bnLowerBound + bnUpperBound) / 2;
    if(fDebug && GetBoolArg("-printcreation"))
      printf("GetProofOfWorkReward() : lower = %"PRI64d", upper = %"PRI64d", mid = %"PRI64d"\n",
             bnLowerBound.getuint64(), bnUpperBound.getuint64(), bnMidValue.getuint64());

    if(bnMidValue * bnMidValue * bnMidValue * bnMidValue * bnTargetLimit > 
       bnSubsidyLimit * bnSubsidyLimit * bnSubsidyLimit * bnSubsidyLimit * bnTarget)
    {
      bnUpperBound = bnMidValue;
    }else
      bnLowerBound = bnMidValue;
  }

  nSubsidy = bnUpperBound.getuint64();
  nSubsidy = (nSubsidy / CENT) * CENT;

  if(fDebug && GetBoolArg("-printcreation"))
    printf("GetProofOfWorkReward() : create = %s nBits = 0x%08x nSubsidy = %"PRI64d" return = %"PRI64d"\n",
           FormatMoney(nSubsidy).c_str(), nBits, nSubsidy, min(nSubsidy, maxSubsidy));

  return min(nSubsidy, maxSubsidy);
}

// slimcoin: miner's coin stake is rewarded based on coin age spent (coin-days)
int64 GetProofOfStakeReward(int64 nCoinAge, u32int nTime)
{
  int64 nRewardCoinYear = nTime > POB_POS_TARGET_SWITCH_TIME ? (10 * CENT) : CENT;  // creation amount per coin-year
  int64 nSubsidy = nCoinAge * nRewardCoinYear * 33 / (365 * 33 + 8);

  if(fDebug && GetBoolArg("-printcreation"))
    printf("GetProofOfStakeReward(): create = %s nCoinAge = %"PRI64d"\n", FormatMoney(nSubsidy).c_str(), nCoinAge);

  return nSubsidy;
}

//for now, use the PoW reward for PoB blocks
int64 GetProofOfBurnReward(u32int nBurnBits)
{
  return GetProofOfWorkReward(nBurnBits, true);
}

//Target time span for PoB is 30 PoW blocks
static const int64 nPoBTargetTimespan = 30;

static inline int64 getTargetTimespan(s32int lastNHeight)
{
  //the nTargetTimespan value cannot be too small since in the GetNextTargetRequired function,
  // the nActualSpacing variable could be negative, but we do not want to be multiplying
  // bnNew by a negative number in: bnNew *= ((nInterval - 1) * nTargetSpacing + nActualSpacing + nActualSpacing);

  //from 4259 and on, use new 6 hour retarget time
  if(lastNHeight >= 4258)
    return 6 * 60 * 60;
  else
    return 30 * 60; //old 30 minute retarget time
}

static const int64 nTargetSpacingWorkMax = 10 * STAKE_TARGET_SPACING; // 15 minutes

// select stake target limit according to hard-coded conditions
static inline CBigNum GetProofOfStakeLimit(u32int nTime)
{
  if(fTestNet || nTime > POB_POS_TARGET_SWITCH_TIME)
    return bnProofOfStakeLimit;

  return bnProofOfWorkLimit; // return bnProofOfWorkLimit of none matched
}

//
// maximum nBits value could possible be required nTime after
//
u32int ComputeMaxBits(CBigNum bnTargetLimit, u32int nBase, int64 nTime)
{
  CBigNum bnResult;
  bnResult.SetCompact(nBase);
  bnResult *= 2;
  while(nTime > 0 && bnResult < bnTargetLimit)
  {
    // Maximum 200% adjustment per day...
    bnResult *= 2;
    nTime -= 24 * 60 * 60;
  }

  if(bnResult > bnTargetLimit)
    bnResult = bnTargetLimit;

  return bnResult.GetCompact();
}

//
// minimum amount of work that could possibly be required nTime after
// minimum proof-of-work required was nBase
//
unsigned int ComputeMinWork(unsigned int nBase, int64 nTime)
{
  return ComputeMaxBits(bnProofOfWorkLimit, nBase, nTime);
}

//
// minimum amount of stake that could possibly be required nTime after
// minimum proof-of-stake required was nBase
//
unsigned int ComputeMinStake(unsigned int nBase, int64 nTime, unsigned int nBlockTime)
{
  return ComputeMaxBits(GetProofOfStakeLimit(nBlockTime), nBase, nTime);
}

// slimcoin: find last block index up to pindex
const CBlockIndex *GetLastBlockIndex(const CBlockIndex *pindex, bool fProofOfStake)
{
  //exclude PoB blocks from this calculation
  while(pindex && pindex->pprev && (pindex->IsProofOfStake() != fProofOfStake || pindex->IsProofOfBurn()))
    pindex = pindex->pprev;

  return pindex;
}

static u32int GetNextTargetRequired(const CBlockIndex *pindexLast, bool fProofOfStake)
{
  const CBigNum bnTargetLimit = fProofOfStake ? GetProofOfStakeLimit(pindexLast->nTime) : bnProofOfWorkLimit;

  if(pindexLast == NULL)
    return bnTargetLimit.GetCompact(); // genesis block

  const CBlockIndex* pindexPrev = GetLastBlockIndex(pindexLast, fProofOfStake);
  if(pindexPrev->pprev == NULL)
    return bnInitialHashTarget.GetCompact(); // first block
  const CBlockIndex* pindexPrevPrev = GetLastBlockIndex(pindexPrev->pprev, fProofOfStake);
  if(pindexPrevPrev->pprev == NULL)
    return bnInitialHashTarget.GetCompact(); // second block

  int64 nActualSpacing = pindexPrev->GetBlockTime() - pindexPrevPrev->GetBlockTime();

  // slimcoin: target change every block
  // slimcoin: retarget with exponential moving toward target spacing

  //use the last block's target as a seed
  CBigNum bnNew;
  bnNew.SetCompact(pindexPrev->nBits);

  int64 nTargetSpacing = fProofOfStake ? STAKE_TARGET_SPACING : 
    min(nTargetSpacingWorkMax, (int64) STAKE_TARGET_SPACING * (1 + pindexLast->nHeight - pindexPrev->nHeight));

  int64 nInterval = getTargetTimespan(pindexLast->nHeight) / nTargetSpacing;
  bnNew *= ((nInterval - 1) * nTargetSpacing + nActualSpacing + nActualSpacing);
  bnNew /= ((nInterval + 1) * nTargetSpacing);

  //we can't make it too easy
  if(bnNew > bnTargetLimit)
    bnNew = bnTargetLimit;

  return bnNew.GetCompact();
}

static u32int GetNextBurnTargetRequired(const CBlockIndex *pindexLast)
{
  //new protocol has a target PoW blocks between each PoB block
  if(fTestNet || pindexLast->nTime > POB_POS_TARGET_SWITCH_TIME)
  {
    const CBigNum bnTargetLimit = bnProofOfBurnLimit;

    if(pindexLast == NULL)
      return bnTargetLimit.GetCompact(); // genesis block

    //go backwards and find the last PoB block in the blockchain
    // once it exits, pindex is the last PoB block in the blockchain
    // and nPoW is the number of PoW blocks between pindexLast (inclusive) and the final pindex
    u32int nPoW = 0;
    const CBlockIndex *pindex = pindexLast;
    for(; pindex && !pindex->IsProofOfBurn(); pindex = pindex->pprev)
      if(pindex->IsProofOfWork())
        nPoW++;

    //if pindex is NULL, that means that there were no PoB blocks found and it got to the genesis block
    if(!pindex)
      return bnTargetLimit.GetCompact();

    //if there were no PoW blocks between, return the pindexLast's nBurnBits
    if(!nPoW)
      return pindexLast->nBurnBits;

    // slimcoin: target change every block
    // slimcoin: retarget with exponential moving toward target spacing

    //use the last PoB block's target as a seed
    CBigNum bnNew;
    bnNew.SetCompact(pindex->nBurnBits);

    //target spacing is 3 PoW blocks inbetween each PoB block
    const int64 nTargetSpacing = POB_TARGET_SPACING;
    const int64 nInterval = nPoBTargetTimespan / nTargetSpacing;

    bnNew *= ((nInterval - 1) * nTargetSpacing + nPoW + nPoW);
    bnNew /= ((nInterval + 1) * nTargetSpacing);

    //we can't make it too easy
    if(bnNew > bnTargetLimit)
      bnNew = bnTargetLimit;

    return bnNew.GetCompact();    

  }else{  //old prototcol is based off of nEffective burnt coins
    
    //go back BURN_MIN_CONFIRMS PoW blocks
    const CBlockIndex *pindexBack = pindexLast;

    s32int i;
    for(i = 0; i < BURN_MIN_CONFIRMS && pindexBack; i++)
    {
      //GetLastBlockIndex with false returns the last proof of work block index
      pindexBack = GetLastBlockIndex(pindexBack, false);
    }

    if(pindexBack == NULL || !pindexBack->nEffectiveBurnCoins)
      return CBigNum(0).GetCompact();

    CBigNum bnNew(~uint256(0));
  
    //formula for difficulty where take 0xffffff... and apply multiplier that is the same
    // as the hash burn data's, excluding the decay factor
    bnNew = (bnNew * BURN_HARDER_TARGET * BURN_CONSTANT) / pindexBack->nEffectiveBurnCoins;

    //we can't make it too easy
    if(bnNew > bnProofOfBurnLimit)
      bnNew = bnProofOfBurnLimit;
  
    return bnNew.GetCompact();

  }
}

bool CheckProofOfWork(uint256 hash, u32int nBits)
{
  CBigNum bnTarget;
  bnTarget.SetCompact(nBits);

  // Check range
  if(bnTarget <= 0 || bnTarget > bnProofOfWorkLimit)
    return error("CheckProofOfWork() : nBits below minimum work");

  // Check proof of work matches claimed amount
  if(hash > bnTarget.getuint256())
    return error("CheckProofOfWork() : hash doesn't match nBits");

  return true;
}

bool CheckProofOfBurnHash(uint256 hash, u32int nBurnBits)
{
  CBigNum bnTarget;
  bnTarget.SetCompact(nBurnBits);
  
  // Check range
  if(bnTarget <= 0 || bnTarget > bnProofOfBurnLimit)
    return error("CheckProofOfBurnHash() : nBurnBits below minimum work");

  // Check proof of work matches claimed amount
  if(hash > bnTarget.getuint256())
    return error("CheckProofOfBurnHash() : hash doesn't match nBurnBits\n\t%s > %s", 
                 hash.ToString().c_str(), bnTarget.getuint256().ToString().c_str());

  return true;
}

bool CBlock::CheckProofOfBurn() const
{
  if(!IsProofOfBurn())
    return false;

  //Get prev block index, this may fail durining initial block download,
  // if the previous block has not been received yet
  map<uint256, CBlockIndex*>::iterator mi = mapBlockIndex.find(hashPrevBlock);
  if(mi == mapBlockIndex.end() || !mi->second)
    return error("CheckProofOfBurn() : INFO: prev block not found");

  CBlockIndex *pindexPrev = mi->second;
  
  //test to see if this pindex can be reached through the linked list of block indexes
  // used internally in pindexByHeight()
  if(!pindexByHeight(pindexPrev->nHeight))
    return DoS(1, error("CheckProofOfBurn() : INFO: prev block not in main chain"));

  CBlockIndex *pBurnIndex = pindexByHeight(burnBlkHeight);
  if(!pBurnIndex)
    return DoS(1, error("CheckProofOfBurn() : INFO: burn block not found"));

  //failure to read a burn block may occur durring the initial block download
  CBlock burnBlock;
  //Read the block
  if(!burnBlock.ReadFromDisk(pBurnIndex))
    return DoS(1, error("CheckProofOfBurn() : INFO: prev block cannot be read"));

  //the previous block must be a PoW block
  if(!pindexPrev->IsProofOfWork())
    return DoS(100, error("CheckProofOfBurn() : previous block is not a proof-of-work block"));

  if(hashBurnBlock != pBurnIndex->GetBlockHash())
    return DoS(10, error("CheckProofOfBurn() : hashBurnBlock does not equal the pBurnIndex's block hash"));

  //see if this PoB block is past the intermediate burn hash update
  const bool fUseIntermediate = use_burn_hash_intermediate(pindexPrev->nHeight + 1);

  //Check proof-of-burn hash matches claimed amount
  // pindexPrev->nHeight + 1 since use_burn_hash_intermediate works on the height of the current block
  uint256 calculatedBurnHash = GetBurnHash(false);
  uint256 intermediateBurnHash = GetBurnHash(fUseIntermediate);

  if(!CheckProofOfBurnHash(calculatedBurnHash, nBurnBits))
    return DoS(100, error("CheckProofOfBurn() : proof-of-burn failed"));

  //the burn hash recorded in the block must equal the calculated burn hash
  // this is used in DoS detection
  if(burnHash != (fUseIntermediate ? intermediateBurnHash : calculatedBurnHash))
    return DoS(75, fUseIntermediate ? 
               error("CheckProofOfBurn() : proof-of-burn hashes do not match\n" //print the intermediate hash error
                     "\t %s != %s (burnHash, intermediateBurnHash)", 
                     burnHash.ToString().c_str(), 
                     intermediateBurnHash.ToString().c_str()) :
               error("CheckProofOfBurn() : proof-of-burn hashes do not match\n" //print the calculated hash error
                     "\t %s != %s (burnHash, calculatedBurnHash)", 
                     burnHash.ToString().c_str(), 
                     calculatedBurnHash.ToString().c_str()));


  if(!BurnCheckPubKeys(burnBlkHeight, burnCTx, burnCTxOut))
    return DoS(100, error("CheckProofOfBurn() : Public signatures do not match with burn transactions's"));

  return true;
}

// Return maximum amount of blocks that other nodes claim to have
int GetNumBlocksOfPeers()
{
  return std::max(cPeerBlockCounts.median(), Checkpoints::GetTotalBlocksEstimate());
}

bool IsInitialBlockDownload()
{
  if(pindexBest == NULL || nBestHeight < Checkpoints::GetTotalBlocksEstimate())
    return true;
  static int64 nLastUpdate;
  static CBlockIndex *pindexLastBest;
  if(pindexBest != pindexLastBest)
  {
    pindexLastBest = pindexBest;
    nLastUpdate = GetTime();
  }
  return (GetTime() - nLastUpdate < 10 &&
          pindexBest->GetBlockTime() < GetTime() - 24 * 60 * 60);
}

void static InvalidChainFound(CBlockIndex* pindexNew)
{
  if(pindexNew->bnChainTrust > bnBestInvalidTrust)
  {
    bnBestInvalidTrust = pindexNew->bnChainTrust;
    CTxDB().WriteBestInvalidTrust(bnBestInvalidTrust);
    MainFrameRepaint();
  }
  printf("InvalidChainFound: invalid block=%s  height=%d  trust=%s\n", pindexNew->GetBlockHash().ToString().substr(0,20).c_str(), pindexNew->nHeight, CBigNum(pindexNew->bnChainTrust).ToString().c_str());
  printf("InvalidChainFound:  current best=%s  height=%d  trust=%s\n", hashBestChain.ToString().substr(0,20).c_str(), nBestHeight, CBigNum(bnBestChainTrust).ToString().c_str());
  // slimcoin: should not enter safe mode for longer invalid chain
}

void CBlock::UpdateTime(const CBlockIndex* pindexPrev)
{
  nTime = max(GetBlockTime(), GetAdjustedTime());
}











bool CTransaction::DisconnectInputs(CTxDB& txdb)
{
  // Relinquish previous transactions' spent pointers
  if(!IsCoinBase())
  {
    BOOST_FOREACH(const CTxIn& txin, vin)
    {
      COutPoint prevout = txin.prevout;

      // Get prev txindex from disk
      CTxIndex txindex;
      if(!txdb.ReadTxIndex(prevout.hash, txindex))
        return error("DisconnectInputs() : ReadTxIndex failed");

      if(prevout.n >= txindex.vSpent.size())
        return error("DisconnectInputs() : prevout.n out of range");

      // Mark outpoint as not spent
      txindex.vSpent[prevout.n].SetNull();

      // Write back
      if(!txdb.UpdateTxIndex(prevout.hash, txindex))
        return error("DisconnectInputs() : UpdateTxIndex failed");
    }
  }

  // Remove transaction from index
  // This can fail if a duplicate of this transaction was in a chain that got
  // reorganized away. This is only possible if this transaction was completely
  // spent, so erasing it would be a no-op anway.
  txdb.EraseTxIndex(*this);

  return true;
}


bool CTransaction::FetchInputs(CTxDB& txdb, const map<uint256, CTxIndex>& mapTestPool,
                               bool fBlock, bool fMiner, MapPrevTx& inputsRet, bool& fInvalid)
{
  // FetchInputs can return false either because we just haven't seen some inputs
  // (in which case the transaction should be stored as an orphan)
  // or because the transaction is malformed (in which case the transaction should
  // be dropped).  If tx is definitely invalid, fInvalid will be set to true.
  fInvalid = false;

  if(IsCoinBase())
    return true; // Coinbase transactions have no inputs to fetch.

  for(unsigned int i = 0; i < vin.size(); i++)
  {
    COutPoint prevout = vin[i].prevout;
    if(inputsRet.count(prevout.hash))
      continue; // Got it already

    // Read txindex
    CTxIndex& txindex = inputsRet[prevout.hash].first;
    bool fFound = true;
    if((fBlock || fMiner) && mapTestPool.count(prevout.hash))
    {
      // Get txindex from current proposed changes
      txindex = mapTestPool.find(prevout.hash)->second;
    }
    else
    {
      // Read txindex from txdb
      fFound = txdb.ReadTxIndex(prevout.hash, txindex);
    }
    if(!fFound && (fBlock || fMiner))
      return fMiner ? false : error("FetchInputs() : %s prev tx %s index entry not found", GetHash().ToString().substr(0,10).c_str(),  prevout.hash.ToString().substr(0,10).c_str());

    // Read txPrev
    CTransaction& txPrev = inputsRet[prevout.hash].second;
    if(!fFound || txindex.pos == CDiskTxPos(1,1,1))
    {
      // Get prev tx from single transactions in memory
      {
        LOCK(mempool.cs);
        if(!mempool.exists(prevout.hash))
          return error("FetchInputs() : %s mempool Tx prev not found %s", GetHash().ToString().substr(0,10).c_str(),  prevout.hash.ToString().substr(0,10).c_str());
        txPrev = mempool.lookup(prevout.hash);
      }
      if(!fFound)
        txindex.vSpent.resize(txPrev.vout.size());
    }
    else
    {
      // Get prev tx from disk
      if(!txPrev.ReadFromDisk(txindex.pos))
        return error("FetchInputs() : %s ReadFromDisk prev tx %s failed", GetHash().ToString().substr(0,10).c_str(),  prevout.hash.ToString().substr(0,10).c_str());
    }
  }

  // Make sure all prevout.n's are valid:
  for(unsigned int i = 0; i < vin.size(); i++)
  {
    const COutPoint prevout = vin[i].prevout;
    assert(inputsRet.count(prevout.hash) != 0);
    const CTxIndex& txindex = inputsRet[prevout.hash].first;
    const CTransaction& txPrev = inputsRet[prevout.hash].second;
    if(prevout.n >= txPrev.vout.size() || prevout.n >= txindex.vSpent.size())
    {
      // Revisit this if/when transaction replacement is implemented and allows
      // adding inputs:
      fInvalid = true;
      return DoS(100, error("FetchInputs() : %s prevout.n out of range %d %d %d prev tx %s\n%s", GetHash().ToString().substr(0,10).c_str(), prevout.n, txPrev.vout.size(), txindex.vSpent.size(), prevout.hash.ToString().substr(0,10).c_str(), txPrev.ToString().c_str()));
    }
  }

  return true;
}

const CTxOut& CTransaction::GetOutputFor(const CTxIn& input, const MapPrevTx& inputs) const
{
  MapPrevTx::const_iterator mi = inputs.find(input.prevout.hash);
  if(mi == inputs.end())
    throw std::runtime_error("CTransaction::GetOutputFor() : prevout.hash not found");

  const CTransaction& txPrev = (mi->second).second;
  if(input.prevout.n >= txPrev.vout.size())
    throw std::runtime_error("CTransaction::GetOutputFor() : prevout.n out of range");

  return txPrev.vout[input.prevout.n];
}

int64 CTransaction::GetValueIn(const MapPrevTx& inputs) const
{
  if(IsCoinBase())
    return 0;

  int64 nResult = 0;
  for(unsigned int i = 0; i < vin.size(); i++)
  {
    nResult += GetOutputFor(vin[i], inputs).nValue;
  }
  return nResult;

}

unsigned int CTransaction::GetP2SHSigOpCount(const MapPrevTx& inputs) const
{
  if(IsCoinBase())
    return 0;

  unsigned int nSigOps = 0;
  for(unsigned int i = 0; i < vin.size(); i++)
  {
    const CTxOut& prevout = GetOutputFor(vin[i], inputs);
    if(prevout.scriptPubKey.IsPayToScriptHash())
      nSigOps += prevout.scriptPubKey.GetSigOpCount(vin[i].scriptSig);
  }
  return nSigOps;
}

bool CTransaction::ConnectInputs(CTxDB& txdb, MapPrevTx inputs,
                                 map<uint256, CTxIndex>& mapTestPool, const CDiskTxPos& posThisTx,
                                 const CBlockIndex* pindexBlock, bool fBlock, 
                                 bool fMiner, bool fStrictPayToScriptHash)
{
  // Take over previous transactions' spent pointers
  // fBlock is true when this is called from AcceptBlock when a new best-block is added to the blockchain
  // fMiner is true when called from the internal bitcoin miner
  // ... both are false when called from CTransaction::AcceptToMemoryPool
  if(!IsCoinBase())
  {
    int64 nValueIn = 0;
    int64 nFees = 0;
    for(unsigned int i = 0; i < vin.size(); i++)
    {
      COutPoint prevout = vin[i].prevout;
      assert(inputs.count(prevout.hash) > 0);
      CTxIndex& txindex = inputs[prevout.hash].first;
      CTransaction& txPrev = inputs[prevout.hash].second;

      if(prevout.n >= txPrev.vout.size() || prevout.n >= txindex.vSpent.size())
        return DoS(100, error("ConnectInputs() : %s prevout.n out of range %d %d %d prev tx %s\n%s", GetHash().ToString().substr(0,10).c_str(), prevout.n, txPrev.vout.size(), txindex.vSpent.size(), prevout.hash.ToString().substr(0,10).c_str(), txPrev.ToString().c_str()));

      // If prev is coinbase/coinstake, check that it's matured
      if(txPrev.IsCoinBase() || txPrev.IsCoinStake())
        for(const CBlockIndex* pindex = pindexBlock; pindex && pindexBlock->nHeight - pindex->nHeight < nCoinbaseMaturity; pindex = pindex->pprev)
          if(pindex->nBlockPos == txindex.pos.nBlockPos && pindex->nFile == txindex.pos.nFile)
            return error("ConnectInputs() : tried to spend coinbase/coinstake at depth %d", pindexBlock->nHeight - pindex->nHeight);

      // slimcoin: check transaction timestamp
      if(txPrev.nTime > nTime)
        return DoS(100, error("ConnectInputs() : transaction timestamp earlier than input transaction"));

      // Check for negative or overflow input values
      nValueIn += txPrev.vout[prevout.n].nValue;
      if(!MoneyRange(txPrev.vout[prevout.n].nValue) || !MoneyRange(nValueIn))
        return DoS(100, error("ConnectInputs() : txin values out of range"));

    }
    // The first loop above does all the inexpensive checks.
    // Only if ALL inputs pass do we perform expensive ECDSA signature checks.
    // Helps prevent CPU exhaustion attacks.
    for(unsigned int i = 0; i < vin.size(); i++)
    {
      COutPoint prevout = vin[i].prevout;
      assert(inputs.count(prevout.hash) > 0);
      CTxIndex& txindex = inputs[prevout.hash].first;
      CTransaction& txPrev = inputs[prevout.hash].second;

      // Check for conflicts (double-spend)
      // This doesn't trigger the DoS code on purpose; if it did, it would make it easier
      // for an attacker to attempt to split the network.
      if(!txindex.vSpent[prevout.n].IsNull())
        return fMiner ? false : error("ConnectInputs() : %s prev tx already used at %s", GetHash().ToString().substr(0,10).c_str(), txindex.vSpent[prevout.n].ToString().c_str());

      // Skip ECDSA signature verification when connecting blocks (fBlock=true)
      // before the last blockchain checkpoint. This is safe because block merkle hashes are
      // still computed and checked, and any change will be caught at the next checkpoint.
      if(!(fBlock && (nBestHeight < Checkpoints::GetTotalBlocksEstimate())))
      {
        // Verify signature
        if(!VerifySignature(txPrev, *this, i, fStrictPayToScriptHash, 0))
        {
          // only during transition phase for P2SH: do not invoke anti-DoS code for
          // potentially old clients relaying bad P2SH transactions
          if(fStrictPayToScriptHash && VerifySignature(txPrev, *this, i, false, 0))
            return error("ConnectInputs() : %s P2SH VerifySignature failed", GetHash().ToString().substr(0,10).c_str());

          return DoS(100,error("ConnectInputs() : %s VerifySignature failed", GetHash().ToString().substr(0,10).c_str()));
        }
      }

      // Mark outpoints as spent
      txindex.vSpent[prevout.n] = posThisTx;

      // Write back
      if(fBlock || fMiner)
      {
        mapTestPool[prevout.hash] = txindex;
      }
    }

    if(IsCoinStake())
    {
      // slimcoin: coin stake tx earns reward instead of paying fee
      uint64 nCoinAge;
      if(!GetCoinAge(txdb, nCoinAge))
        return error("ConnectInputs() : %s unable to get coin age for coinstake", GetHash().ToString().substr(0,10).c_str());
      int64 nStakeReward = GetValueOut() - nValueIn;
      if(nStakeReward > GetProofOfStakeReward(nCoinAge, nTime) - GetMinFee() + MIN_TX_FEE)
        return DoS(100, error("ConnectInputs() : %s stake reward exceeded", GetHash().ToString().substr(0,10).c_str()));
    }
    else
    {
      if(nValueIn < GetValueOut())
        return DoS(100, error("ConnectInputs() : %s value in < value out", GetHash().ToString().substr(0,10).c_str()));

      // Tally transaction fees
      int64 nTxFee = nValueIn - GetValueOut();
      if(nTxFee < 0)
        return DoS(100, error("ConnectInputs() : %s nTxFee < 0", GetHash().ToString().substr(0,10).c_str()));
      // slimcoin: enforce transaction fees for every block
      if(nTxFee < GetMinFee())
        return fBlock? DoS(100, error("ConnectInputs() : %s not paying required fee=%s, paid=%s", GetHash().ToString().substr(0,10).c_str(), FormatMoney(GetMinFee()).c_str(), FormatMoney(nTxFee).c_str())) : false;
      nFees += nTxFee;
      if(!MoneyRange(nFees))
        return DoS(100, error("ConnectInputs() : nFees out of range"));
    }
  }

  return true;
}


bool CTransaction::ClientConnectInputs()
{
  if(IsCoinBase())
    return false;

  // Take over previous transactions' spent pointers
  {
    LOCK(mempool.cs);
    int64 nValueIn = 0;
    for(unsigned int i = 0; i < vin.size(); i++)
    {
      // Get prev tx from single transactions in memory
      COutPoint prevout = vin[i].prevout;
      if(!mempool.exists(prevout.hash))
        return false;
      CTransaction& txPrev = mempool.lookup(prevout.hash);

      if(prevout.n >= txPrev.vout.size())
        return false;

      // Verify signature
      if(!VerifySignature(txPrev, *this, i, true, 0))
        return error("ConnectInputs() : VerifySignature failed");

      ///// this is redundant with the mempool.mapNextTx stuff,
      ///// not sure which I want to get rid of
      ///// this has to go away now that posNext is gone
      // // Check for conflicts
      // if(!txPrev.vout[prevout.n].posNext.IsNull())
      //     return error("ConnectInputs() : prev tx already used");
      //
      // // Flag outpoints as used
      // txPrev.vout[prevout.n].posNext = posThisTx;

      nValueIn += txPrev.vout[prevout.n].nValue;

      if(!MoneyRange(txPrev.vout[prevout.n].nValue) || !MoneyRange(nValueIn))
        return error("ClientConnectInputs() : txin values out of range");
    }
    if(GetValueOut() > nValueIn)
      return false;
  }

  return true;
}




bool CBlock::DisconnectBlock(CTxDB& txdb, CBlockIndex* pindex)
{
  // Disconnect in reverse order
  for(int i = vtx.size()-1; i >= 0; i--)
    if(!vtx[i].DisconnectInputs(txdb))
      return false;

  // Update block index on disk without changing it in memory.
  // The memory index structure will be changed after the db commits.
  if(pindex->pprev)
  {
    CDiskBlockIndex blockindexPrev(pindex->pprev);
    blockindexPrev.hashNext = 0;
    if(!txdb.WriteBlockIndex(blockindexPrev))
      return error("DisconnectBlock() : WriteBlockIndex failed");
  }

  // slimcoin: clean up wallet after disconnecting coinstake
  BOOST_FOREACH(CTransaction& tx, vtx)
    SyncWithWallets(tx, this, false, false);

  return true;
}

bool CBlock::ConnectBlock(CTxDB& txdb, CBlockIndex* pindex)
{
  // Check it again in case a previous version let a bad block in
  if(!CheckBlock())
    return false;

  // Do not allow blocks that contain transactions which 'overwrite' older transactions,
  // unless those are already completely spent.
  // If such overwrites are allowed, coinbases and transactions depending upon those
  // can be duplicated to remove the ability to spend the first instance -- even after
  // being sent to another address.
  // See BIP30 and http://r6.ca/blog/20120206T005236Z.html for more information.
  // This logic is not necessary for memory pool transactions, as AcceptToMemoryPool
  // already refuses previously-known transaction id's entirely.
  // This rule applies to all blocks whose timestamp is after March 15, 2012, 0:00 UTC.
  // On testnet it is enabled as of februari 20, 2012, 0:00 UTC.
  if(pindex->nTime > 1331769600 || (fTestNet && pindex->nTime > 1329696000))
  {
    BOOST_FOREACH(CTransaction& tx, vtx)
    {
      CTxIndex txindexOld;
      if(txdb.ReadTxIndex(tx.GetHash(), txindexOld))
      {
        BOOST_FOREACH(CDiskTxPos &pos, txindexOld.vSpent)
          if(pos.IsNull())
            return false;
      }
    }
  }

  // BIP16 didn't become active until Apr 1 2012 (Feb 15 on testnet)
  int64 nBIP16SwitchTime = fTestNet ? 1329264000 : 1333238400;
  bool fStrictPayToScriptHash = (pindex->nTime >= nBIP16SwitchTime);

  //// issue here: it doesn't know the version
  unsigned int nTxPos = pindex->nBlockPos + ::GetSerializeSize(CBlock(), SER_DISK, CLIENT_VERSION) - (2 * GetSizeOfCompactSize(0)) + GetSizeOfCompactSize(vtx.size());

  map<uint256, CTxIndex> mapQueuedChanges;
  int64 nFees = 0;
  int64 nValueIn = 0;
  int64 nValueOut = 0;
  unsigned int nSigOps = 0;
  BOOST_FOREACH(CTransaction& tx, vtx)
  {
    nSigOps += tx.GetLegacySigOpCount();
    if(nSigOps > MAX_BLOCK_SIGOPS)
      return DoS(100, error("ConnectBlock() : too many sigops"));

    CDiskTxPos posThisTx(pindex->nFile, pindex->nBlockPos, nTxPos);
    nTxPos += ::GetSerializeSize(tx, SER_DISK, CLIENT_VERSION);

    MapPrevTx mapInputs;
    if(tx.IsCoinBase())
      nValueOut += tx.GetValueOut();
    else
    {
      bool fInvalid;
      if(!tx.FetchInputs(txdb, mapQueuedChanges, true, false, mapInputs, fInvalid))
        return false;

      if(fStrictPayToScriptHash)
      {
        // Add in sigops done by pay-to-script-hash inputs;
        // this is to prevent a "rogue miner" from creating
        // an incredibly-expensive-to-validate block.
        nSigOps += tx.GetP2SHSigOpCount(mapInputs);
        if(nSigOps > MAX_BLOCK_SIGOPS)
          return DoS(100, error("ConnectBlock() : too many sigops"));
      }

      int64 nTxValueIn = tx.GetValueIn(mapInputs);
      int64 nTxValueOut = tx.GetValueOut();
      nValueIn += nTxValueIn;
      nValueOut += nTxValueOut;
      if(!tx.IsCoinStake())
        nFees += nTxValueIn - nTxValueOut;

      if(!tx.ConnectInputs(txdb, mapInputs, mapQueuedChanges, posThisTx, pindex, true, false, fStrictPayToScriptHash))
        return false;
    }

    mapQueuedChanges[tx.GetHash()] = CTxIndex(posThisTx, tx.vout.size());
  }

  // slimcoin: track money supply and mint amount info
  pindex->nMint = nValueOut - nValueIn + nFees;
  pindex->nMoneySupply = (pindex->pprev? pindex->pprev->nMoneySupply : 0) + nValueOut - nValueIn;
  if(!txdb.WriteBlockIndex(CDiskBlockIndex(pindex)))
    return error("Connect() : WriteBlockIndex for pindex failed");

  // Write queued txindex changes
  for(map<uint256, CTxIndex>::iterator mi = mapQueuedChanges.begin(); mi != mapQueuedChanges.end(); ++mi)
  {
    if(!txdb.UpdateTxIndex((*mi).first, (*mi).second))
      return error("ConnectBlock() : UpdateTxIndex failed");
  }

  // slimcoin: fees are not collected by miners as in bitcoin
  // slimcoin: fees are destroyed to compensate the entire network
  if(fDebug && GetBoolArg("-printcreation"))
    printf("ConnectBlock() : destroy=%s nFees=%"PRI64d"\n", FormatMoney(nFees).c_str(), nFees);

  // Update block index on disk without changing it in memory.
  // The memory index structure will be changed after the db commits.
  if(pindex->pprev)
  {
    CDiskBlockIndex blockindexPrev(pindex->pprev);
    blockindexPrev.hashNext = pindex->GetBlockHash();
    if(!txdb.WriteBlockIndex(blockindexPrev))
      return error("ConnectBlock() : WriteBlockIndex for blockindexPrev failed");
  }

  // Watch for transactions paying to me
  BOOST_FOREACH(CTransaction& tx, vtx)
    SyncWithWallets(tx, this, true);

  return true;
}

bool Reorganize(CTxDB& txdb, CBlockIndex* pindexNew)
{
  printf("REORGANIZE\n");

  // Find the fork
  CBlockIndex* pfork = pindexBest;
  CBlockIndex* plonger = pindexNew;
  while(pfork != plonger)
  {
    while(plonger->nHeight > pfork->nHeight)
      if(!(plonger = plonger->pprev))
        return error("Reorganize() : plonger->pprev is null");
    if(pfork == plonger)
      break;
    if(!(pfork = pfork->pprev))
      return error("Reorganize() : pfork->pprev is null");
  }

  // List of what to disconnect
  vector<CBlockIndex*> vDisconnect;
  for(CBlockIndex* pindex = pindexBest; pindex != pfork; pindex = pindex->pprev)
    vDisconnect.push_back(pindex);

  // List of what to connect
  vector<CBlockIndex*> vConnect;
  for(CBlockIndex* pindex = pindexNew; pindex != pfork; pindex = pindex->pprev)
    vConnect.push_back(pindex);
  reverse(vConnect.begin(), vConnect.end());

  printf("REORGANIZE: Disconnect %i blocks; %s..%s\n", vDisconnect.size(), pfork->GetBlockHash().ToString().substr(0,20).c_str(), pindexBest->GetBlockHash().ToString().substr(0,20).c_str());
  printf("REORGANIZE: Connect %i blocks; %s..%s\n", vConnect.size(), pfork->GetBlockHash().ToString().substr(0,20).c_str(), pindexNew->GetBlockHash().ToString().substr(0,20).c_str());

  // Disconnect shorter branch
  vector<CTransaction> vResurrect;
  BOOST_FOREACH(CBlockIndex *pindex, vDisconnect)
  {
    CBlock block;
    if(!block.ReadFromDisk(pindex))
      return error("Reorganize() : ReadFromDisk for disconnect failed");
    if(!block.DisconnectBlock(txdb, pindex))
      return error("Reorganize() : DisconnectBlock %s failed", 
                   pindex->GetBlockHash().ToString().substr(0,20).c_str());

    // Queue memory transactions to resurrect
    BOOST_FOREACH(const CTransaction &tx, block.vtx)
      if(!(tx.IsCoinBase() || tx.IsCoinStake()))
        vResurrect.push_back(tx);
  }

  // Connect longer branch
  vector<CTransaction> vDelete;
  for(u32int i = 0; i < vConnect.size(); i++)
  {
    CBlockIndex *pindex = vConnect[i];
    CBlock block;
    if(!block.ReadFromDisk(pindex))
      return error("Reorganize() : ReadFromDisk for connect failed");
    if(!block.ConnectBlock(txdb, pindex))
    {
      // Invalid block
      txdb.TxnAbort();
      return error("Reorganize() : ConnectBlock %s failed", 
                   pindex->GetBlockHash().ToString().substr(0,20).c_str());
    }

    // Queue memory transactions to delete
    BOOST_FOREACH(const CTransaction &tx, block.vtx)
      vDelete.push_back(tx);
  }

  if(!txdb.WriteHashBestChain(pindexNew->GetBlockHash()))
    return error("Reorganize() : WriteHashBestChain failed");

  // Make sure it's successfully written to disk before changing memory structure
  if(!txdb.TxnCommit())
    return error("Reorganize() : TxnCommit failed");

  // Disconnect shorter branch
  BOOST_FOREACH(CBlockIndex *pindex, vDisconnect)
    if(pindex->pprev)
      pindex->pprev->pnext = NULL;

  // Connect longer branch
  BOOST_FOREACH(CBlockIndex *pindex, vConnect)
    if(pindex->pprev)
      pindex->pprev->pnext = pindex;

  // Resurrect memory transactions that were in the disconnected branch
  BOOST_FOREACH(CTransaction &tx, vResurrect)
    tx.AcceptToMemoryPool(txdb, false);

  // Delete redundant memory transactions that are in the connected branch
  BOOST_FOREACH(CTransaction &tx, vDelete)
    mempool.remove(tx);

  printf("REORGANIZE: done\n");

  return true;
}


static void runCommand(std::string strCommand)
{
  int nErr = ::system(strCommand.c_str());
  if(nErr)
    printf("runCommand error: system(%s) returned %d\n", strCommand.c_str(), nErr);
}

// Called from inside SetBestChain: attaches a block to the new best chain being built
bool CBlock::SetBestChainInner(CTxDB& txdb, CBlockIndex *pindexNew)
{
  uint256 hash = GetHash();

  // Adding to current best branch
  if(!ConnectBlock(txdb, pindexNew) || !txdb.WriteHashBestChain(hash))
  {
    txdb.TxnAbort();
    InvalidChainFound(pindexNew);
    return false;
  }
  if(!txdb.TxnCommit())
    return error("SetBestChain() : TxnCommit failed");

  // Add to current best branch
  pindexNew->pprev->pnext = pindexNew;

  // Delete redundant memory transactions
  BOOST_FOREACH(CTransaction& tx, vtx)
    mempool.remove(tx);

  return true;
}

bool CBlock::SetBestChain(CTxDB& txdb, CBlockIndex* pindexNew)
{
  uint256 hash = GetHash();

  if(!txdb.TxnBegin())
    return error("SetBestChain() : TxnBegin failed");

  if(pindexGenesisBlock == NULL && hash == hashGenesisBlock)
  {
    txdb.WriteHashBestChain(hash);
    if(!txdb.TxnCommit())
      return error("SetBestChain() : TxnCommit failed");
    pindexGenesisBlock = pindexNew;
  }else if(hashPrevBlock == hashBestChain)
  {
    if(!SetBestChainInner(txdb, pindexNew))
      return error("SetBestChain() : SetBestChainInner failed");
  }else{
    // the first block in the new chain that will cause it to become the new best chain
    CBlockIndex *pindexIntermediate = pindexNew;

    // list of blocks that need to be connected afterwards
    std::vector<CBlockIndex*> vpindexSecondary;

    // Reorganize is costly in terms of db load, as it works in a single db transaction.
    // Try to limit how much needs to be done inside
    while(pindexIntermediate->pprev && pindexIntermediate->pprev->bnChainTrust > pindexBest->bnChainTrust)
    {
      vpindexSecondary.push_back(pindexIntermediate);
      pindexIntermediate = pindexIntermediate->pprev;
    }

    if(!vpindexSecondary.empty())
      printf("Postponing %i reconnects\n", vpindexSecondary.size());

    // Switch to new best branch
    if(!Reorganize(txdb, pindexIntermediate))
    {
      txdb.TxnAbort();
      InvalidChainFound(pindexNew);
      return error("SetBestChain() : Reorganize failed");
    }

    // Connect futher blocks
    BOOST_REVERSE_FOREACH(CBlockIndex *pindex, vpindexSecondary)
    {
      CBlock block;
      if(!block.ReadFromDisk(pindex))
      {
        printf("SetBestChain() : ReadFromDisk failed\n");
        break;
      }
      if(!txdb.TxnBegin()) {
        printf("SetBestChain() : TxnBegin 2 failed\n");
        break;
      }
      // errors now are not fatal, we still did a reorganisation to a new chain in a valid way
      if(!block.SetBestChainInner(txdb, pindex))
        break;
    }
  }

  // Update best block in wallet (so we can detect restored wallets)
  bool fIsInitialDownload = IsInitialBlockDownload();
  if(!fIsInitialDownload)
  {
    const CBlockLocator locator(pindexNew);
    ::SetBestChain(locator);
  }

  // New best block
  hashBestChain = hash;
  pindexBest = pindexNew;
  nBestHeight = pindexBest->nHeight;
  bnBestChainTrust = pindexNew->bnChainTrust;
  nTimeBestReceived = GetTime();
  nTransactionsUpdated++;
  printf("SetBestChain: new best=%s  height=%d  trust=%s  moneysupply=%s nEffectiveBurnCoins=%s\n", 
         hashBestChain.ToString().substr(0,20).c_str(), nBestHeight, bnBestChainTrust.ToString().c_str(), 
         FormatMoney(pindexBest->nMoneySupply).c_str(), FormatMoney(pindexBest->nEffectiveBurnCoins).c_str());

  std::string strCmd = GetArg("-blocknotify", "");

  if(!fIsInitialDownload && !strCmd.empty())
  {
    boost::replace_all(strCmd, "%s", hashBestChain.GetHex());
    boost::thread t(runCommand, strCmd); // thread runs free
  }

  return true;
}


// slimcoin: total coin age spent in transaction, in the unit of coin-days.
// Only those coins meeting minimum age requirement counts. As those
// transactions not in main chain are not currently indexed so we
// might not find out about their coin age. Older transactions are 
// guaranteed to be in main chain by sync-checkpoint. This rule is
// introduced to help nodes establish a consistent view of the coin
// age (trust score) of competing branches.
bool CTransaction::GetCoinAge(CTxDB& txdb, uint64& nCoinAge) const
{
  CBigNum bnCentSecond = 0;  // coin age in the unit of cent-seconds
  nCoinAge = 0;

  if(IsCoinBase())
    return true;

  BOOST_FOREACH(const CTxIn& txin, vin)
  {
    // First try finding the previous transaction in database
    CTransaction txPrev;
    CTxIndex txindex;
    if(!txPrev.ReadFromDisk(txdb, txin.prevout, txindex))
      continue;  // previous transaction not in main chain

    if(nTime < txPrev.nTime)
      return false;  // Transaction timestamp violation

    // Read block header
    CBlock block;
    if(!block.ReadFromDisk(txindex.pos.nFile, txindex.pos.nBlockPos, false))
      return false; // unable to read block of previous transaction

    if(block.GetBlockTime() + nStakeMinAge > nTime)
      continue; // only count coins meeting min age requirement

    int64 nValueIn = txPrev.vout[txin.prevout.n].nValue;
    bnCentSecond += CBigNum(nValueIn) * (nTime - txPrev.nTime) / CENT;

    if(fDebug && GetBoolArg("-printcoinage"))
      printf("coin age nValueIn=%-12I64d nTimeDiff=%d bnCentSecond=%s\n",
             nValueIn, nTime - txPrev.nTime, bnCentSecond.ToString().c_str());
  }

  CBigNum bnCoinDay = bnCentSecond * CENT / COIN / (24 * 60 * 60);
  if(fDebug && GetBoolArg("-printcoinage"))
    printf("coin age bnCoinDay=%s\n", bnCoinDay.ToString().c_str());

  nCoinAge = bnCoinDay.getuint64();
  return true;
}

// slimcoin: total coin age spent in block, in the unit of coin-days.
bool CBlock::GetCoinAge(uint64& nCoinAge) const
{
  nCoinAge = 0;

  CTxDB txdb("r");
  BOOST_FOREACH(const CTransaction& tx, vtx)
  {
    uint64 nTxCoinAge;
    if(tx.GetCoinAge(txdb, nTxCoinAge))
      nCoinAge += nTxCoinAge;
    else
      return false;
  }

  if(!nCoinAge) // block coin age minimum 1 coin-day
    nCoinAge = 1;

  if(fDebug && GetBoolArg("-printcoinage"))
    printf("block coin age total nCoinDays=%"PRI64d"\n", nCoinAge);

  return true;
}

//MYTODO: 
//
//Empty...
//

bool CBlock::AddToBlockIndex(unsigned int nFile, unsigned int nBlockPos)
{
  // Check for duplicate
  uint256 hash = GetHash();
  if(mapBlockIndex.count(hash))
    return error("AddToBlockIndex() : %s already exists", hash.ToString().substr(0,20).c_str());

  // Construct new block index object
  CBlockIndex *pindexNew = new CBlockIndex(nFile, nBlockPos, *this);
  if(!pindexNew)
    return error("AddToBlockIndex() : new CBlockIndex failed");

  pindexNew->phashBlock = &hash;
  map<uint256, CBlockIndex*>::iterator miPrev = mapBlockIndex.find(hashPrevBlock);
  if(miPrev != mapBlockIndex.end())
  {
    pindexNew->pprev = (*miPrev).second;
    pindexNew->nHeight = pindexNew->pprev->nHeight + 1;
  }

  // slimcoin: compute chain trust score
  pindexNew->bnChainTrust = (pindexNew->pprev ? pindexNew->pprev->bnChainTrust : 0) + pindexNew->GetBlockTrust();

  // slimcoin: compute stake entropy bit for stake modifier
  if(!pindexNew->SetStakeEntropyBit(GetStakeEntropyBit()))
    return error("AddToBlockIndex() : SetStakeEntropyBit() failed");

  // slimcoin: record proof-of-stake hash value
  if(pindexNew->IsProofOfStake())
  {
    if(!mapProofOfStake.count(hash))
      return error("AddToBlockIndex() : hashProofOfStake not found in map");
    pindexNew->hashProofOfStake = mapProofOfStake[hash];
  }

  // slimcoin: compute stake modifier
  uint64 nStakeModifier = 0;
  bool fGeneratedStakeModifier = false;

  if(!ComputeNextStakeModifier(pindexNew->pprev, nStakeModifier, fGeneratedStakeModifier))
    return error("AddToBlockIndex() : ComputeNextStakeModifier() failed");

  pindexNew->SetStakeModifier(nStakeModifier, fGeneratedStakeModifier);
  pindexNew->nStakeModifierChecksum = GetStakeModifierChecksum(pindexNew);

  if(!CheckStakeModifierCheckpoints(pindexNew->nHeight, pindexNew->nStakeModifierChecksum))
    return error("AddToBlockIndex() : Rejected by stake modifier checkpoint height=%d, modifier=0x%016"PRI64x", modifierChecksum 0x%09x", pindexNew->nHeight, nStakeModifier, pindexNew->nStakeModifierChecksum);

  // Add to mapBlockIndex
  map<uint256, CBlockIndex*>::iterator mi = mapBlockIndex.insert(make_pair(hash, pindexNew)).first;
  if(pindexNew->IsProofOfStake())
    setStakeSeen.insert(pindexNew->GetProofOfStake());
  else if(pindexNew->IsProofOfBurn())
    setBurnSeen.insert(pindexNew->GetProofOfBurn());

  pindexNew->phashBlock = &((*mi).first);

  // Write to disk block index
  CTxDB txdb;
  if(!txdb.TxnBegin())
    return false;
  
  txdb.WriteBlockIndex(CDiskBlockIndex(pindexNew));
  if(!txdb.TxnCommit())
    return false;

  // New best
  if(pindexNew->bnChainTrust > bnBestChainTrust)
    if(!SetBestChain(txdb, pindexNew))
      return false;

  txdb.Close();

  if(pindexNew == pindexBest)
  {
    // Notify UI to display prev block's coinbase if it was ours
    static uint256 hashPrevBestCoinBase;
    UpdatedTransaction(hashPrevBestCoinBase);
    hashPrevBestCoinBase = vtx[0].GetHash();
  }

  MainFrameRepaint();
  return true;
}

bool CBlock::CheckBlock() const
{
  // These are checks that are independent of context
  // that can be verified before saving an orphan block.

  // Size limits
  if(vtx.empty() || vtx.size() > MAX_BLOCK_SIZE || 
     ::GetSerializeSize(*this, SER_NETWORK, PROTOCOL_VERSION) > MAX_BLOCK_SIZE)
    return DoS(100, error("CheckBlock() : size limits failed"));

  // Check proof of work matches claimed amount
  if(IsProofOfWork() && !CheckProofOfWork(GetHash(), nBits))
    return DoS(50, error("CheckBlock() : proof-of-work failed"));

  // Check timestamp
  if(GetBlockTime() > GetAdjustedTime() + nMaxClockDrift)
    return error("CheckBlock() : block timestamp too far in the future");

  // First transaction must be coinbase, the rest must not be
  if(vtx.empty() || !vtx[0].IsCoinBase())
    return DoS(100, error("CheckBlock() : first tx is not coinbase"));
  for(unsigned int i = 1; i < vtx.size(); i++)
    if(vtx[i].IsCoinBase())
      return DoS(100, error("CheckBlock() : more than one coinbase"));

  // slimcoin: only the second transaction can be the optional coinstake
  for(int i = 2; i < vtx.size(); i++)
    if(vtx[i].IsCoinStake())
      return DoS(100, error("CheckBlock() : coinstake in wrong position"));

  // slimcoin: coinbase output should be empty if proof-of-stake block
  if(IsProofOfStake() && (vtx[0].vout.size() != 1 || !vtx[0].vout[0].IsEmpty()))
    return error("CheckBlock() : coinbase output not empty for proof-of-stake block");

  // Check coinbase timestamp
  if(GetBlockTime() > (int64)vtx[0].nTime + nMaxClockDrift)
    return DoS(50, error("CheckBlock() : coinbase timestamp is too early"));

  // Check coinstake timestamp
  if(IsProofOfStake() && !CheckCoinStakeTimestamp(GetBlockTime(), (int64)vtx[1].nTime))
    return DoS(50, error("CheckBlock() : coinstake timestamp violation nTimeBlock=%u nTimeTx=%u",
                         GetBlockTime(), vtx[1].nTime));

  // Check coinbase reward    
  if(IsProofOfWork())
  {
    int64 blockReward = GetProofOfWorkReward(nBits);
    if(vtx[0].GetValueOut() > blockReward - vtx[0].GetMinFee() + MIN_TX_FEE)
      return DoS(50, error("CheckBlock() : coinbase reward exceeded %s > %s", 
                           FormatMoney(vtx[0].GetValueOut()).c_str(),
                           FormatMoney(blockReward).c_str()));      
  }else if(IsProofOfBurn())
  {
    int64 blockReward = GetProofOfBurnReward(nBurnBits);
    if(vtx[0].GetValueOut() > blockReward - vtx[0].GetMinFee() + MIN_TX_FEE)
      return DoS(50, error("CheckBlock() : coinbase reward exceeded %s > %s", 
                           FormatMoney(vtx[0].GetValueOut()).c_str(),
                           FormatMoney(blockReward).c_str()));      
  }else{ //proof-of-stake
    if(vtx[0].GetValueOut() > 0)
      return DoS(50, error("CheckBlock() : coinbase reward exceeded %s > %s", 
                           FormatMoney(vtx[0].GetValueOut()).c_str(),
                           FormatMoney(0).c_str()));  
  }

  // Check transactions
  BOOST_FOREACH(const CTransaction& tx, vtx)
  {
    if(!tx.CheckTransaction())
      return DoS(tx.nDoS, error("CheckBlock() : CheckTransaction failed"));
    // slimcoin: check transaction timestamp
    if(GetBlockTime() < (int64)tx.nTime)
      return DoS(50, error("CheckBlock() : block timestamp earlier than transaction timestamp"));
  }

  // Check for duplicate txids. This is caught by ConnectInputs(),
  // but catching it earlier avoids a potential DoS attack:
  set<uint256> uniqueTx;
  BOOST_FOREACH(const CTransaction& tx, vtx)
  {
    uniqueTx.insert(tx.GetHash());
  }

  if(uniqueTx.size() != vtx.size())
    return DoS(100, error("CheckBlock() : duplicate transaction"));

  unsigned int nSigOps = 0;
  BOOST_FOREACH(const CTransaction& tx, vtx)
  {
    nSigOps += tx.GetLegacySigOpCount();
  }

  if(nSigOps > MAX_BLOCK_SIGOPS)
    return DoS(100, error("CheckBlock() : out-of-bounds SigOpCount"));

  // Check merkleroot
  if(hashMerkleRoot != BuildMerkleTree())
    return DoS(100, error("CheckBlock() : hashMerkleRoot mismatch"));

  // slimcoin: check block signature
  if(!CheckBlockSignature())
    return DoS(100, error("CheckBlock() : bad block signature"));

  return true;
}

bool CBlock::AcceptBlock()
{
  // Check for duplicate
  uint256 hash = GetHash();
  if(mapBlockIndex.count(hash))
    return error("AcceptBlock() : block already in mapBlockIndex");

  // Get prev block index
  map<uint256, CBlockIndex*>::iterator mi = mapBlockIndex.find(hashPrevBlock);
  if(mi == mapBlockIndex.end())
    return DoS(10, error("AcceptBlock() : prev block not found"));

  // The effective burn coins have to match, regardless of what block type it is
  int64 calcEffCoins = 0;
  if(!CheckBurnEffectiveCoins(&calcEffCoins))
    return DoS(50, error("AcceptBlock() : Effective burn coins calculation failed: blk %"PRI64d" != calc %"PRI64d,
                         nEffectiveBurnCoins, calcEffCoins));

  CBlockIndex* pindexPrev = (*mi).second;
  int nHeight = pindexPrev->nHeight + 1;

  // Check proof-of-work or proof-of-stake bits
  if(nBits != GetNextTargetRequired(pindexPrev, IsProofOfStake()))
    return DoS(100, error("AcceptBlock() : incorrect proof-of-work/proof-of-stake nBits"));

  // Check proof-of-burn bits
  if(nBurnBits != GetNextBurnTargetRequired(pindexPrev))
    return DoS(100, error("AcceptBlock() : incorrect proof-of-burn nBurnBits"));

  // Check timestamp against prev
  if(GetBlockTime() <= pindexPrev->GetMedianTimePast() || 
     GetBlockTime() + nMaxClockDrift < pindexPrev->GetBlockTime())
    return error("AcceptBlock() : block's timestamp is too early");

  // Check that all transactions are finalized
  BOOST_FOREACH(const CTransaction& tx, vtx)
    if(!tx.IsFinal(nHeight, GetBlockTime()))
      return DoS(10, error("AcceptBlock() : contains a non-final transaction"));

  // Check that the block chain matches the known block chain up to a hardened checkpoint
  if(!Checkpoints::CheckHardened(nHeight, hash))
    return DoS(100, error("AcceptBlock() : rejected by hardened checkpoint lockin at %d", nHeight));

  // slimcoin: check that the block satisfies synchronized checkpoint
  if(!Checkpoints::CheckSync(hash, pindexPrev))
    return error("AcceptBlock() : rejected by synchronized checkpoint");

  // Write block to history file
  if(!CheckDiskSpace(::GetSerializeSize(*this, SER_DISK, CLIENT_VERSION)))
    return error("AcceptBlock() : out of disk space");
  unsigned int nFile = -1;
  unsigned int nBlockPos = 0;
  if(!WriteToDisk(nFile, nBlockPos))
    return error("AcceptBlock() : WriteToDisk failed");

  if(!AddToBlockIndex(nFile, nBlockPos))
    return error("AcceptBlock() : AddToBlockIndex failed");

  // Relay inventory, but don't relay old inventory during initial block download
  int nBlockEstimate = Checkpoints::GetTotalBlocksEstimate();
  if(hashBestChain == hash)
  {
    LOCK(cs_vNodes);
    BOOST_FOREACH(CNode* pnode, vNodes)
      if(nBestHeight > (pnode->nStartingHeight != -1 ? pnode->nStartingHeight - 2000 : nBlockEstimate))
        pnode->PushInventory(CInv(MSG_BLOCK, hash));
  }

  // slimcoin: check pending sync-checkpoint
  Checkpoints::AcceptPendingSyncCheckpoint();

  return true;
}

CBigNum CBlockIndex::GetBlockTrust() const
{

  /* New protocol */
  if(fTestNet || GetBlockTime() > CHAINCHECKS_SWITCH_TIME)
  {

    CBigNum bnTarget;
    bnTarget.SetCompact(IsProofOfBurn() ? nBurnBits : nBits);

    if(bnTarget <= 0)
      return 0;

    // Calculate work amount for block
    uint256 nBlkBase = IsProofOfBurn() ? nPoBBase : nPoWBase;
    CBigNum nBlkTrust = CBigNum(nBlkBase) / (bnTarget + 1);

    // Set nPowTrust to 1 if we are checking PoS block or PoW difficulty is too low
    nBlkTrust = (IsProofOfStake() || nBlkTrust < 1) ? 1 : nBlkTrust;

    // Return nBlkTrust for the first 12 blocks
    if(pprev == NULL || pprev->nHeight < 12)
      return nBlkTrust;

    const CBlockIndex* currentIndex = pprev;

    if(IsProofOfStake())
    {
      CBigNum bnNewTrust = (CBigNum(1)<<256) / (bnTarget + 1);

      // Return 1/3 of score if parent block is not the PoW block
      if(!pprev->IsProofOfWork())
        return bnNewTrust / 3;

      int nPoWCount = 0;

      // Check last 12 blocks type
      while (pprev->nHeight - currentIndex->nHeight < 12)
      {
        if(currentIndex->IsProofOfWork())
          nPoWCount++;
        currentIndex = currentIndex->pprev;
      }

      // Return 1/3 of score if less than 3 PoW blocks found
      if (nPoWCount < 3)
        return bnNewTrust / 3;

      return bnNewTrust;
    }else{
      CBigNum bnLastBlockTrust = CBigNum(pprev->bnChainTrust - pprev->pprev->bnChainTrust);

      // Return nBlkTrust + 2/3 of previous block score if two parent blocks are not PoS blocks
      if (!(pprev->IsProofOfStake() && pprev->pprev->IsProofOfStake()))
        return nBlkTrust + (2 * bnLastBlockTrust / 3);

      int nPoSCount = 0;

      // Check last 12 blocks type
      while(pprev->nHeight - currentIndex->nHeight < 12)
      {
        if(currentIndex->IsProofOfStake())
          nPoSCount++;
        currentIndex = currentIndex->pprev;
      }

      // Return nBlkTrust + 2/3 of previous block score if less than 7 PoS blocks found
      if(nPoSCount < 7)
        return nBlkTrust + (2 * bnLastBlockTrust / 3);

      bnTarget.SetCompact(IsProofOfBurn() ? pprev->nBurnBits : pprev->nBits);

      if(bnTarget <= 0)
        return 0;

      CBigNum bnNewTrust = (CBigNum(1) << 256) / (bnTarget + 1);

      // Return nBlkTrust + full trust score for previous block nBits or nBurnBits
      return nBlkTrust + bnNewTrust;
    }

  }else{

    /* Old protocol */

    CBigNum bnTarget;
    bnTarget.SetCompact(nBits);

    if(bnTarget <= 0)
      return 0;

    return (IsProofOfStake() ? (CBigNum(1) << 256) / (bnTarget + 1) : 1);
  }
}

bool ProcessBlock(CNode *pfrom, CBlock *pblock)
{
  // Check for duplicate
  uint256 hash = pblock->GetHash();

  if(mapBlockIndex.count(hash))
    return error("ProcessBlock() : already have block %d %s", 
                 mapBlockIndex[hash]->nHeight, hash.ToString().substr(0, 20).c_str());

  if(mapOrphanBlocks.count(hash))
    return error("ProcessBlock() : already have block (orphan) %s", hash.ToString().substr(0, 20).c_str());

  // slimcoin: check proof-of-stake
  // Limited duplicity on stake: prevents block flood attack
  // Duplicate stake allowed only when there is an orphan child block
  if(pblock->IsProofOfStake() && setStakeSeen.count(pblock->GetProofOfStake()) && 
     !mapOrphanBlocksByPrev.count(hash) && !Checkpoints::WantedByPendingSyncCheckpoint(hash))
    return error("ProcessBlock() : duplicate proof-of-stake (%s, %d) for block %s", 
                 pblock->GetProofOfStake().first.ToString().c_str(), pblock->GetProofOfStake().second, 
                 hash.ToString().c_str());

  // slimcoin: check proof-of-burn
  // Limited duplicity of burn blocks: prevents block flood attack
  // Duplicate burn block allowed only when there is an orphan child block
  if(pblock->IsProofOfBurn() && setBurnSeen.count(pblock->GetProofOfBurn()) && 
     !mapOrphanBlocksByPrev.count(hash) && !Checkpoints::WantedByPendingSyncCheckpoint(hash))
    return error("ProcessBlock() : duplicate proof-of-burn\n\t "
                 "(Burn Hash: %s\n\t "
                 "(Hash Prev: %s)\n\t "
                 "for block %s", 
                 pblock->GetProofOfBurn().first.ToString().c_str(), 
                 pblock->GetProofOfBurn().second.ToString().c_str(), 
                 hash.ToString().c_str());

  // Preliminary checks
  if(!pblock->CheckBlock())
    return error("ProcessBlock() : CheckBlock FAILED");

  // slimcoin: verify hash target and signature of coinstake tx
  if(pblock->IsProofOfStake())
  {
    uint256 hashProofOfStake = 0;
    if(!CheckProofOfStake(pblock->vtx[1], pblock->nBits, hashProofOfStake))
    {
      printf("WARNING: ProcessBlock() : check proof-of-stake failed for block %s\n", hash.ToString().c_str());
      return false; // do not error here as we expect this during initial block download
    }

    if(!mapProofOfStake.count(hash)) // add to mapProofOfStake
      mapProofOfStake.insert(make_pair(hash, hashProofOfStake));
  }

  // Verify the burn hash, matching signatures between block and burn tx, and effective coins
  if(pblock->IsProofOfBurn())
  {
    if(!pblock->CheckProofOfBurn())
    {
      //do not error here as we expect this during initial block download
      // because the previous blocks may not have been received yet
      printf("WARNING: ProcessBlock() : check proof-of-burn failed for block %s\n", hash.ToString().c_str());

      //if another block requires this one, don't error out, this PoB block will be accepted as an orphan
      // only return false when this PoB block is lone
      if(!mapOrphanBlocksByPrev.count(hash))
        return false;
    }
  }

  CBlockIndex *pcheckpoint = Checkpoints::GetLastSyncCheckpoint();
  if(pcheckpoint && pblock->hashPrevBlock != hashBestChain && !Checkpoints::WantedByPendingSyncCheckpoint(hash))
  {
    // Extra checks to prevent "fill up memory by spamming with bogus blocks"
    int64 deltaTime = pblock->GetBlockTime() - pcheckpoint->nTime;
    CBigNum bnNewBlock;
    bnNewBlock.SetCompact(pblock->nBits);
    CBigNum bnRequired;

    if(pblock->IsProofOfStake())
      bnRequired.SetCompact(
        ComputeMinStake(GetLastBlockIndex(pcheckpoint, true)->nBits, deltaTime, pblock->nTime));
    else
      bnRequired.SetCompact(ComputeMinWork(GetLastBlockIndex(pcheckpoint, false)->nBits, deltaTime));

    if(bnNewBlock > bnRequired)
    {
      if(pfrom)
        pfrom->Misbehaving(100);
      
      string block_type;
      if(pblock->IsProofOfBurn())
        block_type = "proof-of-burn";
      else if(pblock->IsProofOfStake())
        block_type = "proof-of-stake";
      else //proof of work block
        block_type = "proof-of-work";

      return error("ProcessBlock() : block with too little %s", block_type.c_str());
    }
  }

  // slimcoin: ask for pending sync-checkpoint if any
  if(!IsInitialBlockDownload())
    Checkpoints::AskForPendingSyncCheckpoint(pfrom);

  //If we do not already have its previous block, shunt it off to holding area until we get it
  if(!mapBlockIndex.count(pblock->hashPrevBlock))
  {
    printf("ProcessBlock: ORPHAN BLOCK, prev=%s\n", pblock->hashPrevBlock.ToString().substr(0,20).c_str());
    CBlock *pblock2 = new CBlock(*pblock);

    // slimcoin: check proof-of-stake
    if(pblock2->IsProofOfStake())
    {
      // Limited duplicity on stake: prevents block flood attack
      // Duplicate stake allowed only when there is orphan child block
      if(setStakeSeenOrphan.count(pblock2->GetProofOfStake()) && !mapOrphanBlocksByPrev.count(hash) && 
         !Checkpoints::WantedByPendingSyncCheckpoint(hash))
      {
        error("ProcessBlock() : duplicate proof-of-stake (%s, %d) for orphan block %s", 
              pblock2->GetProofOfStake().first.ToString().c_str(), 
              pblock2->GetProofOfStake().second, hash.ToString().c_str());
        delete pblock2;
        return false;
      }else
        setStakeSeenOrphan.insert(pblock2->GetProofOfStake());
    }else if(pblock2->IsProofOfBurn())
    {
      if(setBurnSeenOrphan.count(hash) && !mapOrphanBlocksByPrev.count(hash) &&
         !Checkpoints::WantedByPendingSyncCheckpoint(hash))
      {
        error("ProcessBlock() : duplicate proof-of-burn (%s, %s) for orphan block %s",
              pblock2->GetProofOfBurn().first.ToString().c_str(),
              pblock2->GetProofOfBurn().second.ToString().c_str(),
              hash.ToString().c_str());
        delete pblock2;
        return false;
      }else
        setBurnSeenOrphan.insert(hash);
    }
    
    mapOrphanBlocks.insert(make_pair(hash, pblock2));
    mapOrphanBlocksByPrev.insert(make_pair(pblock2->hashPrevBlock, pblock2));

    // Ask this guy to fill in what we're missing
    if(pfrom)
    {

      pfrom->PushGetBlocks(pindexBest, GetOrphanRoot(pblock2));

      // slimcoin: getblocks may not obtain the ancestor block rejected
      // earlier by duplicate-stake check so we ask for it again directly
      if(!IsInitialBlockDownload())
        pfrom->AskFor(CInv(MSG_BLOCK, WantedByOrphan(pblock2)));
    }
    return true;
  }

  // Store to disk
  if(!pblock->AcceptBlock())
    return error("ProcessBlock() : AcceptBlock FAILED");

  // Recursively process any orphan blocks that depended on this one
  vector<uint256> vWorkQueue;
  vWorkQueue.push_back(hash);
  for(unsigned int i = 0; i < vWorkQueue.size(); i++)
  {
    uint256 hashPrev = vWorkQueue[i];

    for(multimap<uint256, CBlock*>::iterator mi = mapOrphanBlocksByPrev.lower_bound(hashPrev);
         mi != mapOrphanBlocksByPrev.upper_bound(hashPrev); ++mi)
    {
      CBlock *pblockOrphan = (*mi).second;
      uint256 pblockOrphanHash = pblockOrphan->GetHash();

      if(pblockOrphan->AcceptBlock())
        vWorkQueue.push_back(pblockOrphanHash);

      mapOrphanBlocks.erase(pblockOrphanHash);
      setStakeSeenOrphan.erase(pblockOrphan->GetProofOfStake());
      setBurnSeenOrphan.erase(pblockOrphanHash);
      delete pblockOrphan;
    }

    mapOrphanBlocksByPrev.erase(hashPrev);
  }

  printf("ProcessBlock: ACCEPTED\n");

  // slimcoin: if responsible for sync-checkpoint send it
  if(pfrom && !CSyncCheckpoint::strMasterPrivKey.empty())
    Checkpoints::SendSyncCheckpoint(Checkpoints::AutoSelectSyncCheckpoint());

  return true;
}

// slimcoin: sign block
bool CBlock::SignBlock(const CKeyStore &keystore)
{
  vector<valtype> vSolutions;
  txnouttype whichType;
  const CTxOut &txout = IsProofOfStake() ? vtx[1].vout[1] : vtx[0].vout[0];

  if(!Solver(txout.scriptPubKey, whichType, vSolutions))
    return false;
  if(whichType == TX_PUBKEY)
  {
    // Sign
    const valtype& vchPubKey = vSolutions[0];
    CKey key;
    if(!keystore.GetKey(Hash160(vchPubKey), key))
      return false;
    if(key.GetPubKey() != vchPubKey)
      return false;
    return key.Sign(GetHash(), vchBlockSig);
  }
  return false;
}

// slimcoin: check block signature
bool CBlock::CheckBlockSignature() const
{
  //if it is the genesis block, first checks if the prev block's hash is 0 since
  // it should only be that for the genesis block, then check the actual hash,
  // that is done because checking the actual hash is very intensive
  if(hashPrevBlock == 0 && GetHash() == hashGenesisBlock)
    return vchBlockSig.empty();

  vector<valtype> vSolutions;
  txnouttype whichType;
  const CTxOut& txout = IsProofOfStake()? vtx[1].vout[1] : vtx[0].vout[0];

  if(!Solver(txout.scriptPubKey, whichType, vSolutions))
    return false;
  if(whichType == TX_PUBKEY)
  {
    const valtype& vchPubKey = vSolutions[0];
    CKey key;
    if(!key.SetPubKey(vchPubKey))
      return false;
    if(vchBlockSig.empty())
      return false;
    return key.Verify(GetHash(), vchBlockSig);
  }
  return false;
}

bool CBlock::CheckBurnEffectiveCoins(int64 *calcEffCoinsRet) const
{
  //Genesis Block
  if(!hashPrevBlock)
    return true;

  CBlockIndex *pindexPrev;

  std::map<uint256, CBlockIndex*>::iterator mi = mapBlockIndex.find(hashPrevBlock);
  if(mi != mapBlockIndex.end())
    pindexPrev = mapBlockIndex[hashPrevBlock];
  else
    return error("CheckBurnEffectiveCoins() : Prev block hash %s not in mapBlockIndex", 
                 hashPrevBlock.ToString().c_str());

  //get the number of burn coins in this block
  int64 nBurnedCoins = 0;
  BOOST_FOREACH(const CTransaction &tx, vtx)
  {
    s32int burnOutTxIndex = tx.GetBurnOutTxIndex();
    if(burnOutTxIndex != -1) //this is a burn transaction
      nBurnedCoins += tx.vout[burnOutTxIndex].nValue;
  }

  int64 calcEffCoins;

  //only apply the decay when the current block is a PoW block
  if(IsProofOfWork())
    calcEffCoins = (int64)((pindexPrev->nEffectiveBurnCoins / BURN_DECAY_RATE) + nBurnedCoins);
  else
    calcEffCoins = pindexPrev->nEffectiveBurnCoins + nBurnedCoins;

  if(calcEffCoinsRet)
    *calcEffCoinsRet = calcEffCoins;

  //the effective coins should equal each other
  return nEffectiveBurnCoins == calcEffCoins;
}




bool CheckDiskSpace(uint64 nAdditionalBytes)
{
  uint64 nFreeBytesAvailable = filesystem::space(GetDataDir()).available;

  // Check for 15MB because database could create another 10MB log file at any time
  if(nFreeBytesAvailable < (uint64)15000000 + nAdditionalBytes)
  {
    fShutdown = true;
    string strMessage = _("Warning: Disk space is low");
    strMiscWarning = strMessage;
    printf("*** %s\n", strMessage.c_str());
    ThreadSafeMessageBox(strMessage, "Slimcoin", wxOK | wxICON_EXCLAMATION | wxMODAL);
    StartShutdown();
    return false;
  }
  return true;
}

FILE* OpenBlockFile(unsigned int nFile, unsigned int nBlockPos, const char* pszMode)
{
  if(nFile == -1)
    return NULL;
  FILE* file = fopen((GetDataDir() / strprintf("blk%04d.dat", nFile)).string().c_str(), pszMode);
  if(!file)
    return NULL;
  if(nBlockPos != 0 && !strchr(pszMode, 'a') && !strchr(pszMode, 'w'))
  {
    if(fseek(file, nBlockPos, SEEK_SET) != 0)
    {
      fclose(file);
      return NULL;
    }
  }
  return file;
}

static unsigned int nCurrentBlockFile = 1;

FILE* AppendBlockFile(unsigned int& nFileRet)
{
  nFileRet = 0;
  for(;;)
  {
    FILE* file = OpenBlockFile(nCurrentBlockFile, 0, "ab");
    if(!file)
      return NULL;

    if(fseek(file, 0, SEEK_END))
      return NULL;

    // FAT32 filesize max 4GB, fseek and ftell max 2GB, so we must stay under 2GB
    if(ftell(file) < 0x7F000000 - MAX_SIZE)
    {
      nFileRet = nCurrentBlockFile;
      return file;
    }

    fclose(file);
    nCurrentBlockFile++;
  }
}

bool LoadBlockIndex(bool fAllowNew)
{
  if(fTestNet)
  {
    hashGenesisBlock = hashGenesisBlockTestNet;
    bnProofOfWorkLimit = CBigNum(~uint256(0) >> 16);  //4 preceding 0s, 16/4 since every hex = 4 bits
    bnInitialHashTarget = CBigNum(~uint256(0) >> 17); //0x00007ffff.....
    bnProofOfBurnLimit = CBigNum(~uint256(0) >> 16);  //4 preceding 0s, 16/4 since every hex = 4 bits
    bnProofOfStakeLimit = CBigNum(~uint256(0) >> 16); //4 preceding 0s, 16/4 since every hex = 4 bits
    nPoWBase = (~uint256(0) >> 20);                   //5 preceding 0s, 20/4 since every hex = 4 bits
    nPoBBase = (~uint256(0) >> 20);                   //5 preceding 0s, 20/4 since every hex = 4 bits

    nStakeMinAge = 60 * 60 * 24; // test net min age is 1 day
    nCoinbaseMaturity = 60;
    nModifierInterval = 60 * 20; // test net modifier interval is 20 minutes
  }

  printf("%s Network: \n\tgenesis=0x%s \n\tnBitsLimit=0x%08x \n\tnBitsInitial=0x%08x\n", 
         fTestNet ? "Test" : "Slimcoin", hashGenesisBlock.ToString().substr(0, 20).c_str(),
         bnProofOfWorkLimit.GetCompact(), bnInitialHashTarget.GetCompact());

  printf("\tnStakeMinAge=%d \n\tnCoinbaseMaturity=%d \n\tnModifierInterval=%d\n\n",
         nStakeMinAge, nCoinbaseMaturity, nModifierInterval);

  //
  // Load block index
  //
  CTxDB txdb("cr");
  if(!txdb.LoadBlockIndex())
    return false;

  txdb.Close();

  //
  // Init with genesis block
  //
  if(mapBlockIndex.empty())
  {
    if(!fAllowNew)
      return false;

    // Genesis Block:
    // CBlock(hash=000000000019d6, ver=1, hashPrevBlock=00000000000000, hashMerkleRoot=4a5e1e, nTime=1231006505, nBits=1d00ffff, nNonce=2083236893, vtx=1)
    //   CTransaction(hash=4a5e1e, ver=1, vin.size=1, vout.size=1, nLockTime=0)
    //     CTxIn(COutPoint(000000, -1), coinbase 04ffff001d0104455468652054696d65732030332f4a616e2f32303039204368616e63656c6c6f72206f6e206272696e6b206f66207365636f6e64206261696c6f757420666f722062616e6b73)
    //     CTxOut(nValue=50.00000000, scriptPubKey=0x5F1DF16B2B704C8A578D0B)
    //   vMerkleTree: 4a5e1e

    // Genesis block
    const char *pszTimestamp = "RT: 2 southeast Ukranian regions to hold referendum May 11 as planned";
    CTransaction txNew;
    txNew.nTime = !fTestNet ? 1399578460 : 1390500425;
    txNew.vin.resize(1);
    txNew.vout.resize(1);
    txNew.vin[0].scriptSig = CScript() << 486604799 << CBigNum(9999) << vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
    txNew.vout[0].SetEmpty();

    CBlock block;
    block.vtx.push_back(txNew);
    block.hashPrevBlock = 0;
    block.hashMerkleRoot = block.BuildMerkleTree();
    block.nVersion = 1;
    block.nTime    = !fTestNet ? 1399578460 : 1390500425;
    block.nBits    = bnProofOfWorkLimit.GetCompact();
    block.nNonce   = !fTestNet ? 116872 : 63626;

    // debug print
    printf("block.GetHash() = %s\n", block.GetHash().ToString().c_str());
    printf("hashGenesisBlock = %s\n", hashGenesisBlock.ToString().c_str());
    printf("block.hashMerkleRoot = %s\n", block.hashMerkleRoot.ToString().c_str());

    if(fTestNet)
      assert(block.hashMerkleRoot == uint256("0xce86aa96a71e5c74ea535ed5f23d5b1b6ca279ad16cac3cb95e123d80027f014"));
    else
      assert(block.hashMerkleRoot == uint256("0xbae3867d5e5d35c321adaf9610b9e4147a855f9ad319fdcf70913083d783753f"));

    block.print();

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
/**************************Scan for the Genesis Block**************************/
////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////// 

    //There is a byte reveral thing in the miner for the
    // hash and also the block's nonce, do not think that is needed but check

    // If genesis block hash does not match, then generate new genesis hash
    if(false && block.GetHash() != hashGenesisBlock)
    {
      printf("\nScanning for the Genesis Block\n");

      //I also included dcrypt.h up above, be sure of that
      u32int hashes_done = 0;
      uint256 phash, hashTarget = CBigNum().SetCompact(block.nBits).getuint256();

      for(block.nNonce = 0;; block.nNonce++)
      {
        //if scan does not return -1, then check if the hash is less than the target
        if(ScanDcryptHash(&block, &hashes_done, &phash) != (u32int)-1)
        {
          if(phash < hashTarget)
            break;
        }

        //if we have exhausted all nonces, increment the time and reset the nNonce
        if(block.nNonce > 0xffff0000)
        {
          printf("NONCE WRAPPED, incrementing time\n");
          block.nTime++;
          block.nNonce = 0;
        }

        printf("Looped! nNonce is %#x:%d\n", block.nNonce, block.nNonce);
      }

      printf("\n\nHash Target %s\n", hashTarget.ToString().c_str());
      printf("NEW block.nTime = %d\n", block.nTime);
      printf("NEW block.nNonce = %d\n", block.nNonce);
      printf("NEW block.GetHash = %s\n", block.GetHash().ToString().c_str());

    }

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
/**************************Scan for the Genesis Block**************************/
////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////// 

    assert(block.GetHash() == hashGenesisBlock);
    assert(block.CheckBlock());

    // Start new block file
    unsigned int nFile;
    unsigned int nBlockPos;
    if(!block.WriteToDisk(nFile, nBlockPos))
      return error("LoadBlockIndex() : writing genesis block to disk failed");
    if(!block.AddToBlockIndex(nFile, nBlockPos))
      return error("LoadBlockIndex() : genesis block not accepted");

    // slimcoin: initialize synchronized checkpoint
    if(!Checkpoints::WriteSyncCheckpoint(hashGenesisBlock))
      return error("LoadBlockIndex() : failed to init sync checkpoint");
  }

  // slimcoin: if checkpoint master key changed must reset sync-checkpoint
  {
    CTxDB txdb;
    string strPubKey = "";
    if(!txdb.ReadCheckpointPubKey(strPubKey) || strPubKey != CSyncCheckpoint::strMasterPubKey)
    {
      // write checkpoint master key to db
      txdb.TxnBegin();
      if(!txdb.WriteCheckpointPubKey(CSyncCheckpoint::strMasterPubKey))
        return error("LoadBlockIndex() : failed to write new checkpoint master key to db");
      if(!txdb.TxnCommit())
        return error("LoadBlockIndex() : failed to commit new checkpoint master key to db");
      if((!fTestNet) && !Checkpoints::ResetSyncCheckpoint())
        return error("LoadBlockIndex() : failed to reset sync-checkpoint");
    }
    txdb.Close();
  }

  return true;
}



void PrintBlockTree()
{
  // precompute tree structure
  map<CBlockIndex*, vector<CBlockIndex*> > mapNext;
  for(map<uint256, CBlockIndex*>::iterator mi = mapBlockIndex.begin(); mi != mapBlockIndex.end(); ++mi)
  {
    CBlockIndex* pindex = (*mi).second;
    mapNext[pindex->pprev].push_back(pindex);
    // test
    //while(rand() % 3 == 0)
    //    mapNext[pindex->pprev].push_back(pindex);
  }

  vector<pair<int, CBlockIndex*> > vStack;
  vStack.push_back(make_pair(0, pindexGenesisBlock));

  int nPrevCol = 0;
  while(!vStack.empty())
  {
    int nCol = vStack.back().first;
    CBlockIndex* pindex = vStack.back().second;
    vStack.pop_back();

    // print split or gap
    if(nCol > nPrevCol)
    {
      for(int i = 0; i < nCol-1; i++)
        printf("| ");
      printf("|\\\n");
    }
    else if(nCol < nPrevCol)
    {
      for(int i = 0; i < nCol; i++)
        printf("| ");
      printf("|\n");
    }
    nPrevCol = nCol;

    // print columns
    for(int i = 0; i < nCol; i++)
      printf("| ");

    // print item
    CBlock block;
    block.ReadFromDisk(pindex);
    printf("%d (%u,%u) %s  %08lx  %s  mint %7s  tx %d",
           pindex->nHeight,
           pindex->nFile,
           pindex->nBlockPos,
           block.GetHash().ToString().c_str(),
           block.nBits,
           DateTimeStrFormat(block.GetBlockTime()).c_str(),
           FormatMoney(pindex->nMint).c_str(),
           block.vtx.size());

    PrintWallets(block);

    // put the main timechain first
    vector<CBlockIndex*>& vNext = mapNext[pindex];
    for(unsigned int i = 0; i < vNext.size(); i++)
    {
      if(vNext[i]->pnext)
      {
        swap(vNext[0], vNext[i]);
        break;
      }
    }

    // iterate children
    for(unsigned int i = 0; i < vNext.size(); i++)
      vStack.push_back(make_pair(nCol+i, vNext[i]));
  }
}










//////////////////////////////////////////////////////////////////////////////
//
// CAlert
//

map<uint256, CAlert> mapAlerts;
CCriticalSection cs_mapAlerts;

static string strMintMessage = _("Info: Minting suspended due to locked wallet."); 
static string strMintWarning;

string GetWarnings(string strFor)
{
  int nPriority = 0;
  string strStatusBar;
  string strRPC;
  if(GetBoolArg("-testsafemode"))
    strRPC = "test";

  if(!CLIENT_VERSION_IS_RELEASE)
    strStatusBar = _("This is a pre-release test build - use at your own risk - do not use for mining or merchant applications");

  // slimcoin: wallet lock warning for minting
  if(strMintWarning != "")
  {
    nPriority = 0;
    strStatusBar = strMintWarning;
  }

  // Misc warnings like out of disk space and clock is wrong
  if(strMiscWarning != "")
  {
    nPriority = 1000;
    strStatusBar = strMiscWarning;
  }

  // slimcoin: should not enter safe mode for longer invalid chain
  // slimcoin: if sync-checkpoint is too old do not enter safe mode
  if(Checkpoints::IsSyncCheckpointTooOld(60 * 60 * 24 * 10) && !fTestNet)
  {
    nPriority = 100;
    strStatusBar = "WARNING: Checkpoint is too old. Wait for block chain to download, or notify developers of the issue.";
  }

  // slimcoin: if detected invalid checkpoint enter safe mode
  if(Checkpoints::hashInvalidCheckpoint != 0)
  {
    nPriority = 3000;
    strStatusBar = strRPC = "WARNING: Invalid checkpoint found! Displayed transactions may not be correct! You may need to upgrade, or notify developers of the issue.";
  }

  // Alerts
  {
    LOCK(cs_mapAlerts);
    BOOST_FOREACH(PAIRTYPE(const uint256, CAlert)& item, mapAlerts)
    {
      const CAlert& alert = item.second;
      if(alert.AppliesToMe() && alert.nPriority > nPriority)
      {
        nPriority = alert.nPriority;
        strStatusBar = alert.strStatusBar;
        if(nPriority > 1000)
          strRPC = strStatusBar;  // slimcoin: safe mode for high alert
      }
    }
  }

  if(strFor == "statusbar")
    return strStatusBar;
  else if(strFor == "rpc")
    return strRPC;
  assert(!"GetWarnings() : invalid parameter");
  return "error";
}

bool CAlert::ProcessAlert()
{
  if(!CheckSignature())
    return false;
  if(!IsInEffect())
    return false;

  {
    LOCK(cs_mapAlerts);
    // Cancel previous alerts
    for(map<uint256, CAlert>::iterator mi = mapAlerts.begin(); mi != mapAlerts.end();)
    {
      const CAlert& alert = (*mi).second;
      if(Cancels(alert))
      {
        printf("cancelling alert %d\n", alert.nID);
        mapAlerts.erase(mi++);
      }
      else if(!alert.IsInEffect())
      {
        printf("expiring alert %d\n", alert.nID);
        mapAlerts.erase(mi++);
      }
      else
        mi++;
    }

    // Check if this alert has been cancelled
    BOOST_FOREACH(PAIRTYPE(const uint256, CAlert)& item, mapAlerts)
    {
      const CAlert& alert = item.second;
      if(alert.Cancels(*this))
      {
        printf("alert already cancelled by %d\n", alert.nID);
        return false;
      }
    }

    // Add to mapAlerts
    mapAlerts.insert(make_pair(GetHash(), *this));
  }

  printf("accepted alert %d, AppliesToMe()=%d\n", nID, AppliesToMe());
  MainFrameRepaint();
  return true;
}








//////////////////////////////////////////////////////////////////////////////
//
// Messages
//


bool static AlreadyHave(CTxDB& txdb, const CInv& inv)
{
  switch (inv.type)
  {
  case MSG_TX:
  {
    bool txInMap = false;
    {
      LOCK(mempool.cs);
      txInMap = (mempool.exists(inv.hash));
    }
    return txInMap ||
      mapOrphanTransactions.count(inv.hash) ||
      txdb.ContainsTx(inv.hash);
  }

  case MSG_BLOCK:
    return mapBlockIndex.count(inv.hash) ||
      mapOrphanBlocks.count(inv.hash);
  }
  // Don't know what it is, just say we already got one
  return true;
}




bool static ProcessMessage(CNode* pfrom, string strCommand, CDataStream& vRecv)
{
  static map<CService, vector<unsigned char> > mapReuseKey;
  RandAddSeedPerfmon();
  if(fDebug) {
    printf("%s ", DateTimeStrFormat(GetTime()).c_str());
    printf("received: %s (%d bytes)\n", strCommand.c_str(), vRecv.size());
  }
  if(mapArgs.count("-dropmessagestest") && GetRand(atoi(mapArgs["-dropmessagestest"])) == 0)
  {
    printf("dropmessagestest DROPPING RECV MESSAGE\n");
    return true;
  }





  if(strCommand == "version")
  {
    // Each connection can only send one version message
    if(pfrom->nVersion != 0)
    {
      pfrom->Misbehaving(1);
      return false;
    }

    int64 nTime;
    CAddress addrMe;
    CAddress addrFrom;
    uint64 nNonce = 1;
    vRecv >> pfrom->nVersion >> pfrom->nServices >> nTime >> addrMe;
    if(pfrom->nVersion < MIN_PROTO_VERSION)
    {
      // Since February 20, 2012, the protocol is initiated at version 209,
      // and earlier versions are no longer supported
      printf("partner %s using obsolete version %i; disconnecting\n", pfrom->addr.ToString().c_str(), pfrom->nVersion);
      pfrom->fDisconnect = true;
      return false;
    }

    if(pfrom->nVersion == 10300)
      pfrom->nVersion = 300;
    if(!vRecv.empty())
      vRecv >> addrFrom >> nNonce;
    if(!vRecv.empty())
      vRecv >> pfrom->strSubVer;
    if(!vRecv.empty())
      vRecv >> pfrom->nStartingHeight;

    // Disconnect if we connected to ourself
    if(nNonce == nLocalHostNonce && nNonce > 1)
    {
      printf("connected to self at %s, disconnecting\n", pfrom->addr.ToString().c_str());
      pfrom->fDisconnect = true;
      return true;
    }

    // slimcoin: record my external IP reported by peer
    if(addrFrom.IsRoutable() && addrMe.IsRoutable())
      addrSeenByPeer = addrMe;

    // Be shy and don't send version until we hear
    if(pfrom->fInbound)
      pfrom->PushVersion();

    pfrom->fClient = !(pfrom->nServices & NODE_NETWORK);

    AddTimeData(pfrom->addr, nTime);

    // Change version
    pfrom->PushMessage("verack");
    pfrom->vSend.SetVersion(min(pfrom->nVersion, PROTOCOL_VERSION));

    if(!pfrom->fInbound)
    {
      // Advertise our address
      if(!fNoListen && !fUseProxy && addrLocalHost.IsRoutable() &&
          !IsInitialBlockDownload())
      {
        CAddress addr(addrLocalHost);
        addr.nTime = GetAdjustedTime();
        pfrom->PushAddress(addr);
      }

      // Get recent addresses
      if(pfrom->nVersion >= CADDR_TIME_VERSION || addrman.size() < 1000)
      {
        pfrom->PushMessage("getaddr");
        pfrom->fGetAddr = true;
      }
      addrman.Good(pfrom->addr);
    } else {
      if(((CNetAddr)pfrom->addr) == (CNetAddr)addrFrom)
      {
        addrman.Add(addrFrom, addrFrom);
        addrman.Good(addrFrom);
      }
    }

    // Ask the first connected node for block updates
    static int nAskedForBlocks = 0;
    if(!pfrom->fClient &&
        (pfrom->nVersion < NOBLKS_VERSION_START ||
         pfrom->nVersion >= NOBLKS_VERSION_END) &&
        (nAskedForBlocks < 1 || vNodes.size() <= 1))
    {
      nAskedForBlocks++;
      pfrom->PushGetBlocks(pindexBest, uint256(0));
    }

    // Relay alerts
    {
      LOCK(cs_mapAlerts);
      BOOST_FOREACH(PAIRTYPE(const uint256, CAlert)& item, mapAlerts)
        item.second.RelayTo(pfrom);
    }

    // slimcoin: relay sync-checkpoint
    {
      LOCK(Checkpoints::cs_hashSyncCheckpoint);
      if(!Checkpoints::checkpointMessage.IsNull())
        Checkpoints::checkpointMessage.RelayTo(pfrom);
    }

    pfrom->fSuccessfullyConnected = true;

    printf("version message: version %d, blocks=%d\n", pfrom->nVersion, pfrom->nStartingHeight);

    cPeerBlockCounts.input(pfrom->nStartingHeight);

    // slimcoin: ask for pending sync-checkpoint if any
    if(!IsInitialBlockDownload())
      Checkpoints::AskForPendingSyncCheckpoint(pfrom);
  }


  else if(pfrom->nVersion == 0)
  {
    // Must have a version message before anything else
    pfrom->Misbehaving(1);
    return false;
  }


  else if(strCommand == "verack")
  {
    pfrom->vRecv.SetVersion(min(pfrom->nVersion, PROTOCOL_VERSION));
  }


  else if(strCommand == "addr")
  {
    vector<CAddress> vAddr;
    vRecv >> vAddr;

    // Don't want addr from older versions unless seeding
    if(pfrom->nVersion < CADDR_TIME_VERSION && addrman.size() > 1000)
      return true;
    if(vAddr.size() > 1000)
    {
      pfrom->Misbehaving(20);
      return error("message addr size() = %d", vAddr.size());
    }

    // Store the new addresses
    int64 nNow = GetAdjustedTime();
    int64 nSince = nNow - 10 * 60;
    BOOST_FOREACH(CAddress& addr, vAddr)
    {
      if(fShutdown)
        return true;
      // ignore IPv6 for now, since it isn't implemented anyway
      if(!addr.IsIPv4())
        continue;
      if(addr.nTime <= 100000000 || addr.nTime > nNow + 10 * 60)
        addr.nTime = nNow - 5 * 24 * 60 * 60;
      pfrom->AddAddressKnown(addr);
      if(addr.nTime > nSince && !pfrom->fGetAddr && vAddr.size() <= 10 && addr.IsRoutable())
      {
        // Relay to a limited number of other nodes
        {
          LOCK(cs_vNodes);
          // Use deterministic randomness to send to the same nodes for 24 hours
          // at a time so the setAddrKnowns of the chosen nodes prevent repeats
          static uint256 hashSalt;
          if(hashSalt == 0)
            hashSalt = GetRandHash();
          int64 hashAddr = addr.GetHash();
          uint256 hashRand = hashSalt ^ (hashAddr<<32) ^ ((GetTime()+hashAddr)/(24*60*60));
          hashRand = Hash(BEGIN(hashRand), END(hashRand));
          multimap<uint256, CNode*> mapMix;
          BOOST_FOREACH(CNode* pnode, vNodes)
          {
            if(pnode->nVersion < CADDR_TIME_VERSION)
              continue;
            unsigned int nPointer;
            memcpy(&nPointer, &pnode, sizeof(nPointer));
            uint256 hashKey = hashRand ^ nPointer;
            hashKey = Hash(BEGIN(hashKey), END(hashKey));
            mapMix.insert(make_pair(hashKey, pnode));
          }
          int nRelayNodes = 2;
          for(multimap<uint256, CNode*>::iterator mi = mapMix.begin(); mi != mapMix.end() && nRelayNodes-- > 0; ++mi)
            ((*mi).second)->PushAddress(addr);
        }
      }
    }
    addrman.Add(vAddr, pfrom->addr, 2 * 60 * 60);
    if(vAddr.size() < 1000)
      pfrom->fGetAddr = false;
  }


  else if(strCommand == "inv")
  {
    vector<CInv> vInv;
    vRecv >> vInv;
    if(vInv.size() > 50000)
    {
      pfrom->Misbehaving(20);
      return error("message inv size() = %d", vInv.size());
    }

    // find last block in inv vector
    unsigned int nLastBlock = (unsigned int)(-1);
    for(unsigned int nInv = 0; nInv < vInv.size(); nInv++) {
      if(vInv[vInv.size() - 1 - nInv].type == MSG_BLOCK) {
        nLastBlock = vInv.size() - 1 - nInv;
        break;
      }
    }
    CTxDB txdb("r");
    for(unsigned int nInv = 0; nInv < vInv.size(); nInv++)
    {
      const CInv &inv = vInv[nInv];

      if(fShutdown)
        return true;
      pfrom->AddInventoryKnown(inv);

      bool fAlreadyHave = AlreadyHave(txdb, inv);
      if(fDebug)
        printf("  got inventory: %s  %s\n", inv.ToString().c_str(), fAlreadyHave ? "have" : "new");

      if(!fAlreadyHave)
        pfrom->AskFor(inv);
      else if(inv.type == MSG_BLOCK && mapOrphanBlocks.count(inv.hash)) {
        pfrom->PushGetBlocks(pindexBest, GetOrphanRoot(mapOrphanBlocks[inv.hash]));
      } else if(nInv == nLastBlock) {
        // In case we are on a very long side-chain, it is possible that we already have
        // the last block in an inv bundle sent in response to getblocks. Try to detect
        // this situation and push another getblocks to continue.
        std::vector<CInv> vGetData(1,inv);
        pfrom->PushGetBlocks(mapBlockIndex[inv.hash], uint256(0));
        if(fDebug)
          printf("force request: %s\n", inv.ToString().c_str());
      }

      // Track requests for our stuff
      Inventory(inv.hash);
    }
  }


  else if(strCommand == "getdata")
  {
    vector<CInv> vInv;
    vRecv >> vInv;
    if(vInv.size() > 50000)
    {
      pfrom->Misbehaving(20);
      return error("message getdata size() = %d", vInv.size());
    }

    BOOST_FOREACH(const CInv &inv, vInv)
    {
      if(fShutdown)
        return true;

      printf("received getdata for: %s", inv.ToString().c_str());

      if(inv.type == MSG_BLOCK)
      {
        // Send block from disk
        map<uint256, CBlockIndex*>::iterator mi = mapBlockIndex.find(inv.hash);
        if(mi != mapBlockIndex.end())
        {
          //useful to print the nHeight
          printf("\tnHeight: %d", mi->second->nHeight);

          CBlock block;
          block.ReadFromDisk((*mi).second);
          pfrom->PushMessage("block", block);

          // Trigger them to send a getblocks request for the next batch of inventory
          if(inv.hash == pfrom->hashContinue)
          {
            // Bypass PushInventory, this must send even if redundant,
            // and we want it right after the last block so they don't
            // wait for other stuff first.
            // slimcoin: send latest proof-of-work block to allow the
            // download node to accept as orphan (proof-of-stake 
            // block might be rejected by stake connection check)
            vector<CInv> vInv;
            vInv.push_back(CInv(MSG_BLOCK, GetLastBlockIndex(pindexBest, false)->GetBlockHash()));
            pfrom->PushMessage("inv", vInv);
            pfrom->hashContinue = 0;
          }
        }
      }
      else if(inv.IsKnownType())
      {
        // Send stream from relay memory
        {
          LOCK(cs_mapRelay);
          map<CInv, CDataStream>::iterator mi = mapRelay.find(inv);
          if(mi != mapRelay.end())
            pfrom->PushMessage(inv.GetCommand(), (*mi).second);
        }
      }

      // Track requests for our stuff
      Inventory(inv.hash);

      printf("\n");
    }
  }


  else if(strCommand == "getblocks")
  {
    CBlockLocator locator;
    uint256 hashStop;
    vRecv >> locator >> hashStop;

    // Find the last block the caller has in the main chain
    CBlockIndex* pindex = locator.GetBlockIndex();

    // Send the rest of the chain
    if(pindex)
      pindex = pindex->pnext;
    int nLimit = 500 + locator.GetDistanceBack();
    printf("getblocks %d to %s limit %d\n", (pindex ? pindex->nHeight : -1), hashStop.ToString().substr(0,20).c_str(), nLimit);
    for(; pindex; pindex = pindex->pnext)
    {
      if(pindex->GetBlockHash() == hashStop)
      {
        printf("  getblocks stopping at %d %s\n", pindex->nHeight, pindex->GetBlockHash().ToString().substr(0,20).c_str());
        // slimcoin: tell downloading node about the latest block if it's
        // without risk being rejected due to stake connection check
        if(hashStop != hashBestChain && pindex->GetBlockTime() + nStakeMinAge > pindexBest->GetBlockTime())
          pfrom->PushInventory(CInv(MSG_BLOCK, hashBestChain));
        break;
      }
      pfrom->PushInventory(CInv(MSG_BLOCK, pindex->GetBlockHash()));
      if(--nLimit <= 0)
      {
        // When this block is requested, we'll send an inv that'll make them
        // getblocks the next batch of inventory.
        printf("  getblocks stopping at limit %d %s\n", pindex->nHeight, pindex->GetBlockHash().ToString().substr(0,20).c_str());
        pfrom->hashContinue = pindex->GetBlockHash();
        break;
      }
    }
  }


  else if(strCommand == "getheaders")
  {
    CBlockLocator locator;
    uint256 hashStop;
    vRecv >> locator >> hashStop;

    CBlockIndex* pindex = NULL;
    if(locator.IsNull())
    {
      // If locator is null, return the hashStop block
      map<uint256, CBlockIndex*>::iterator mi = mapBlockIndex.find(hashStop);
      if(mi == mapBlockIndex.end())
        return true;
      pindex = (*mi).second;
    }
    else
    {
      // Find the last block the caller has in the main chain
      pindex = locator.GetBlockIndex();
      if(pindex)
        pindex = pindex->pnext;
    }

    vector<CBlock> vHeaders;
    int nLimit = 2000;
    printf("getheaders %d to %s\n", (pindex ? pindex->nHeight : -1), hashStop.ToString().substr(0,20).c_str());
    for(; pindex; pindex = pindex->pnext)
    {
      vHeaders.push_back(pindex->GetBlockHeader());
      if(--nLimit <= 0 || pindex->GetBlockHash() == hashStop)
        break;
    }
    pfrom->PushMessage("headers", vHeaders);
  }


  else if(strCommand == "tx")
  {
    vector<uint256> vWorkQueue;
    vector<uint256> vEraseQueue;
    CDataStream vMsg(vRecv);
    CTxDB txdb("r");
    CTransaction tx;
    vRecv >> tx;

    CInv inv(MSG_TX, tx.GetHash());
    pfrom->AddInventoryKnown(inv);

    bool fMissingInputs = false;
    if(tx.AcceptToMemoryPool(txdb, true, &fMissingInputs))
    {
      SyncWithWallets(tx, NULL, true);

      RelayMessage(inv, vMsg);
      mapAlreadyAskedFor.erase(inv);
      vWorkQueue.push_back(inv.hash);
      vEraseQueue.push_back(inv.hash);

      // Recursively process any orphan transactions that depended on this one
      for(unsigned int i = 0; i < vWorkQueue.size(); i++)
      {
        uint256 hashPrev = vWorkQueue[i];
        for(map<uint256, CDataStream*>::iterator mi = mapOrphanTransactionsByPrev[hashPrev].begin();
             mi != mapOrphanTransactionsByPrev[hashPrev].end();
             ++mi)
        {
          const CDataStream& vMsg = *((*mi).second);
          CTransaction tx;
          CDataStream(vMsg) >> tx;
          CInv inv(MSG_TX, tx.GetHash());
          bool fMissingInputs2 = false;

          if(tx.AcceptToMemoryPool(txdb, true, &fMissingInputs2))
          {
            printf("   accepted orphan tx %s\n", inv.hash.ToString().substr(0,10).c_str());
            SyncWithWallets(tx, NULL, true);
            RelayMessage(inv, vMsg);
            mapAlreadyAskedFor.erase(inv);
            vWorkQueue.push_back(inv.hash);
            vEraseQueue.push_back(inv.hash);
          }
          else if(!fMissingInputs2)
          {
            // invalid orphan
            vEraseQueue.push_back(inv.hash);
            printf("   removed invalid orphan tx %s\n", inv.hash.ToString().substr(0,10).c_str());
          }
        }
      }

      BOOST_FOREACH(uint256 hash, vEraseQueue)
        EraseOrphanTx(hash);
    }
    else if(fMissingInputs)
    {
      AddOrphanTx(vMsg);

      // DoS prevention: do not allow mapOrphanTransactions to grow unbounded
      unsigned int nEvicted = LimitOrphanTxSize(MAX_ORPHAN_TRANSACTIONS);
      if(nEvicted > 0)
        printf("mapOrphan overflow, removed %u tx\n", nEvicted);
    }

    if(tx.nDoS)
      pfrom->Misbehaving(tx.nDoS);
  }


  else if(strCommand == "block")
  {
    CBlock block;
    vRecv >> block;

    printf("received block %s\n", block.GetHash().ToString().substr(0, 20).c_str());
    
    block.print();

    CInv inv(MSG_BLOCK, block.GetHash());
    pfrom->AddInventoryKnown(inv);

    if(ProcessBlock(pfrom, &block))
      mapAlreadyAskedFor.erase(inv);

    if(block.nDoS)
      pfrom->Misbehaving(block.nDoS);
  }


  else if(strCommand == "getaddr")
  {
    pfrom->vAddrToSend.clear();
    vector<CAddress> vAddr = addrman.GetAddr();
    BOOST_FOREACH(const CAddress &addr, vAddr)
      pfrom->PushAddress(addr);
  }


  else if(strCommand == "checkorder")
  {
    uint256 hashReply;
    vRecv >> hashReply;

    if(!GetBoolArg("-allowreceivebyip"))
    {
      pfrom->PushMessage("reply", hashReply, (int)2, string(""));
      return true;
    }

    CWalletTx order;
    vRecv >> order;

    /// we have a chance to check the order here

    // Keep giving the same key to the same ip until they use it
    if(!mapReuseKey.count(pfrom->addr))
      pwalletMain->GetKeyFromPool(mapReuseKey[pfrom->addr], true);

    // Send back approval of order and pubkey to use
    CScript scriptPubKey;
    scriptPubKey << mapReuseKey[pfrom->addr] << OP_CHECKSIG;
    pfrom->PushMessage("reply", hashReply, (int)0, scriptPubKey);
  }


  else if(strCommand == "reply")
  {
    uint256 hashReply;
    vRecv >> hashReply;

    CRequestTracker tracker;
    {
      LOCK(pfrom->cs_mapRequests);
      map<uint256, CRequestTracker>::iterator mi = pfrom->mapRequests.find(hashReply);
      if(mi != pfrom->mapRequests.end())
      {
        tracker = (*mi).second;
        pfrom->mapRequests.erase(mi);
      }
    }
    if(!tracker.IsNull())
      tracker.fn(tracker.param1, vRecv);
  }


  else if(strCommand == "ping")
  {
    if(pfrom->nVersion > BIP0031_VERSION)
    {
      uint64 nonce = 0;
      vRecv >> nonce;
      // Echo the message back with the nonce. This allows for two useful features:
      //
      // 1) A remote node can quickly check if the connection is operational
      // 2) Remote nodes can measure the latency of the network thread. If this node
      //    is overloaded it won't respond to pings quickly and the remote node can
      //    avoid sending us more work, like chain download requests.
      //
      // The nonce stops the remote getting confused between different pings: without
      // it, if the remote node sends a ping once per second and this node takes 5
      // seconds to respond to each, the 5th ping the remote sends would appear to
      // return very quickly.
      pfrom->PushMessage("pong", nonce);
    }
  }


  else if(strCommand == "alert")
  {
    CAlert alert;
    vRecv >> alert;

    if(alert.ProcessAlert())
    {
      // Relay
      pfrom->setKnown.insert(alert.GetHash());
      {
        LOCK(cs_vNodes);
        BOOST_FOREACH(CNode* pnode, vNodes)
          alert.RelayTo(pnode);
      }
    }
  }

  else if(strCommand == "checkpoint")
  {
    CSyncCheckpoint checkpoint;
    vRecv >> checkpoint;

    if(checkpoint.ProcessSyncCheckpoint(pfrom))
    {
      // Relay
      pfrom->hashCheckpointKnown = checkpoint.hashCheckpoint;
      LOCK(cs_vNodes);
      BOOST_FOREACH(CNode* pnode, vNodes)
        checkpoint.RelayTo(pnode);
    }
  }

  else
  {
    // Ignore unknown commands for extensibility
  }


  // Update the last seen time for this node's address
  if(pfrom->fNetworkNode)
    if(strCommand == "version" || strCommand == "addr" || strCommand == "inv" || strCommand == "getdata" || strCommand == "ping")
      AddressCurrentlyConnected(pfrom->addr);


  return true;
}

bool ProcessMessages(CNode* pfrom)
{
  CDataStream& vRecv = pfrom->vRecv;
  if(vRecv.empty())
    return true;
  //if(fDebug)
  //    printf("ProcessMessages(%u bytes)\n", vRecv.size());

  //
  // Message format
  //  (4) message start
  //  (12) command
  //  (4) size
  //  (4) checksum
  //  (x) data
  //

  unsigned char pchMessageStart[4];
  GetMessageStart(pchMessageStart);
  static int64 nTimeLastPrintMessageStart = 0;
  if(fDebug && GetBoolArg("-printmessagestart") && nTimeLastPrintMessageStart + 30 < GetAdjustedTime())
  {
    string strMessageStart((const char *)pchMessageStart, sizeof(pchMessageStart));
    vector<unsigned char> vchMessageStart(strMessageStart.begin(), strMessageStart.end());
    printf("ProcessMessages : AdjustedTime=%"PRI64d" MessageStart=%s\n", GetAdjustedTime(), HexStr(vchMessageStart).c_str());
    nTimeLastPrintMessageStart = GetAdjustedTime();
  }

  for(;;)
  {
    // Scan for message start
    CDataStream::iterator pstart = search(vRecv.begin(), vRecv.end(), BEGIN(pchMessageStart), END(pchMessageStart));
    int nHeaderSize = vRecv.GetSerializeSize(CMessageHeader());
    if(vRecv.end() - pstart < nHeaderSize)
    {
      if((int)vRecv.size() > nHeaderSize)
      {
        printf("\n\nPROCESSMESSAGE MESSAGESTART NOT FOUND\n\n");
        vRecv.erase(vRecv.begin(), vRecv.end() - nHeaderSize);
      }
      break;
    }
    if(pstart - vRecv.begin() > 0)
      printf("\n\nPROCESSMESSAGE SKIPPED %d BYTES\n\n", pstart - vRecv.begin());
    vRecv.erase(vRecv.begin(), pstart);

    // Read header
    vector<char> vHeaderSave(vRecv.begin(), vRecv.begin() + nHeaderSize);
    CMessageHeader hdr;
    vRecv >> hdr;
    if(!hdr.IsValid())
    {
      printf("\n\nPROCESSMESSAGE: ERRORS IN HEADER %s\n\n\n", hdr.GetCommand().c_str());
      continue;
    }
    string strCommand = hdr.GetCommand();

    // Message size
    unsigned int nMessageSize = hdr.nMessageSize;
    if(nMessageSize > MAX_SIZE)
    {
      printf("ProcessMessages(%s, %u bytes) : nMessageSize > MAX_SIZE\n", strCommand.c_str(), nMessageSize);
      continue;
    }
    if(nMessageSize > vRecv.size())
    {
      // Rewind and wait for rest of message
      vRecv.insert(vRecv.begin(), vHeaderSave.begin(), vHeaderSave.end());
      break;
    }

    // Checksum
    uint256 hash = Hash(vRecv.begin(), vRecv.begin() + nMessageSize);
    unsigned int nChecksum = 0;
    memcpy(&nChecksum, &hash, sizeof(nChecksum));
    if(nChecksum != hdr.nChecksum)
    {
      printf("ProcessMessages(%s, %u bytes) : CHECKSUM ERROR nChecksum=%08x hdr.nChecksum=%08x\n",
             strCommand.c_str(), nMessageSize, nChecksum, hdr.nChecksum);
      continue;
    }

    // Copy message to its own buffer
    CDataStream vMsg(vRecv.begin(), vRecv.begin() + nMessageSize, vRecv.nType, vRecv.nVersion);
    vRecv.ignore(nMessageSize);

    // Process message
    bool fRet = false;
    try
    {
      {
        LOCK(cs_main);
        fRet = ProcessMessage(pfrom, strCommand, vMsg);
      }
      if(fShutdown)
        return true;
    }
    catch (std::ios_base::failure& e)
    {
      if(strstr(e.what(), "end of data"))
      {
        // Allow exceptions from underlength message on vRecv
        printf("ProcessMessages(%s, %u bytes) : Exception '%s' caught, normally caused by a message being shorter than its stated length\n", strCommand.c_str(), nMessageSize, e.what());
      }
      else if(strstr(e.what(), "size too large"))
      {
        // Allow exceptions from overlong size
        printf("ProcessMessages(%s, %u bytes) : Exception '%s' caught\n", strCommand.c_str(), nMessageSize, e.what());
      }
      else
      {
        PrintExceptionContinue(&e, "ProcessMessages()");
      }
    }
    catch (std::exception& e) {
      PrintExceptionContinue(&e, "ProcessMessages()");
    } catch (...) {
      PrintExceptionContinue(NULL, "ProcessMessages()");
    }

    if(!fRet)
      printf("ProcessMessage(%s, %u bytes) FAILED\n", strCommand.c_str(), nMessageSize);
  }

  vRecv.Compact();
  return true;
}


bool SendMessages(CNode* pto, bool fSendTrickle)
{
  TRY_LOCK(cs_main, lockMain);
  if(lockMain) {
    // Don't send anything until we get their version message
    if(pto->nVersion == 0)
      return true;

    // Keep-alive ping. We send a nonce of zero because we don't use it anywhere
    // right now.
    if(pto->nLastSend && GetTime() - pto->nLastSend > 30 * 60 && pto->vSend.empty()) {
      uint64 nonce = 0;
      if(pto->nVersion > BIP0031_VERSION)
        pto->PushMessage("ping", nonce);
      else
        pto->PushMessage("ping");
    }

    // Resend wallet transactions that haven't gotten in a block yet
    ResendWalletTransactions();

    // Address refresh broadcast
    static int64 nLastRebroadcast;
    if(!IsInitialBlockDownload() && (GetTime() - nLastRebroadcast > 24 * 60 * 60))
    {
      {
        LOCK(cs_vNodes);
        BOOST_FOREACH(CNode* pnode, vNodes)
        {
          // Periodically clear setAddrKnown to allow refresh broadcasts
          if(nLastRebroadcast)
            pnode->setAddrKnown.clear();

          // Rebroadcast our address
          if(!fNoListen && !fUseProxy && addrLocalHost.IsRoutable())
          {
            CAddress addr(addrLocalHost);
            addr.nTime = GetAdjustedTime();
            pnode->PushAddress(addr);
          }
        }
      }
      nLastRebroadcast = GetTime();
    }

    //
    // Message: addr
    //
    if(fSendTrickle)
    {
      vector<CAddress> vAddr;
      vAddr.reserve(pto->vAddrToSend.size());
      BOOST_FOREACH(const CAddress& addr, pto->vAddrToSend)
      {
        // returns true if wasn't already contained in the set
        if(pto->setAddrKnown.insert(addr).second)
        {
          vAddr.push_back(addr);
          // receiver rejects addr messages larger than 1000
          if(vAddr.size() >= 1000)
          {
            pto->PushMessage("addr", vAddr);
            vAddr.clear();
          }
        }
      }
      pto->vAddrToSend.clear();
      if(!vAddr.empty())
        pto->PushMessage("addr", vAddr);
    }


    //
    // Message: inventory
    //
    vector<CInv> vInv;
    vector<CInv> vInvWait;
    {
      LOCK(pto->cs_inventory);
      vInv.reserve(pto->vInventoryToSend.size());
      vInvWait.reserve(pto->vInventoryToSend.size());
      BOOST_FOREACH(const CInv& inv, pto->vInventoryToSend)
      {
        if(pto->setInventoryKnown.count(inv))
          continue;

        // trickle out tx inv to protect privacy
        if(inv.type == MSG_TX && !fSendTrickle)
        {
          // 1/4 of tx invs blast to all immediately
          static uint256 hashSalt;
          if(hashSalt == 0)
            hashSalt = GetRandHash();
          uint256 hashRand = inv.hash ^ hashSalt;
          hashRand = Hash(BEGIN(hashRand), END(hashRand));
          bool fTrickleWait = ((hashRand & 3) != 0);

          // always trickle our own transactions
          if(!fTrickleWait)
          {
            CWalletTx wtx;
            if(GetTransaction(inv.hash, wtx))
              if(wtx.fFromMe)
                fTrickleWait = true;
          }

          if(fTrickleWait)
          {
            vInvWait.push_back(inv);
            continue;
          }
        }

        // returns true if wasn't already contained in the set
        if(pto->setInventoryKnown.insert(inv).second)
        {
          vInv.push_back(inv);
          if(vInv.size() >= 1000)
          {
            pto->PushMessage("inv", vInv);
            vInv.clear();
          }
        }
      }
      pto->vInventoryToSend = vInvWait;
    }
    if(!vInv.empty())
      pto->PushMessage("inv", vInv);


    //
    // Message: getdata
    //
    vector<CInv> vGetData;
    int64 nNow = GetTime() * 1000000;
    CTxDB txdb("r");
    while(!pto->mapAskFor.empty() && (*pto->mapAskFor.begin()).first <= nNow)
    {
      const CInv& inv = (*pto->mapAskFor.begin()).second;
      if(!AlreadyHave(txdb, inv))
      {
        printf("sending getdata: %s\n", inv.ToString().c_str());
        vGetData.push_back(inv);
        if(vGetData.size() >= 1000)
        {
          pto->PushMessage("getdata", vGetData);
          vGetData.clear();
        }
      }
      mapAlreadyAskedFor[inv] = nNow;
      pto->mapAskFor.erase(pto->mapAskFor.begin());
    }
    if(!vGetData.empty())
      pto->PushMessage("getdata", vGetData);

  }
  return true;
}














//////////////////////////////////////////////////////////////////////////////
//
// SlimCoinMiner
//

int static FormatHashBlocks(void* pbuffer, unsigned int len)
{
  unsigned char* pdata = (unsigned char*)pbuffer;
  unsigned int blocks = 1 + ((len + 8) / 64);
  unsigned char* pend = pdata + 64 * blocks;
  memset(pdata + len, 0, 64 * blocks - len);
  pdata[len] = 0x80;
  unsigned int bits = len * 8;
  pend[-1] = (bits >> 0) & 0xff;
  pend[-2] = (bits >> 8) & 0xff;
  pend[-3] = (bits >> 16) & 0xff;
  pend[-4] = (bits >> 24) & 0xff;
  return blocks;
}

static const unsigned int pSHA256InitState[8] =
{0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};

void SHA256Transform(void* pstate, void* pinput, const void* pinit)
{
  SHA256_CTX ctx;
  unsigned char data[64];

  SHA256_Init(&ctx);

  for(int i = 0; i < 16; i++)
    ((uint32_t*)data)[i] = ByteReverse(((uint32_t*)pinput)[i]);

  for(int i = 0; i < 8; i++)
    ctx.h[i] = ((uint32_t*)pinit)[i];

  SHA256_Update(&ctx, data, sizeof(data));
  for(int i = 0; i < 8; i++)
    ((uint32_t*)pstate)[i] = ctx.h[i];

  return;
}

//
// ScanDcryptHash scans nonces looking for a hash with at least some zero bits.
// It operates on big endian data.  Caller does the byte reversing.
// nNonce is usually preserved between calls, but periodically or if nNonce is 0xffff0000 or above,
// the block is rebuilt and nNonce starts over at zero.
//
static u32int ScanDcryptHash(CBlock *pblock, u32int *nHashesDone, uint256 *phash)
{
  u32int *nNonce = &(pblock->nNonce);
  u32int orig_nNonce = *nNonce;
  uint8_t digest[DCRYPT_DIGEST_LENGTH];

  for(; !fShutdown;)
  {
    //hash the block
    (*nNonce)++;

    *phash = dcrypt(UBEGIN(pblock->nVersion), HASH_PBLOCK_SIZE(pblock), digest);

    // Return the nonce if the top 8 bits of the hash are all 0s,
    // caller will check if it satisfies the target
    if(!uint256_get_top<u8int>(*phash))
    {
      //increment the hash counter accordingly
      *nHashesDone += (*nNonce - orig_nNonce);
      return *nNonce;
    }

    // If nothing found after trying for a while, return -1
    if(!(*nNonce & 0xffff))
    {
      *nHashesDone = 0xffff + 1;
      return (u32int) -1;
    }
  }

  return (u32int) -1;
}

CBlockIndex *pindexByHeight(s32int nHeight)
{
  if(nHeight < 0)
    return NULL;

  //Get the block index by burnBlkHeight
  CBlockIndex *pindex = NULL;
  const CBlockIndex *pTestBlkIndex;

  //if pindexBest is not set yet, scan through the entire map
  if(!pindexBest)
  {
    BOOST_FOREACH(const PAIRTYPE(uint256, CBlockIndex*)& item, mapBlockIndex)
    {
      pTestBlkIndex = item.second;
      if(pTestBlkIndex->nHeight == nHeight)
      {
        pindex = (CBlockIndex*)pTestBlkIndex;
        break;
      }
    }
  }else{
    for(pTestBlkIndex = pindexBest; pTestBlkIndex && pTestBlkIndex->pprev; pTestBlkIndex = pTestBlkIndex->pprev)
      if(pTestBlkIndex->nHeight == nHeight)
      {
        pindex = (CBlockIndex*)pTestBlkIndex;
        break;
      }
  }

  return pindex;
}

//given a valid block height, transaction depth, out transaction dept, this will set values for those
bool GetAllTxClassesByIndex(s32int blkHeight, s32int txDepth, s32int txOutDepth, 
                            CBlock &blockRet, CTransaction &txRet, CTxOut &txOutRet)
{
  if(blkHeight < 0 || txDepth < 0 || txOutDepth < 0)
    return false;

  //Get the block index by burnBlkHeight
  const CBlockIndex *pindex = pindexByHeight(blkHeight);

  if(!pindex || !pindex->pprev)
    return false;

  CBlock block;
  //Read the block
  if(!block.ReadFromDisk(pindex))
    return false;

  if(txDepth >= block.vtx.size())
    return false;

  CTransaction transaction = block.vtx[txDepth];

  if(txOutDepth >= transaction.vout.size())
    return false;

  //we may now set the return values if nothing failed
  blockRet = block;
  txRet = transaction;
  txOutRet = transaction.vout[txOutDepth];
  
  //sucess!
  return true;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
/*                              Proof Of Burn                               */
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//Returns the number of proof of work blocks between (not including) the
// blocks with heights startHeight and endHeight
s32int nPoWBlocksBetween(s32int startHeight, s32int endHeight)
{
  if(startHeight >= endHeight || startHeight < 0 || endHeight < 0)
    return 0;

  s32int between = 0;
  CBlockIndex *pindex = pindexByHeight(endHeight);
  
  //Go backwards and count the number of proof of work indexes
  for(; pindex && pindex->pprev && pindex->pprev->nHeight > startHeight; pindex = pindex->pprev)
  {
    if(pindex->IsProofOfWork())
      between++;
  }

  //exited the for loop early, bad thing
  if(!pindex || !pindex->pprev)
    return 0;

  return between;
}

//Calculates the has with the given input data
// burnBlockHash is the hash of the block holding the burn transaction
// hashPrevBlock is the hash of the previous block of the block the hash is being calculated on
// burnTxHash is the hash of the burn transaction
// burnBlkHeight is the nHeight of the block holding the burn transaction
// burnValue is the amount of coins burnded
// 
// smallestHashRet is the returned proof-of-burn hash 
// if fRetIntermediate is true, returns the burn hash before the multiplier is applied
bool HashBurnData(uint256 burnBlockHash, uint256 hashPrevBlock, uint256 burnTxHash,
                  s32int burnBlkHeight, int64 burnValue, uint256 &smallestHashRet, bool fRetIntermediate)
{
  //start off the smallest hash the absolute biggest it can be
  smallestHashRet = ~uint256(0);

  //the hashBlock must appear in the mapBlockIndex
  if(!mapBlockIndex.count(hashPrevBlock))
    return error("HashBurnData() : Block hash %s not found in mapBlockIndex", hashPrevBlock.ToString().c_str());

  const s32int lastBlkHeight = mapBlockIndex[hashPrevBlock]->nHeight;
  const s32int between = nPoWBlocksBetween(burnBlkHeight, lastBlkHeight);

  if(between < BURN_MIN_CONFIRMS)
    return error("HashBurnData() : Burn transaction does not meet minimum number of confirmations %d < %d", 
                 between, BURN_MIN_CONFIRMS);

  //calculate the multiplier for the hash, the pow() represents the decay
  // subtracts BURN_MIN_CONFIRMS since the first block the coins get active should have 100% power
  const double multiplier = (BURN_CONSTANT / burnValue) * 
    pow(2, (between - BURN_MIN_CONFIRMS) / BURN_HASH_DOUBLE);

  //Calculate the burn hash
  {
    //the largest value a uint256 can store
    const CBigNum bnMax(~uint256(0));
    CBigNum bnTest;

    // package the data to be hashed and hash
    CDataStream ss(SER_GETHASH, 0);
    ss << burnBlockHash << burnTxHash << hashPrevBlock;
    CBigNum bnHash(Hash(ss.begin(), ss.end()));
    
    //if the intermediate burn hash is wanted, return now
    if(fRetIntermediate)
    {
      smallestHashRet = bnHash.getuint256();
      
      //sucess!
      return true;
    }

    //apply the multiplier
    bnTest = bnHash * multiplier;

    //if bignum test is too big to fit in a uint256, continue
    if(bnTest > bnMax)
      return false;

    //assign the final bnTest hash to the smallestHashRet
    if(lastBlkHeight >= BURN_ROUND_DOWN)
      smallestHashRet = becomeCompact(bnTest.getuint256());
    else
      smallestHashRet = bnTest.getuint256();
  }

  //impossible, used as a saftey net if something went wrong
  if(!smallestHashRet)
  {
    smallestHashRet = ~uint256(0);
    return error("HashBurnData(): smallestHashRet is 0\n");
  }

  //sucess!
  return true;
}

//Gets the hash for PoB given only the indexes and a hashPrevBlock, usually the best block's hash at the time
// This function does the sanity checks, the HashBurnData() does the actual hashing
bool GetBurnHash(uint256 hashPrevBlock, s32int burnBlkHeight, s32int burnCTx,
                 s32int burnCTxOut, uint256 &smallestHashRet, bool fRetIntermediate)
{

  //make the smallest hash the absolute biggest it can be
  smallestHashRet = ~uint256(0);

  if(burnBlkHeight < 0 || burnCTx < 0 || burnCTxOut < 0)
    return error("GetBurnHash(): Input indexes are invalid %d:%d:%d\n", 
                 burnBlkHeight, burnCTx, burnCTxOut);

  CBlock block;
  CTransaction burnTx;
  CTxOut burnTxOut;

  if(!GetAllTxClassesByIndex(burnBlkHeight, burnCTx, burnCTxOut, block, burnTx, burnTxOut))
    return error("GetBurnHash(): Unable to read burn transaction %d:%d:%d\n", burnBlkHeight, burnCTx, burnCTxOut);

  const uint256 txHashBlock = block.GetHash();

  //check if burnTxOut's address is a burn address
  // with a bunch of sanity checks
  CBurnAddress burnAddress;

  //check if the out transaction went to a burn address
  CBitcoinAddress address;
  if(!ExtractAddress(burnTxOut.scriptPubKey, address))
    return error("GetBurnHash(): ExtractAddress failed");

  if(address != burnAddress)
    return error("GetBurnHash(): TxOut's address is not a valid burn address");

  if(!address.IsValid())
    return error("GetBurnHash(): TxOut's address is invalid");

  if(!burnTxOut.nValue)
    return error("GetBurnHash(): Burn transaction's value is 0");

  //passed all sanity checks, now do the actuall hashing
  return HashBurnData(txHashBlock, hashPrevBlock, burnTx.GetHash(), 
                      burnBlkHeight , burnTxOut.nValue, smallestHashRet, fRetIntermediate);
}

//Scans all of the hashes of this transaction and returns the smallest one
bool ScanBurnHashes(const CWalletTx &burnWTx, uint256 &smallestHashRet)
{
  /*slimcoin: a burn hash is calculated by:
   * hash = (c / b) * 2 ** ((nPoWBlocks - M) / E) * [Hash]
   *
   * Where: c = BURN_CONSTANT
   *        b = amount of coins burned
   *        nPoWBlocks = the number of proof of work blocks between (not including)
   *                     the blocks with heights last_BlkNHeight and burned_BlkNHeight
   *                         where
   *                             last_BlkNHeight = the height of the last block in the chain
   *                             burned_BlkNHeight = the height of the block at the time of the burning
   *        M = BURN_MIN_CONFIRMS, the required amount of proof of work blocks between (not including)
   *                                   the block at the time of burning and the last block in the chain
   *                                   The offset by M allows for the first burn block the burnt coins
   *                                   can hash to be at 100% strength and decay from there, instead of having
   *                                   the coins slightly decayed from the beginning
   *        E = BURN_HASH_DOUBLE, an exponential constant which causes 
   *                                   burnt coins to produce slightly larger hashes as time passes
   *
   *        [Hash] = Hash(burntBlockHash ++ burnWTx.GetHash() ++ hashBestBlock)
   *        Where: burntBlockHash = the hash of the block the transaction is found ing
   *               burnTx.GetHash() = the hash of this transaction
   *               hashBestBlock = the hash of the best proof-of-work block in the chain at the time of hashing
   */

  //check if the transaction is old enough
  if(!burnWTx.IsBurnTxMature())
    return false;

  //check if the wallet transaction has a block hash connected to it
  if(!burnWTx.hashBlock)
    return error("ScanBurnHashes: burnWTx.hashBlock == 0, the transaction has %d confirmations", 
                 burnWTx.GetDepthInMainChain());

  //start off the smallest hash the absolute biggest it can be
  smallestHashRet = ~uint256(0);

  //find the burnt out transaction
  CTxOut burnTxOut = burnWTx.GetBurnOutTx();

  //if burnTxOut is still null, that means it did not find a burn transaction in burnWTx
  if(burnTxOut.IsNull())
    return error("ScanBurnHashes: Did not find a burn transaction in burnWTx");

  if(!burnTxOut.nValue)
    return error("ScanBurnHashes: Burn transaction's value is 0");

  if(!mapBlockIndex.count(burnWTx.hashBlock))
    return error("ScanBurnHashes: hash %s not is mapBlockIndex", burnWTx.hashBlock.ToString().c_str());

  CBlockIndex *pindex = mapBlockIndex[burnWTx.hashBlock];

  //passed all sanity checks, now do the actual hashing
  return HashBurnData(burnWTx.hashBlock, pindexBest->GetBlockHash(), burnWTx.GetHash(), 
                      pindex->nHeight, burnTxOut.nValue, smallestHashRet, false);
}

//returns the (if found) the best hash with the transaction that produced it
void HashAllBurntTx(uint256 &smallestHashRet, CWalletTx &smallestWTxRet)
{
  //give the smallest hash the absolute largest value it can hold
  smallestHashRet = ~uint256(0);

  //if the best index is not a proof-of-work index, do not bother hashing as it will throw errors
  if(!pindexBest->IsProofOfWork())
    return;

  //go through all of the burnt hashes in the setBurnHashes
  for(set<uint256>::iterator it = pwalletMain->setBurnHashes.begin(); 
      it != pwalletMain->setBurnHashes.end(); ++it)
  {
    if(!pwalletMain->mapWallet.count(*it))
      continue;

    CWalletTx tmpWTx = pwalletMain->mapWallet[*it];
    if(!tmpWTx.IsBurnTxMature()) //transaction has to have at least some confirmations
      continue;

    uint256 tmpHash;
    if(!ScanBurnHashes(tmpWTx, tmpHash))
      continue;

    if(tmpHash < smallestHashRet)
    {
      smallestHashRet = tmpHash;
      smallestWTxRet = const_cast<const CWalletTx&>(tmpWTx);
    }

  }
  
  return;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
/*                              Proof Of Burn                               */
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////



// Some explaining would be appreciated
class COrphan
{
public:
  CTransaction* ptx;
  set<uint256> setDependsOn;
  double dPriority;

  COrphan(CTransaction* ptxIn)
  {
    ptx = ptxIn;
    dPriority = 0;
  }

  void print() const
  {
    printf("COrphan(hash=%s, dPriority=%.1f)\n", ptx->GetHash().ToString().substr(0,10).c_str(), dPriority);
    BOOST_FOREACH(uint256 hash, setDependsOn)
      printf("   setDependsOn %s\n", hash.ToString().substr(0,10).c_str());
  }
};


uint64 nLastBlockTx = 0;
uint64 nLastBlockSize = 0;
int64 nLastCoinStakeSearchInterval = 0;

// CreateNewBlock:
//   fProofOfStake: try (best effort) to make a proof-of-stake block
//   burnWalletTx is the walletTx for a burn transaction that when hashed, produces a valid hash <= burn target
CBlock *CreateNewBlock(CWallet* pwallet, bool fProofOfStake, const CWalletTx *burnWalletTx, CReserveKey *resKey)
{
  //if resKey exists, use it, else make a temporary reservekey
  CReserveKey tmpResKey(pwallet);
  CReserveKey *reservekey = resKey ? resKey : &tmpResKey;

  // Create new block
  auto_ptr<CBlock> pblock(new CBlock());
  if(!pblock.get())
    return NULL;

  //if the burnWalletTx is a non-NULL pointer, set the burn coords
  if(burnWalletTx)
  {
    if(!mapBlockIndex.count(burnWalletTx->hashBlock))
      return NULL;

    pblock->fProofOfBurn = true;
    pblock->hashBurnBlock = burnWalletTx->hashBlock;
    pblock->burnBlkHeight = mapBlockIndex[burnWalletTx->hashBlock]->nHeight;
    pblock->burnCTx = burnWalletTx->nIndex;
    pblock->burnCTxOut = burnWalletTx->GetBurnOutTxIndex();
  }

  // Create coinbase tx
  CTransaction txNew;
  txNew.vin.resize(1);
  txNew.vin[0].prevout.SetNull();
  txNew.vout.resize(1);

  //handle the public key of burn block differently
  if(pblock->IsProofOfBurn())
  {
    CBlock burnBlock;
    CTransaction burnTx;
    CTxOut burnTxOut;

    //given the burn coords in pblock, set the class objects burnBlock, burnTx, burnTxOut
    if(!GetAllTxClassesByIndex(pblock->burnBlkHeight, pblock->burnCTx, pblock->burnCTxOut, 
                               burnBlock, burnTx, burnTxOut))
      return NULL;

    CScript sendersPubKey;
    if(!burnTx.GetSendersPubKey(sendersPubKey, true))
      return NULL;

    vector<valtype> vSolutions;
    txnouttype whichType;
    if(!Solver(sendersPubKey, whichType, vSolutions))
      return NULL;

    if(whichType != TX_PUBKEY)
      return NULL;

    txNew.vout[0].scriptPubKey << vSolutions[0];
  }else
    txNew.vout[0].scriptPubKey << reservekey->GetReservedKey();

  txNew.vout[0].scriptPubKey << OP_CHECKSIG;

  // Add our coinbase tx as first transaction
  pblock->vtx.push_back(txNew);

  // slimcoin: if coinstake available add coinstake tx
  static int64 nLastCoinStakeSearchTime = GetAdjustedTime();  // only initialized at startup
  CBlockIndex *pindexPrev = pindexBest;

  if(fProofOfStake)  // attemp to find a coinstake
  {
    pblock->nBits = GetNextTargetRequired(pindexPrev, true);
    CTransaction txCoinStake;
    int64 nSearchTime = txCoinStake.nTime; // search to current time
    if(nSearchTime > nLastCoinStakeSearchTime)
    {
      if(pwallet->CreateCoinStake(*pwallet, pblock->nBits, nSearchTime - nLastCoinStakeSearchTime, txCoinStake))
      {
        if(txCoinStake.nTime >= max(pindexPrev->GetMedianTimePast()+1, pindexPrev->GetBlockTime() - nMaxClockDrift))
        {   
          // make sure coinstake would meet timestamp protocol
          // as it would be the same as the block timestamp
          pblock->vtx[0].vout[0].SetEmpty();
          pblock->vtx[0].nTime = txCoinStake.nTime;

          pblock->vtx.push_back(txCoinStake);
        }
      }
      nLastCoinStakeSearchInterval = nSearchTime - nLastCoinStakeSearchTime;
      nLastCoinStakeSearchTime = nSearchTime;
    }
  }

  pblock->nBits = GetNextTargetRequired(pindexPrev, pblock->IsProofOfStake());

  // Collect memory pool transactions into the block
  int64 nFees = 0;
  {
    LOCK2(cs_main, mempool.cs);
    CTxDB txdb("r");

    // Priority order to process transactions
    list<COrphan> vOrphan; // list memory doesn't move
    map<uint256, vector<COrphan*> > mapDependers;
    multimap<double, CTransaction*> mapPriority;
    for(map<uint256, CTransaction>::iterator mi = mempool.mapTx.begin(); mi != mempool.mapTx.end(); ++mi)
    {
      CTransaction& tx = (*mi).second;
      if(tx.IsCoinBase() || tx.IsCoinStake() || !tx.IsFinal())
        continue;

      COrphan* porphan = NULL;
      double dPriority = 0;
      BOOST_FOREACH(const CTxIn& txin, tx.vin)
      {
        // Read prev transaction
        CTransaction txPrev;
        CTxIndex txindex;
        if(!txPrev.ReadFromDisk(txdb, txin.prevout, txindex))
        {
          // Has to wait for dependencies
          if(!porphan)
          {
            // Use list for automatic deletion
            vOrphan.push_back(COrphan(&tx));
            porphan = &vOrphan.back();
          }
          mapDependers[txin.prevout.hash].push_back(porphan);
          porphan->setDependsOn.insert(txin.prevout.hash);
          continue;
        }
        int64 nValueIn = txPrev.vout[txin.prevout.n].nValue;

        // Read block header
        int nConf = txindex.GetDepthInMainChain();

        dPriority += (double)nValueIn * nConf;

        if(fDebug && GetBoolArg("-printpriority"))
          printf("priority     nValueIn=%-12"PRI64d" nConf=%-5d dPriority=%-20.1f\n", nValueIn, nConf, dPriority);
      }

      // Priority is sum(valuein * age) / txsize
      dPriority /= ::GetSerializeSize(tx, SER_NETWORK, PROTOCOL_VERSION);

      if(porphan)
        porphan->dPriority = dPriority;
      else
        mapPriority.insert(make_pair(-dPriority, &(*mi).second));

      if(fDebug && GetBoolArg("-printpriority"))
      {
        printf("priority %-20.1f %s\n%s", 
               dPriority, tx.GetHash().ToString().substr(0,10).c_str(), tx.ToString().c_str());
        if(porphan)
          porphan->print();
        printf("\n");
      }
    }

    // Collect transactions into block
    map<uint256, CTxIndex> mapTestPool;
    uint64 nBlockSize = 1000;
    uint64 nBlockTx = 0;
    int nBlockSigOps = 100;
    while(!mapPriority.empty())
    {
      // Take highest priority transaction off priority queue
      CTransaction& tx = *(*mapPriority.begin()).second;
      mapPriority.erase(mapPriority.begin());

      // Size limits
      unsigned int nTxSize = ::GetSerializeSize(tx, SER_NETWORK, PROTOCOL_VERSION);
      if(nBlockSize + nTxSize >= MAX_BLOCK_SIZE_GEN)
        continue;

      // Legacy limits on sigOps:
      unsigned int nTxSigOps = tx.GetLegacySigOpCount();
      if(nBlockSigOps + nTxSigOps >= MAX_BLOCK_SIGOPS)
        continue;

      // Timestamp limit
      if(tx.nTime > GetAdjustedTime() || (pblock->IsProofOfStake() && tx.nTime > pblock->vtx[1].nTime))
        continue;

      // slimcoin: simplify transaction fee - allow free = false
      int64 nMinFee = tx.GetMinFee(nBlockSize, false, GMF_BLOCK);

      // Connecting shouldn't fail due to dependency on other memory pool transactions
      // because we're already processing them in order of dependency
      map<uint256, CTxIndex> mapTestPoolTmp(mapTestPool);
      MapPrevTx mapInputs;
      bool fInvalid;
      if(!tx.FetchInputs(txdb, mapTestPoolTmp, false, true, mapInputs, fInvalid))
        continue;

      int64 nTxFees = tx.GetValueIn(mapInputs) - tx.GetValueOut();

      if(nTxFees < nMinFee)
        continue;

      nTxSigOps += tx.GetP2SHSigOpCount(mapInputs);

      if(nBlockSigOps + nTxSigOps >= MAX_BLOCK_SIGOPS)
        continue;

      if(!tx.ConnectInputs(txdb, mapInputs, mapTestPoolTmp, CDiskTxPos(1,1,1), pindexPrev, false, true))
        continue;

      mapTestPoolTmp[tx.GetHash()] = CTxIndex(CDiskTxPos(1,1,1), tx.vout.size());
      swap(mapTestPool, mapTestPoolTmp);

      // Added
      pblock->vtx.push_back(tx);
      nBlockSize += nTxSize;
      ++nBlockTx;
      nBlockSigOps += nTxSigOps;
      nFees += nTxFees;

      // Add transactions that depend on this one to the priority queue
      uint256 hash = tx.GetHash();
      if(mapDependers.count(hash))
      {
        BOOST_FOREACH(COrphan* porphan, mapDependers[hash])
        {
          if(!porphan->setDependsOn.empty())
          {
            porphan->setDependsOn.erase(hash);
            if(porphan->setDependsOn.empty())
              mapPriority.insert(make_pair(-porphan->dPriority, porphan->ptx));
          }
        }
      }
    }

    nLastBlockTx = nBlockTx;
    nLastBlockSize = nBlockSize;
    if(fDebug && GetBoolArg("-printpriority"))
      printf("CreateNewBlock(): total size %lu\n", nBlockSize);

  }

  // Fill in header
  pblock->hashPrevBlock = pindexPrev->GetBlockHash();
  pblock->hashMerkleRoot = pblock->BuildMerkleTree();

  if(pblock->IsProofOfStake())
    pblock->nTime = pblock->vtx[1].nTime; //same as coinstake timestamp

  pblock->nTime = max(pindexPrev->GetMedianTimePast()+1, pblock->GetMaxTransactionTime());
  pblock->nTime = max(pblock->GetBlockTime(), pindexPrev->GetBlockTime() - nMaxClockDrift);

  if(pblock->IsProofOfWork() || pblock->IsProofOfBurn())
    pblock->UpdateTime(pindexPrev);

  pblock->nNonce = 0;

  //set the pblock's effective burn content
  int64 nBurnedCoins = 0;
  BOOST_FOREACH(const CTransaction &tx, pblock->vtx)
  {
    s32int burnOutTxIndex = tx.GetBurnOutTxIndex();
    if(burnOutTxIndex != -1) //this is a burn transaction
      nBurnedCoins += tx.vout[burnOutTxIndex].nValue;
  }

  //apply the decay only when this block is a proof of work block
  if(pblock->IsProofOfWork())
    //The new blocks nEffectiveBurnCoins is (the last blocks effective burn coins / BURN_DECAY_RATE) + nBurnCoins
    pblock->nEffectiveBurnCoins = (int64)((pindexPrev->nEffectiveBurnCoins / BURN_DECAY_RATE) + nBurnedCoins);
  else
    pblock->nEffectiveBurnCoins = pindexPrev->nEffectiveBurnCoins + nBurnedCoins;

  pblock->nBurnBits = GetNextBurnTargetRequired(pindexPrev);

  //Finally, set the block rewards
  if(pblock->IsProofOfWork())
    pblock->vtx[0].vout[0].nValue = GetProofOfWorkReward(pblock->nBits);
  else if(pblock->IsProofOfBurn())
    pblock->vtx[0].vout[0].nValue = GetProofOfBurnReward(pblock->nBurnBits);

  return pblock.release();
}


void IncrementExtraNonce(CBlock *pblock, CBlockIndex *pindexPrev, unsigned int &nExtraNonce)
{
  // Update nExtraNonce
  static uint256 hashPrevBlock;
  if(hashPrevBlock != pblock->hashPrevBlock)
  {
    nExtraNonce = 0;
    hashPrevBlock = pblock->hashPrevBlock;
  }
  ++nExtraNonce;
  pblock->vtx[0].vin[0].scriptSig = (CScript() << pblock->nTime << CBigNum(nExtraNonce)) + COINBASE_FLAGS;
  assert(pblock->vtx[0].vin[0].scriptSig.size() <= 100);

  pblock->hashMerkleRoot = pblock->BuildMerkleTree();

  return;
}


void FormatHashBuffers(CBlock *pblock, char *pmidstate, char *pdata, char *phash1)
{
  //
  // Prebuild hash buffers
  //
  struct
  {
    struct unnamed2
    {
      int nVersion;
      uint256 hashPrevBlock;
      uint256 hashMerkleRoot;
      unsigned int nTime;
      unsigned int nBits;
      unsigned int nNonce;
    } block;

    unsigned char pchPadding0[64];
    uint256 hash1;
    unsigned char pchPadding1[64];
  } tmp;
  memset(&tmp, 0x0, sizeof(tmp));

  tmp.block.nVersion       = pblock->nVersion;
  tmp.block.hashPrevBlock  = pblock->hashPrevBlock;
  tmp.block.hashMerkleRoot = pblock->hashMerkleRoot;
  tmp.block.nTime          = pblock->nTime;
  tmp.block.nBits          = pblock->nBits;
  tmp.block.nNonce         = pblock->nNonce;

  FormatHashBlocks(&tmp.block, sizeof(tmp.block));
  FormatHashBlocks(&tmp.hash1, sizeof(tmp.hash1));

  // Byte swap all the input buffer
  for(unsigned int i = 0; i < sizeof(tmp)/4; i++)
    ((unsigned int*)&tmp)[i] = ByteReverse(((unsigned int*)&tmp)[i]);

  // Precalc the first half of the first hash, which stays constant
  SHA256Transform(pmidstate, &tmp.block, pSHA256InitState);

  memcpy(pdata, &tmp.block, 128);
  memcpy(phash1, &tmp.hash1, 64);

  return;
}


bool CheckWork(CBlock *pblock, CWallet &wallet, CReserveKey &reservekey)
{
  uint256 hash = pblock->GetHash();
  uint256 burnHash = pblock->IsProofOfBurn() ? pblock->GetBurnHash(false) : ~uint256(0);

  uint256 hashTarget = CBigNum().SetCompact(pblock->nBits).getuint256();
  uint256 hashBurnTarget = CBigNum().SetCompact(pblock->nBurnBits).getuint256();

  if(pblock->IsProofOfWork() && hash > hashTarget)
    return error("SlimCoinMiner : proof-of-work not meeting target");

  if(pblock->IsProofOfBurn() && burnHash > hashBurnTarget)
    return error("SlimCoinMiner : proof-of-burn not meeting target");

  //// debug prints
  printf("\nSlimCoinMiner:\n");

  //It is useful to say what type of block was found
  string block_type;
  if(pblock->IsProofOfBurn())
    block_type = "Proof-of-Burn";
  else if(pblock->IsProofOfStake())
    block_type = "Proof-of-Stake";
  else //proof of work block
    block_type = "Proof-of-Work";

  printf("New %s block found\n", block_type.c_str());

  printf("\n");

  printf(" Block hash: %s\n", hash.GetHex().c_str());
  if(pblock->IsProofOfBurn())   //it is useful to print PoB specific information
  {
    printf("  Burn hash: %s\n", burnHash.GetHex().c_str());
     printf("Burn Target: %s\n", hashBurnTarget.GetHex().c_str());
  }
  printf("     Target: %s\n", hashTarget.GetHex().c_str());

  printf("\n");

  pblock->print();
  printf("%s ", DateTimeStrFormat(GetTime()).c_str());
  printf("generated %s\n", FormatMoney(pblock->vtx[0].vout[0].nValue).c_str());

  // Found a solution
  {
    LOCK(cs_main);
    if(pblock->hashPrevBlock != hashBestChain)
      return error("SlimCoinMiner : generated block is stale");

    // Remove key from key pool
    reservekey.KeepKey();

    // Track how many getdata requests this block gets
    {
      LOCK(wallet.cs_wallet);
      wallet.mapRequestCount[pblock->GetHash()] = 0;
    }

    // Process this block the same as if we had received it from another node
    if(!ProcessBlock(NULL, pblock))
      return error("SlimCoinMiner : ProcessBlock, block not accepted");
  }

  return true;
}

void static ThreadSlimcoinMiner(void* parg);

static bool fGenerateSlimcoins = false;
static bool fLimitProcessors = false;
static int nLimitProcessors = -1;

void SlimCoinMiner(CWallet *pwallet, bool fProofOfStake)
{
  printf("CPUMiner started for proof-of-%s\n", fProofOfStake ? "stake" : "work");
  SetThreadPriority(THREAD_PRIORITY_LOWEST);

  // Each thread has its own key and counter
  CReserveKey reservekey(pwallet);
  unsigned int nExtraNonce = 0;

  while(fGenerateSlimcoins || fProofOfStake)
  {
    if(fShutdown)
      return;

    while(vNodes.empty() || IsInitialBlockDownload())
    {
      Sleep(1000);
      if(fShutdown)
        return;
      if((!fGenerateSlimcoins) && !fProofOfStake)
        return;
    }

    while(pwallet->IsLocked())
    {
      strMintWarning = strMintMessage;
      Sleep(1000);
    }
    strMintWarning = "";

    //
    // Create new block
    //
    unsigned int nTransactionsUpdatedLast = nTransactionsUpdated;
    CBlockIndex* pindexPrev = pindexBest;

    auto_ptr<CBlock> pblock(CreateNewBlock(pwallet, fProofOfStake));
    if(!pblock.get())
      return;

    IncrementExtraNonce(pblock.get(), pindexPrev, nExtraNonce);

    if(fProofOfStake)
    {
      // slimcoin: if proof-of-stake block found then process block
      if(pblock->IsProofOfStake())
      {
        if(!pblock->SignBlock(*pwalletMain))
        {
          strMintWarning = strMintMessage;
          continue;
        }

        strMintWarning = "";
        printf("CPUMiner : proof-of-stake block found %s\n", pblock->GetHash().ToString().c_str()); 
        SetThreadPriority(THREAD_PRIORITY_NORMAL);
        CheckWork(pblock.get(), *pwalletMain, reservekey);
        SetThreadPriority(THREAD_PRIORITY_LOWEST);
      }

      Sleep(500);
      continue;
    }

    printf("Running SlimCoinMiner with %d %s in block\n", 
           pblock->vtx.size(), pblock->vtx.size() != 1 ? "transactions" : "transaction");


    //
    // Prebuild hash buffers
    //
    char pmidstatebuf[32+16]; char* pmidstate = alignup<16>(pmidstatebuf);
    char pdatabuf[128+16];    char* pdata     = alignup<16>(pdatabuf);
    char phash1buf[64+16];    char* phash1    = alignup<16>(phash1buf);

    FormatHashBuffers(pblock.get(), pmidstate, pdata, phash1);

    unsigned int& nBlockTime = *(unsigned int*)(pdata + 64 + 4);
    unsigned int& nBlockNonce = *(unsigned int*)(pdata + 64 + 12);


    //
    // Search
    //
    int64 nStart = GetTime();
    uint256 hashTarget = CBigNum().SetCompact(pblock->nBits).getuint256();
    uint256 test_hash;

    for(;;)
    {
      unsigned int nHashesDone = 0;
      unsigned int nNonceFound;

      nNonceFound = ScanDcryptHash(pblock.get(), &nHashesDone, &test_hash);

      // Check if something found
      if(nNonceFound != (unsigned int) -1)
      {

        if(test_hash <= hashTarget)
        {
          // Found a solution!
          assert(test_hash == pblock->GetHash());

          if(!pblock->SignBlock(*pwalletMain))
          {
            strMintWarning = strMintMessage;
            break;
          }

          strMintWarning = "";
          SetThreadPriority(THREAD_PRIORITY_NORMAL);
          CheckWork(pblock.get(), *pwalletMain, reservekey);
          SetThreadPriority(THREAD_PRIORITY_LOWEST);
          break;
        }
      }

      // Meter hashes/sec
      static int64 nHashCounter;
      if(!nHPSTimerStart)
      {
        nHPSTimerStart = GetTimeMillis();
        nHashCounter = 0;
      }else
        nHashCounter += nHashesDone;

      if(GetTimeMillis() - nHPSTimerStart > 4000)
      {
        static CCriticalSection cs;
        {
          LOCK(cs);
          if(GetTimeMillis() - nHPSTimerStart > 4000)
          {
            //times 1000 to get to seconds
            dHashesPerSec = 1000.0 * nHashCounter / (GetTimeMillis() - nHPSTimerStart);
            nHPSTimerStart = GetTimeMillis();
            nHashCounter = 0;
            static int64 nLogTime;

            //update with hashing speed information 30 secs
            if(GetTime() - nLogTime > 30)
            {
              nLogTime = GetTime();
              printf("%s ", DateTimeStrFormat(GetTime()).c_str());
              printf("hashmeter %3d CPUs %6.0f hash/s\n", vnThreadsRunning[THREAD_MINER], dHashesPerSec);
              printf("\tPoW Target: %s\n", hashTarget.ToString().c_str());
            }
          }
        }
      }

      // Check for stop or if block needs to be rebuilt
      if(fShutdown)
        return;
      if(!fGenerateSlimcoins)
        return;
      if(fLimitProcessors && vnThreadsRunning[THREAD_MINER] > nLimitProcessors)
        return;
      if(vNodes.empty())
        break;
      if(nBlockNonce >= 0xffff0000)
        break;
      if(nTransactionsUpdated != nTransactionsUpdatedLast && GetTime() - nStart > 60)
        break;
      if(pindexPrev != pindexBest)
        break;

      // Update nTime every few seconds
      pblock->nTime = max(pindexPrev->GetMedianTimePast() + 1, pblock->GetMaxTransactionTime());
      pblock->nTime = max(pblock->GetBlockTime(), pindexPrev->GetBlockTime() - nMaxClockDrift);
      pblock->UpdateTime(pindexPrev);
      nBlockTime = ByteReverse(pblock->nTime);

      if(pblock->GetBlockTime() >= (int64)pblock->vtx[0].nTime + nMaxClockDrift)
        break;  // need to update coinbase timestamp
    }
  }
}

void SlimCoinAfterBurner(CWallet *pwallet)
{
  printf("CPUMiner started for proof-of-burn\n");
  SetThreadPriority(THREAD_PRIORITY_LOWEST);

  // Each thread has its own key and counter
  CReserveKey reservekey(pwallet);
  unsigned int nExtraNonce = 0;
  CBlockIndex *pindexLastBlock = NULL;

  for(;;)
  {
    if(fShutdown)
      return;

    while(vNodes.empty() || IsInitialBlockDownload())
    {
      Sleep(1000);
      if(fShutdown)
        return;
    }

    while(pwallet->IsLocked())
    {
      strMintWarning = strMintMessage;
      Sleep(1000);
    }

    //if the best block in the chain has changed
    if(pindexLastBlock != pindexBest)
    {
      //record the best block
      pindexLastBlock = pindexBest;
      
      //calculate the smallest burn hash
      uint256 smallestHash;
      CWalletTx smallestWTx;
      HashAllBurntTx(smallestHash, smallestWTx);

      //if the smallest hash == 0xfffffffff..., that means there was some sort of error, so continue
      if(!smallestWTx.hashBlock || smallestHash == ~uint256(0))
        continue;
      
      //
      // Create new block
      //
      auto_ptr<CBlock> pblock(CreateNewBlock(pwallet, false, &smallestWTx));
      if(!pblock.get())
        continue;

      IncrementExtraNonce(pblock.get(), pindexLastBlock, nExtraNonce);

      uint256 hashTarget = CBigNum().SetCompact(pblock->nBurnBits).getuint256();

      //debug print
      printf("SlimCoinAfterBurner():\n");
      printf("\tSmallest Hash is %s\n", smallestHash.ToString().c_str());
      printf("\tby tx %s\n", smallestWTx.GetHash().ToString().c_str());
      printf("\twith Block height %d, transaction depth %d, vout depth %d\n", 
             mapBlockIndex.at(smallestWTx.hashBlock)->nHeight, 
             smallestWTx.nIndex, smallestWTx.GetBurnOutTxIndex());
      printf("\tPoB Tartget is %s\n", hashTarget.ToString().c_str());
      printf("\tnBurnBits=%08x, nEffectiveBurnCoins=%"PRI64u" (formatted %s)\n",
                      pblock->nBurnBits, pblock->nEffectiveBurnCoins, 
                      FormatMoney(pblock->nEffectiveBurnCoins).c_str());

      if(smallestHash <= hashTarget)
      {
        //Set the PoB flag and indexes
        pblock->fProofOfBurn = true;
        smallestWTx.SetBurnTxCoords(pblock->burnBlkHeight, pblock->burnCTx, pblock->burnCTxOut);

        //hash it as if it was not our block and test if the hash matches our claimed hash
        // pindexLastBlock->nHeight + 1 since use_burn_hash_intermediate works on the height of the current block
        uint256 hasher;
        GetBurnHash(pblock->hashPrevBlock, pblock->burnBlkHeight, pblock->burnCTx, 
                    pblock->burnCTxOut, hasher, use_burn_hash_intermediate(pindexLastBlock->nHeight + 1));
        
        //if this block's IsProofOfBurn() does not trigger, continue
        if(!pblock->IsProofOfBurn())
          continue;

        //TODO: CTransaction errors out when processing a block, no good, at main.cpp :: 491
        // possible has to do with this
        if(!pblock->SignBlock(*pwalletMain))
        {
          strMintWarning = strMintMessage;
          continue;
        }

        //the burn hash needs to be recorded
        pblock->burnHash = hasher;

        strMintWarning = "";
        printf("CPUMiner : proof-of-burn block found %s\n", pblock->GetHash().ToString().c_str()); 
        SetThreadPriority(THREAD_PRIORITY_NORMAL);
        CheckWork(pblock.get(), *pwalletMain, reservekey);
        SetThreadPriority(THREAD_PRIORITY_LOWEST);

      }

    }

    Sleep(1);
  }
  
  return;
}

void static ThreadSlimcoinMiner(void* parg)
{
  CWallet* pwallet = (CWallet*)parg;
  try
  {
    vnThreadsRunning[THREAD_MINER]++;
    SlimCoinMiner(pwallet, false);
    vnThreadsRunning[THREAD_MINER]--;
  }catch(std::exception& e)
  {
    vnThreadsRunning[THREAD_MINER]--;
    PrintException(&e, "ThreadSlimcoinMiner()");
  }catch(...)
  {
    vnThreadsRunning[THREAD_MINER]--;
    PrintException(NULL, "ThreadSlimcoinMiner()");
  }

  nHPSTimerStart = 0;

  if(!vnThreadsRunning[THREAD_MINER] == 0)
    dHashesPerSec = 0;

  printf("ThreadSlimcoinMiner exiting, %d threads remaining\n", vnThreadsRunning[THREAD_MINER]);
}

void GenerateSlimcoins(bool fGenerate, CWallet* pwallet)
{
  fGenerateSlimcoins = fGenerate;
  nLimitProcessors = GetArg("-genproclimit", -1);

  if(!nLimitProcessors)
    fGenerateSlimcoins = false;

  fLimitProcessors = (nLimitProcessors != -1);

  if(fGenerate)
  {
    int nProcessors = boost::thread::hardware_concurrency();
    printf("%d processors\n", nProcessors);

    //there must be atleast one CPU core
    if(nProcessors < 1)
      nProcessors = 1;

    if(fLimitProcessors && nProcessors > nLimitProcessors)
      nProcessors = nLimitProcessors;

    int nAddThreads = nProcessors - vnThreadsRunning[THREAD_MINER];
    printf("Starting %d SlimCoinMiner threads\n", nAddThreads);

    for(int i = 0; i < nAddThreads; i++)
    {
      if(!CreateThread(ThreadSlimcoinMiner, pwallet))
        printf("Error: CreateThread(ThreadSlimcoinMiner) failed\n");

      Sleep(10);
    }
  }
}
