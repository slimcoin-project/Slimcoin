// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2012 The Bitcoin developers
// Copyright (c) 2011-2013 The Peercoin developers
// Copyright (c) 2013-2014 The Slimcoin Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "db.h"
#include "net.h"
#include "checkpoints.h"
#include "util.h"
#include "main.h"
#include "kernel.h"
#include "ui_interface.h"
#include <boost/version.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

#ifndef WIN32
#include "sys/stat.h"
#endif

using namespace std;
using namespace boost;


unsigned int nWalletDBUpdated;



//
// CDB
//

CCriticalSection cs_db;
static bool fDbEnvInit = false;
bool fDetachDB = false;
DbEnv dbenv(0);
map<string, int> mapFileUseCount;
static map<string, Db*> mapDb;

static void EnvShutdown()
{
  if(!fDbEnvInit)
    return;

  fDbEnvInit = false;
  try
  {
    dbenv.close(0);
  }
  catch (const DbException& e)
  {
    printf("EnvShutdown exception: %s (%d)\n", e.what(), e.get_errno());
  }
  DbEnv(0).remove(GetDataDir().string().c_str(), 0);
}

class CDBInit
{
public:
  CDBInit()
  {
  }
  ~CDBInit()
  {
    EnvShutdown();
  }
}
  instance_of_cdbinit;


CDB::CDB(const char *pszFile, const char* pszMode) : pdb(NULL)
{
  int ret;
  if(pszFile == NULL)
    return;

  fReadOnly = (!strchr(pszMode, '+') && !strchr(pszMode, 'w'));
  bool fCreate = strchr(pszMode, 'c');
  unsigned int nFlags = DB_THREAD;
  if(fCreate)
    nFlags |= DB_CREATE;

  {
    LOCK(cs_db);
    if(!fDbEnvInit)
    {
      if(fShutdown)
        return;
      filesystem::path pathDataDir = GetDataDir();
      filesystem::path pathLogDir = pathDataDir / "database";
      filesystem::create_directory(pathLogDir);
      filesystem::path pathErrorFile = pathDataDir / "db.log";
      printf("dbenv.open LogDir=%s ErrorFile=%s\n", pathLogDir.string().c_str(), pathErrorFile.string().c_str());

      int nDbCache = GetArg("-dbcache", 25);
      dbenv.set_lg_dir(pathLogDir.string().c_str());
      dbenv.set_cachesize(nDbCache / 1024, (nDbCache % 1024)*1048576, 1);
      dbenv.set_lg_bsize(1048576);
      dbenv.set_lg_max(10485760);
      dbenv.set_lk_max_locks(10000);
      dbenv.set_lk_max_objects(10000);
      dbenv.set_errfile(fopen(pathErrorFile.string().c_str(), "a")); /// debug
      dbenv.set_flags(DB_TXN_WRITE_NOSYNC, 1);
      dbenv.set_flags(DB_AUTO_COMMIT, 1);
      dbenv.log_set_config(DB_LOG_AUTO_REMOVE, 1);
      ret = dbenv.open(pathDataDir.string().c_str(),
                       DB_CREATE     |
                       DB_INIT_LOCK  |
                       DB_INIT_LOG   |
                       DB_INIT_MPOOL |
                       DB_INIT_TXN   |
                       DB_THREAD     |
                       DB_RECOVER,
                       S_IRUSR | S_IWUSR);
      if(ret > 0)
        throw runtime_error(strprintf("CDB() : error %d opening database environment", ret));
      fDbEnvInit = true;
    }

    strFile = pszFile;
    ++mapFileUseCount[strFile];
    pdb = mapDb[strFile];
    if(pdb == NULL)
    {
      pdb = new Db(&dbenv, 0);

      ret = pdb->open(NULL,      // Txn pointer
                      pszFile,   // Filename
                      "main",    // Logical db name
                      DB_BTREE,  // Database type
                      nFlags,    // Flags
                      0);

      if(ret > 0)
      {
        delete pdb;
        pdb = NULL;
        {
          LOCK(cs_db);
          --mapFileUseCount[strFile];
        }
        strFile = "";
        throw runtime_error(strprintf("CDB() : can't open database file %s, error %d", pszFile, ret));
      }

      if(fCreate && !Exists(string("version")))
      {
        bool fTmp = fReadOnly;
        fReadOnly = false;
        WriteVersion(CLIENT_VERSION);
        fReadOnly = fTmp;
      }

      mapDb[strFile] = pdb;
    }
  }
}

void CDB::Close()
{
  if(!pdb)
    return;
  if(!vTxn.empty())
    vTxn.front()->abort();
  vTxn.clear();
  pdb = NULL;

  // Flush database activity from memory pool to disk log
  unsigned int nMinutes = 0;
  if(fReadOnly)
    nMinutes = 1;
  if(strFile == "addr.dat")
    nMinutes = 2;
  if(strFile == "blkindex.dat")
    nMinutes = 2;
  if(strFile == "blkindex.dat" && IsInitialBlockDownload())
    nMinutes = 5;

  dbenv.txn_checkpoint(nMinutes ? GetArg("-dblogsize", 100)*1024 : 0, nMinutes, 0);

  {
    LOCK(cs_db);
    --mapFileUseCount[strFile];
  }
}

void CloseDb(const string& strFile)
{
  {
    LOCK(cs_db);
    if(mapDb[strFile] != NULL)
    {
      // Close the database handle
      Db* pdb = mapDb[strFile];
      pdb->close(0);
      delete pdb;
      mapDb[strFile] = NULL;
    }
  }
}

bool CDB::Rewrite(const string& strFile, const char* pszSkip)
{
  while(!fShutdown)
  {
    {
      LOCK(cs_db);
      if(!mapFileUseCount.count(strFile) || mapFileUseCount[strFile] == 0)
      {
        // Flush log data to the dat file
        CloseDb(strFile);
        dbenv.txn_checkpoint(0, 0, 0);
        dbenv.lsn_reset(strFile.c_str(), 0);
        mapFileUseCount.erase(strFile);

        bool fSuccess = true;
        printf("Rewriting %s...\n", strFile.c_str());
        string strFileRes = strFile + ".rewrite";
        { // surround usage of db with extra {}
          CDB db(strFile.c_str(), "r");
          Db* pdbCopy = new Db(&dbenv, 0);
    
          int ret = pdbCopy->open(NULL,                 // Txn pointer
                                  strFileRes.c_str(),   // Filename
                                  "main",    // Logical db name
                                  DB_BTREE,  // Database type
                                  DB_CREATE,    // Flags
                                  0);
          if(ret > 0)
          {
            printf("Cannot create database file %s\n", strFileRes.c_str());
            fSuccess = false;
          }
    
          Dbc* pcursor = db.GetCursor();
          if(pcursor)
            while(fSuccess)
            {
              CDataStream ssKey(SER_DISK, CLIENT_VERSION);
              CDataStream ssValue(SER_DISK, CLIENT_VERSION);
              int ret = db.ReadAtCursor(pcursor, ssKey, ssValue, DB_NEXT);
              if(ret == DB_NOTFOUND)
              {
                pcursor->close();
                break;
              }
              else if(ret != 0)
              {
                pcursor->close();
                fSuccess = false;
                break;
              }
              if(pszSkip &&
                 strncmp(&ssKey[0], pszSkip, std::min(ssKey.size(), strlen(pszSkip))) == 0)
                continue;
              if(strncmp(&ssKey[0], "\x07version", 8) == 0)
              {
                // Update version:
                ssValue.clear();
                ssValue << CLIENT_VERSION;
              }
              Dbt datKey(&ssKey[0], ssKey.size());
              Dbt datValue(&ssValue[0], ssValue.size());
              int ret2 = pdbCopy->put(NULL, &datKey, &datValue, DB_NOOVERWRITE);
              if(ret2 > 0)
                fSuccess = false;
            }
          if(fSuccess)
          {
            db.Close();
            CloseDb(strFile);
            if(pdbCopy->close(0))
              fSuccess = false;
            delete pdbCopy;
          }
        }
        if(fSuccess)
        {
          Db dbA(&dbenv, 0);
          if(dbA.remove(strFile.c_str(), NULL, 0))
            fSuccess = false;
          Db dbB(&dbenv, 0);
          if(dbB.rename(strFileRes.c_str(), NULL, strFile.c_str(), 0))
            fSuccess = false;
        }
        if(!fSuccess)
          printf("Rewriting of %s FAILED!\n", strFileRes.c_str());
        return fSuccess;
      }
    }
    Sleep(100);
  }
  return false;
}


void DBFlush(bool fShutdown)
{
  // Flush log data to the actual data file
  //  on all files that are not in use
  printf("DBFlush(%s)%s\n", fShutdown ? "true" : "false", fDbEnvInit ? "" : " db not started");
  if(!fDbEnvInit)
    return;
  {
    LOCK(cs_db);
    map<string, int>::iterator mi = mapFileUseCount.begin();
    while(mi != mapFileUseCount.end())
    {
      string strFile = (*mi).first;
      int nRefCount = (*mi).second;
      printf("%s refcount=%d\n", strFile.c_str(), nRefCount);
      if(nRefCount == 0)
      {
        // Move log data to the dat file
        CloseDb(strFile);
        printf("%s checkpoint\n", strFile.c_str());
        dbenv.txn_checkpoint(0, 0, 0);
        if((strFile != "blkindex.dat" && strFile != "addr.dat") || fDetachDB) {
          printf("%s detach\n", strFile.c_str());
          dbenv.lsn_reset(strFile.c_str(), 0);
        }
        printf("%s closed\n", strFile.c_str());
        mapFileUseCount.erase(mi++);
      }
      else
        mi++;
    }
    if(fShutdown)
    {
      char** listp;
      if(mapFileUseCount.empty())
      {
        dbenv.log_archive(&listp, DB_ARCH_REMOVE);
        EnvShutdown();
      }
    }
  }
}






//
// CTxDB
//

bool CTxDB::ReadTxIndex(uint256 hash, CTxIndex& txindex)
{
  assert(!fClient);
  txindex.SetNull();
  return Read(make_pair(string("tx"), hash), txindex);
}

bool CTxDB::UpdateTxIndex(uint256 hash, const CTxIndex& txindex)
{
  assert(!fClient);
  return Write(make_pair(string("tx"), hash), txindex);
}

bool CTxDB::AddTxIndex(const CTransaction& tx, const CDiskTxPos& pos, int nHeight)
{
  assert(!fClient);

  // Add to tx index
  uint256 hash = tx.GetHash();
  CTxIndex txindex(pos, tx.vout.size());
  return Write(make_pair(string("tx"), hash), txindex);
}

bool CTxDB::EraseTxIndex(const CTransaction& tx)
{
  assert(!fClient);
  uint256 hash = tx.GetHash();

  return Erase(make_pair(string("tx"), hash));
}

bool CTxDB::ContainsTx(uint256 hash)
{
  assert(!fClient);
  return Exists(make_pair(string("tx"), hash));
}

bool CTxDB::ReadOwnerTxes(uint160 hash160, int nMinHeight, vector<CTransaction>& vtx)
{
  assert(!fClient);
  vtx.clear();

  // Get cursor
  Dbc* pcursor = GetCursor();
  if(!pcursor)
    return false;

  unsigned int fFlags = DB_SET_RANGE;
  loop
  {
    // Read next record
    CDataStream ssKey(SER_DISK, CLIENT_VERSION);
    if(fFlags == DB_SET_RANGE)
      ssKey << string("owner") << hash160 << CDiskTxPos(0, 0, 0);
    CDataStream ssValue(SER_DISK, CLIENT_VERSION);
    int ret = ReadAtCursor(pcursor, ssKey, ssValue, fFlags);
    fFlags = DB_NEXT;
    if(ret == DB_NOTFOUND)
      break;
    else if(ret != 0)
    {
      pcursor->close();
      return false;
    }

    // Unserialize
    string strType;
    uint160 hashItem;
    CDiskTxPos pos;
    int nItemHeight;

    try {
      ssKey >> strType >> hashItem >> pos;
      ssValue >> nItemHeight;
    }
    catch (std::exception &e) {
      return error("%s() : deserialize error", __PRETTY_FUNCTION__);
    }

    // Read transaction
    if(strType != "owner" || hashItem != hash160)
      break;
    if(nItemHeight >= nMinHeight)
    {
      vtx.resize(vtx.size()+1);
      if(!vtx.back().ReadFromDisk(pos))
      {
        pcursor->close();
        return false;
      }
    }
  }

  pcursor->close();
  return true;
}

bool CTxDB::ReadDiskTx(uint256 hash, CTransaction& tx, CTxIndex& txindex)
{
  assert(!fClient);
  tx.SetNull();
  if(!ReadTxIndex(hash, txindex))
    return false;
  return (tx.ReadFromDisk(txindex.pos));
}

bool CTxDB::ReadDiskTx(uint256 hash, CTransaction& tx)
{
  CTxIndex txindex;
  return ReadDiskTx(hash, tx, txindex);
}

bool CTxDB::ReadDiskTx(COutPoint outpoint, CTransaction& tx, CTxIndex& txindex)
{
  return ReadDiskTx(outpoint.hash, tx, txindex);
}

bool CTxDB::ReadDiskTx(COutPoint outpoint, CTransaction& tx)
{
  CTxIndex txindex;
  return ReadDiskTx(outpoint.hash, tx, txindex);
}

bool CTxDB::WriteBlockIndex(const CDiskBlockIndex& blockindex)
{
  return Write(make_pair(string("blockindex"), blockindex.GetBlockHash()), blockindex);
}

bool CTxDB::EraseBlockIndex(uint256 hash)
{
  return Erase(make_pair(string("blockindex"), hash));
}

bool CTxDB::ReadHashBestChain(uint256& hashBestChain)
{
  return Read(string("hashBestChain"), hashBestChain);
}

bool CTxDB::WriteHashBestChain(uint256 hashBestChain)
{
  return Write(string("hashBestChain"), hashBestChain);
}

bool CTxDB::ReadBestInvalidTrust(CBigNum& bnBestInvalidTrust)
{
  return Read(string("bnBestInvalidTrust"), bnBestInvalidTrust);
}

bool CTxDB::WriteBestInvalidTrust(CBigNum bnBestInvalidTrust)
{
  return Write(string("bnBestInvalidTrust"), bnBestInvalidTrust);
}

bool CTxDB::ReadSyncCheckpoint(uint256& hashCheckpoint)
{
  return Read(string("hashSyncCheckpoint"), hashCheckpoint);
}

bool CTxDB::WriteSyncCheckpoint(uint256 hashCheckpoint)
{
  return Write(string("hashSyncCheckpoint"), hashCheckpoint);
}

bool CTxDB::ReadCheckpointPubKey(string& strPubKey)
{
  return Read(string("strCheckpointPubKey"), strPubKey);
}

bool CTxDB::WriteCheckpointPubKey(const string& strPubKey)
{
  return Write(string("strCheckpointPubKey"), strPubKey);
}

static CBlockIndex *InsertBlockIndex(uint256 hash)
{
  if(!hash)
    return NULL;

  // Return existing
  map<uint256, CBlockIndex*>::iterator mi = mapBlockIndex.find(hash);
  if(mi != mapBlockIndex.end())
    return (*mi).second;

  // Create new
  CBlockIndex *pindexNew = new CBlockIndex();

  if(!pindexNew)
    throw runtime_error("LoadBlockIndex() : new CBlockIndex failed");

  mi = mapBlockIndex.insert(make_pair(hash, pindexNew)).first;
  pindexNew->phashBlock = &((*mi).first);

  return pindexNew;
}

bool CTxDB::LoadBlockIndex()
{
  // Get database cursor
  Dbc* pcursor = GetCursor();
  if(!pcursor)
    return false;

  // Load mapBlockIndex
  unsigned int fFlags = DB_SET_RANGE;
  for(;;)
  {    
    // Read next record
    CDataStream ssKey(SER_DISK, CLIENT_VERSION);
    if(fFlags == DB_SET_RANGE)
      ssKey << make_pair(string("blockindex"), uint256(0));
    CDataStream ssValue(SER_DISK, CLIENT_VERSION);
    int ret = ReadAtCursor(pcursor, ssKey, ssValue, fFlags);
    fFlags = DB_NEXT;

    if(ret == DB_NOTFOUND)
      break;
    else if(ret)
      return false;

    // Unserialize
    
    try
    {
      string strType;
      ssKey >> strType;
      if(strType == "blockindex" && !fRequestShutdown)
      {
        CDiskBlockIndex diskindex;
        ssValue >> diskindex;

        uint256 blockHash = diskindex.GetBlockHash();

        // Construct block index object
        CBlockIndex *pindexNew    = InsertBlockIndex(blockHash);
        pindexNew->pprev          = InsertBlockIndex(diskindex.hashPrev);
        pindexNew->pnext          = InsertBlockIndex(diskindex.hashNext);

        pindexNew->nFile          = diskindex.nFile;
        pindexNew->nBlockPos      = diskindex.nBlockPos;
        pindexNew->nHeight        = diskindex.nHeight;
        pindexNew->nMint          = diskindex.nMint;
        pindexNew->nMoneySupply   = diskindex.nMoneySupply;
        pindexNew->nFlags         = diskindex.nFlags;
        pindexNew->nStakeModifier = diskindex.nStakeModifier;
        pindexNew->prevoutStake   = diskindex.prevoutStake;
        pindexNew->nStakeTime     = diskindex.nStakeTime;
        pindexNew->hashProofOfStake = diskindex.hashProofOfStake;
        pindexNew->nVersion       = diskindex.nVersion;
        pindexNew->hashMerkleRoot = diskindex.hashMerkleRoot;
        pindexNew->nTime          = diskindex.nTime;
        pindexNew->nBits          = diskindex.nBits;
        pindexNew->nNonce         = diskindex.nNonce;

        //Load Proof of Burn switch and indexes
        pindexNew->fProofOfBurn   = diskindex.fProofOfBurn;
        pindexNew->burnHash       = diskindex.burnHash;
        pindexNew->burnBlkHeight  = diskindex.burnBlkHeight;
        pindexNew->burnCTx        = diskindex.burnCTx;
        pindexNew->burnCTxOut     = diskindex.burnCTxOut;
        pindexNew->nEffectiveBurnCoins = diskindex.nEffectiveBurnCoins;
        pindexNew->nBurnBits      = diskindex.nBurnBits;

        // Watch for genesis block
        if(pindexGenesisBlock == NULL && blockHash == hashGenesisBlock)
          pindexGenesisBlock = pindexNew;

        if(!pindexNew->CheckIndex())
          return error("LoadBlockIndex() : CheckIndex failed at %d", pindexNew->nHeight);

        // slimcoin: build setStakeSeen
        if(pindexNew->IsProofOfStake())
          setStakeSeen.insert(pindexNew->GetProofOfStake());
        else if(pindexNew->IsProofOfBurn()) //build the setBurnSeen
          setBurnSeen.insert(pindexNew->GetProofOfBurn());

      }else
        break; // if shutdown requested or finished loading block index

    }    // try
    catch(std::exception &e)
    {
      return error("LoadBlockIndex() : deserialize error : %s", e.what());
    }
  }

  pcursor->close();

  if(fRequestShutdown)
    return true;

  // Calculate bnChainTrust
  vector<pair<int, CBlockIndex*> > vSortedByHeight;
  vSortedByHeight.reserve(mapBlockIndex.size());
  BOOST_FOREACH(const PAIRTYPE(uint256, CBlockIndex*)& item, mapBlockIndex)
  {
    CBlockIndex* pindex = item.second;
    vSortedByHeight.push_back(make_pair(pindex->nHeight, pindex));
  }
  sort(vSortedByHeight.begin(), vSortedByHeight.end());
  BOOST_FOREACH(const PAIRTYPE(int, CBlockIndex*)& item, vSortedByHeight)
  {
    CBlockIndex* pindex = item.second;
    pindex->bnChainTrust = (pindex->pprev ? pindex->pprev->bnChainTrust : 0) + pindex->GetBlockTrust();
    // slimcoin: calculate stake modifier checksum
    pindex->nStakeModifierChecksum = GetStakeModifierChecksum(pindex);
    if(!CheckStakeModifierCheckpoints(pindex->nHeight, pindex->nStakeModifierChecksum))
      return error("CTxDB::LoadBlockIndex() : Failed stake modifier checkpoint height=%d, modifier=0x%016"PRI64x, pindex->nHeight, pindex->nStakeModifier);
  }

  // Load hashBestChain pointer to end of best chain
  if(!ReadHashBestChain(hashBestChain))
  {
    if(pindexGenesisBlock == NULL)
      return true;
    return error("CTxDB::LoadBlockIndex() : hashBestChain not loaded");
  }
  if(!mapBlockIndex.count(hashBestChain))
    return error("CTxDB::LoadBlockIndex() : hashBestChain not found in the block index");
  pindexBest = mapBlockIndex[hashBestChain];
  nBestHeight = pindexBest->nHeight;
  bnBestChainTrust = pindexBest->bnChainTrust;
  printf("LoadBlockIndex(): hashBestChain=%s  height=%d  trust=%s\n", hashBestChain.ToString().substr(0,20).c_str(), nBestHeight, bnBestChainTrust.ToString().c_str());

  // slimcoin: load hashSyncCheckpoint
  if(!ReadSyncCheckpoint(Checkpoints::hashSyncCheckpoint))
    return error("CTxDB::LoadBlockIndex() : hashSyncCheckpoint not loaded");
  printf("LoadBlockIndex(): synchronized checkpoint %s\n", Checkpoints::hashSyncCheckpoint.ToString().c_str());

  // Load bnBestInvalidTrust, OK if it doesn't exist
  ReadBestInvalidTrust(bnBestInvalidTrust);

  // Verify blocks in the best chain
  int nCheckLevel = GetArg("-checklevel", 1);
  int nCheckDepth = GetArg("-checkblocks", 2500);

  if(!nCheckDepth)
    nCheckDepth = 1000000000; // suffices until the year 19000
  if(nCheckDepth > nBestHeight)
    nCheckDepth = nBestHeight;

  {
    //after all of the indexes are loaded, now check the proof-of-burn blocks
    for(const CBlockIndex *pindex = pindexBest; pindex && pindex->pprev; pindex = pindex->pprev)
    {

      //check it if the fUseFastIndex is false or
      // if this index is a proof-of-burn as was found within the last 24 hours, check it
      if(!fUseFastIndex || (pindex->IsProofOfBurn() && pindex->nTime > GetAdjustedTime() - 24 * 60 * 60))
      {

        uint256 burnHashRet;

        /*
         * Check if the burn hash meets the burn target requirement
         */

        //Get the actual burn hash, with the multiplier applied
        // nHeight - 1 since GetBurnHash wants the index of the previous block
        GetBurnHash(pindex->pprev->GetBlockHash(), pindex->burnBlkHeight, pindex->burnCTx, 
                    pindex->burnCTxOut, burnHashRet, false);

        if(!CheckProofOfBurnHash(burnHashRet, pindex->nBurnBits))
          return error("%s : deserialize error on PoB index %d", __PRETTY_FUNCTION__, pindex->nHeight);

        /*
         * If Slimcoin is past the intermediate hash update, check that too
         */

        //apply the checking of burn hashes if the height is past the burn intermediate hash change in the client
        const bool fUseIntermediate = use_burn_hash_intermediate(pindex->nTime);

        if(fUseIntermediate)
        {
          //Get the intermediate burn hash without the multiplier applied
          // nHeight - 1 since GetBurnHash wants the index of the previous block
          GetBurnHash(pindex->pprev->GetBlockHash(), pindex->burnBlkHeight, pindex->burnCTx, 
                      pindex->burnCTxOut, burnHashRet, true);

          if(pindex->burnHash != burnHashRet)
            return error("%s : deserialize error on PoB index %d", __PRETTY_FUNCTION__, pindex->nHeight);
        }
      }
    }
  }

  InitMessage("Verifying blocks...");
  printf("Verifying last %i blocks at level %i\n", nCheckDepth, nCheckLevel);
  CBlockIndex* pindexFork = NULL;
  map<pair<unsigned int, unsigned int>, CBlockIndex*> mapBlockPos;

  for(CBlockIndex* pindex = pindexBest; pindex && pindex->pprev; pindex = pindex->pprev)
  {
    if(fRequestShutdown || pindex->nHeight < nBestHeight - nCheckDepth)
      break;

    CBlock block;
    if(!block.ReadFromDisk(pindex))
      return error("LoadBlockIndex() : block.ReadFromDisk failed");

    // Debug print
    printf("Block %3d has this many transactions in it: %d, Block Pos %d\n", 
           pindex->nHeight, block.vtx.size(), pindex->nBlockPos);

    // check level 1: verify block validity
    if(nCheckLevel > 0 && !block.CheckBlock())
    {
      printf("LoadBlockIndex() : *** found bad block at %d, hash=%s\n", 
             pindex->nHeight, pindex->GetBlockHash().ToString().c_str());
      pindexFork = pindex->pprev;
    }

    // check level 2: verify transaction index validity
    if(nCheckLevel > 1)
    {
      pair<unsigned int, unsigned int> pos = make_pair(pindex->nFile, pindex->nBlockPos);
      mapBlockPos[pos] = pindex;
      BOOST_FOREACH(const CTransaction &tx, block.vtx)
      {
        uint256 hashTx = tx.GetHash();
        CTxIndex txindex;
        if(ReadTxIndex(hashTx, txindex))
        {
          // check level 3: checker transaction hashes
          if(nCheckLevel > 2 || pindex->nFile != txindex.pos.nFile || pindex->nBlockPos != txindex.pos.nBlockPos)
          {
            // either an error or a duplicate transaction
            CTransaction txFound;
            if(!txFound.ReadFromDisk(txindex.pos))
            {
              printf("LoadBlockIndex() : *** cannot read mislocated transaction %s\n", hashTx.ToString().c_str());
              pindexFork = pindex->pprev;
            }
            else
              if(txFound.GetHash() != hashTx) // not a duplicate tx
              {
                printf("LoadBlockIndex(): *** invalid tx position for %s\n", hashTx.ToString().c_str());
                pindexFork = pindex->pprev;
              }
          }
          // check level 4: check whether spent txouts were spent within the main chain
          unsigned int nOutput = 0;
          if(nCheckLevel > 3)
          {
            BOOST_FOREACH(const CDiskTxPos &txpos, txindex.vSpent)
            {
              if(!txpos.IsNull())
              {
                pair<unsigned int, unsigned int> posFind = make_pair(txpos.nFile, txpos.nBlockPos);
                if(!mapBlockPos.count(posFind))
                {
                  printf("LoadBlockIndex(): *** found bad spend at %d, hashBlock=%s, hashTx=%s\n", pindex->nHeight, pindex->GetBlockHash().ToString().c_str(), hashTx.ToString().c_str());
                  pindexFork = pindex->pprev;
                }
                // check level 6: check whether spent txouts were spent by a valid transaction that consume them
                if(nCheckLevel > 5)
                {
                  CTransaction txSpend;
                  if(!txSpend.ReadFromDisk(txpos))
                  {
                    printf("LoadBlockIndex(): *** cannot read spending transaction of %s:%i from disk\n", hashTx.ToString().c_str(), nOutput);
                    pindexFork = pindex->pprev;
                  }
                  else if(!txSpend.CheckTransaction())
                  {
                    printf("LoadBlockIndex(): *** spending transaction of %s:%i is invalid\n", hashTx.ToString().c_str(), nOutput);
                    pindexFork = pindex->pprev;
                  }
                  else
                  {
                    bool fFound = false;
                    BOOST_FOREACH(const CTxIn &txin, txSpend.vin)
                      if(txin.prevout.hash == hashTx && txin.prevout.n == nOutput)
                        fFound = true;
                    if(!fFound)
                    {
                      printf("LoadBlockIndex(): *** spending transaction of %s:%i does not spend it\n", hashTx.ToString().c_str(), nOutput);
                      pindexFork = pindex->pprev;
                    }
                  }
                }
              }
              nOutput++;
            }
          }
        }

        // check level 5: check whether all prevouts are marked spent
        if(nCheckLevel > 4)
        {
          BOOST_FOREACH(const CTxIn &txin, tx.vin)
          {
            CTxIndex txindex;
            if(ReadTxIndex(txin.prevout.hash, txindex))
              if(txindex.vSpent.size()-1 < txin.prevout.n || txindex.vSpent[txin.prevout.n].IsNull())
              {
                printf("LoadBlockIndex(): *** found unspent prevout %s:%i in %s\n", txin.prevout.hash.ToString().c_str(), txin.prevout.n, hashTx.ToString().c_str());
                pindexFork = pindex->pprev;
              }
          }
        }
      }
    }
  }

  {
    //start the lastCheckpointHeight at the genesis block
    s32int lastCheckpointHeight = 0;
    const CBlockIndex *lastCheckpointIndex = NULL;

    //go through the indexes again, if the pindex's height matches a hard coded checkpoints, but their
    // hashes differ, set pindexFork to the last hard coded checkpoint
    // starts from front to back since if there is a fork found, once we set the pindexFork, it is
    // not needed to continue checking since every other block ahead will be in the forked blockchain
    for(const CBlockIndex *pindex = pindexGenesisBlock; pindex; pindex = pindex->pnext)
    {
      //if a checkpoint of height, pindex->nHeight does not exist, continue
      if(!Checkpoints::CheckpointExists(pindex->nHeight))
        continue;

      //if the pindex's height is in the mapCheckpoints and the hashes differ
      if(!Checkpoints::CheckHardened(pindex->nHeight, pindex->GetBlockHash()))
      {
        printf("Block index at height %d mismatches hard checkpoint's hash\n", pindex->nHeight);
        //only set the pindexFork if it is either NULL or on a block after the lastCheckpointHeight
        if(!pindexFork || pindexFork->nHeight > lastCheckpointHeight)
        {
          printf("\tpindexFork set to last checkpoint at height %d\n", lastCheckpointHeight);
          pindexFork = (CBlockIndex*)lastCheckpointIndex;
        }

        //break since a fork was set, and everything after it will be forked off
        // pindexFork may sometimes not be set if it is set with a pindex, which is
        // before lastCheckpointHeight
        break;
      }else{
        //record this pindex and the last nHeight checkpoint
        lastCheckpointHeight = pindex->nHeight;
        lastCheckpointIndex = pindex;
      }
    }
  }

  if(pindexFork)
  {
    // Reorg back to the fork
    printf("LoadBlockIndex() : *** moving best chain pointer back to block %d\n", pindexFork->nHeight);
    CBlock block;
    if(!block.ReadFromDisk(pindexFork))
      return error("LoadBlockIndex() : block.ReadFromDisk failed");
    CTxDB txdb;
    block.SetBestChain(txdb, pindexFork);
  }

  return true;
}





//
// CAddrDB
//

bool CAddrDB::WriteAddrman(const CAddrMan& addrman)
{
  return Write(string("addrman"), addrman);
}

bool CAddrDB::LoadAddresses()
{
  if(Read(string("addrman"), addrman))
  {
    printf("Loaded %i addresses\n", addrman.size());
    return true;
  }
    
  // Read pre-0.6 addr records

  vector<CAddress> vAddr;
  vector<vector<unsigned char> > vDelete;

  // Get cursor
  Dbc* pcursor = GetCursor();
  if(!pcursor)
    return false;

  for(;;)
  {
    // Read next record
    CDataStream ssKey(SER_DISK, CLIENT_VERSION);
    CDataStream ssValue(SER_DISK, CLIENT_VERSION);
    int ret = ReadAtCursor(pcursor, ssKey, ssValue);
    if(ret == DB_NOTFOUND)
      break;
    else if(ret != 0)
      return false;

    // Unserialize
    string strType;
    ssKey >> strType;
    if(strType == "addr")
    {
      CAddress addr;
      ssValue >> addr;
      vAddr.push_back(addr);
    }
  }
  pcursor->close();

  addrman.Add(vAddr, CNetAddr("0.0.0.0"));
  printf("Loaded %i addresses\n", addrman.size());

  // Note: old records left; we ran into hangs-on-startup
  // bugs for some users who (we think) were running after
  // an unclean shutdown.

  return true;
}

bool LoadAddresses()
{
  return CAddrDB("cr+").LoadAddresses();
}


