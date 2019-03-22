#! /usr/bin/env python
"""
Routines to read and parse a linearized Slimcoin boostrap.dat file

The C++ classes and data structures to be parsed.
/* ---------------------------------------------------------------- */
class CBlock
{
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

    // ppcoin: block signature - signed by coin base txout[0]'s owner
    std::vector<unsigned char> vchBlockSig;

    IMPLEMENT_SERIALIZE
    (
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

            READWRITE(vtx);
            READWRITE(vchBlockSig);
    )
}

/* ------------------------------------------------------------ */

/* ------------------------------------------------------------ */
class CTransaction
{
    int nVersion;
    unsigned int nTime;
    std::vector<CTxIn> vin;
    std::vector<CTxOut> vout;
    unsigned int nLockTime;

    IMPLEMENT_SERIALIZE
    (
        READWRITE(this->nVersion);
        nVersion = this->nVersion;
        READWRITE(nTime);
        READWRITE(vin);
        READWRITE(vout);
        READWRITE(nLockTime);
    )
/* ------------------------------------------------------------ */

/* ------------------------------------------------------------ */
class CTxIn
{
    COutPoint prevout;
    CScript scriptSig;
    unsigned int nSequence;

    IMPLEMENT_SERIALIZE
    (
        READWRITE(prevout);
        READWRITE(scriptSig);
        READWRITE(nSequence);
    )
}

/* ------------------------------------------------------------ */
class COutPoint

    uint256 hash;
    unsigned int n;

    IMPLEMENT_SERIALIZE( READWRITE(FLATDATA(*this))

/* ------------------------------------------------------------ */
class CTxOut
{
    int64 nValue;
    CScript scriptPubKey;

    IMPLEMENT_SERIALIZE
    (
        READWRITE(nValue);
        READWRITE(scriptPubKey);
    )
}


/* ------------------------------------------------------------ */
class CScript : public std::vector<unsigned char> {}

"""

import unittest
import os
import subprocess
import struct  # conversion between Python values and C structs as Python strings
try:
    import StringIO  # Reads and writes a string buffer
    import BytesIO  # Reads and writes a byte buffer
except ImportError:
    from io import StringIO
    from io import BytesIO
import os
import sys
import mmap  # mutable string
from binascii import unhexlify, hexlify
from datetime import datetime
import hashlib
import sqlite3
import dcrypt_hash
import json

datadir = "/home/gjh/minkiz/fabshop/SlimCoinWork/slimcoin-development/bootstrap.dat"

class BCDataStream(object):
    def __init__(self):
        self.input = None
        self.read_cursor = 0

    def clear(self):
        self.input = None
        self.read_cursor = 0

    def write(self, _bytes):  # Initialize with string of _bytes
        if self.input is None:
            self.input = bytearray(_bytes)
        else:
            self.input += bytearray(_bytes)

    def map_file(self, file, start):  # Initialize with bytes from file
        self.input = mmap.mmap(file.fileno(), 0, access=mmap.ACCESS_READ)
        self.read_cursor = start

    def seek_file(self, position):
        self.read_cursor = position

    def close_file(self):
        self.input.close()

    def read_string(self, encoding='ascii'):
        # Strings are encoded depending on length:
        # 0 to 252 :  1-byte-length followed by bytes (if any)
        # 253 to 65,535 : byte'253' 2-byte-length followed by bytes
        # 65,536 to 4,294,967,295 : byte '254' 4-byte-length followed by bytes
        # ... and the Bitcoin client is coded to understand:
        # greater than 4,294,967,295 : byte '255' 8-byte-length followed by bytes of string
        # ... but I don't think it actually handles any strings that big.
        if self.input is None:
            raise SerializationError("call write(bytes) before trying to deserialize")

        length = self.read_compact_size()

        return self.read_bytes(length).decode(encoding)

    def write_string(self, string, encoding='ascii'):
        string = to_bytes(string, encoding)
        # Length-encoded as with read-string
        self.write_compact_size(len(string))
        self.write(string)

    def read_bytes(self, length):
        try:
            result = self.input[self.read_cursor:self.read_cursor+length]
            self.read_cursor += length
            return result
        except IndexError:
            raise SerializationError("attempt to read past end of buffer")

        return ''

    def read_boolean(self): return self.read_bytes(1)[0] != chr(0)
    def read_int16(self): return self._read_num('<h')
    def read_uint16(self): return self._read_num('<H')
    def read_int32(self): return self._read_num('<i')
    def read_uint32(self): return self._read_num('<I')
    def read_int64(self): return self._read_num('<q')
    def read_uint64(self): return self._read_num('<Q')

    def write_boolean(self, val): return self.write(chr(1) if val else chr(0))
    def write_int16(self, val): return self._write_num('<h', val)
    def write_uint16(self, val): return self._write_num('<H', val)
    def write_int32(self, val): return self._write_num('<i', val)
    def write_uint32(self, val): return self._write_num('<I', val)
    def write_int64(self, val): return self._write_num('<q', val)
    def write_uint64(self, val): return self._write_num('<Q', val)

    def read_compact_size(self):
        try:
            size = self.input[self.read_cursor]
            self.read_cursor += 1
            if size == 253:
                size = self._read_num('<H')
            elif size == 254:
                size = self._read_num('<I')
            elif size == 255:
                size = self._read_num('<Q')
            return size
        except IndexError:
            raise SerializationError("attempt to read past end of buffer")

    def write_compact_size(self, size):
        if size < 0:
            raise SerializationError("attempt to write size < 0")
        elif size < 253:
            self.write(bytes([size]))
        elif size < 2**16:
            self.write(b'\xfd')
            self._write_num('<H', size)
        elif size < 2**32:
            self.write(b'\xfe')
            self._write_num('<I', size)
        elif size < 2**64:
            self.write(b'\xff')
            self._write_num('<Q', size)

    def _read_num(self, format):
        try:
            (i,) = struct.unpack_from(format, self.input, self.read_cursor)
            self.read_cursor += struct.calcsize(format)
        except Exception as e:
            raise SerializationError(e)
        return i

    def _write_num(self, format, num):
        s = struct.pack(format, num)
        self.write(s)


class Block:
    size = 0
    start = 0
    data = None
    blockhash = ""
    version = 0
    prev_header_hash = ""
    merkle_root_hash = ""
    timestamp = None
    timestampint = 0
    nbits = ""
    nonce = ""
    nonceint = 0
    # Proof of burn stuff
    fproofofburn = False
    hashburnblock = ""
    burnhash = ""
    burnblockheight = ""
    burnctx = ""
    burnctxout = ""
    neffectiveburncoins = ""
    nburnbits = ""

    # Transactions 
    tx_count = 0

    def __init__(self, ds):
        magic = "6e8b92a5"
        self.magic = ds.read_bytes(4).hex()
        # Acid test
        if self.magic != magic:
            print("FAILED {}\n{}".format(magic, ds.read_bytes(512).hex()))
            return {}
        # Good to go, find out the length of this block
        self.size = int(struct.pack('>l', *struct.unpack('<l', ds.read_bytes(4))).hex(), 16)
        # Save point
        self.start = ds.read_cursor
        # Read the raw data for the block
        self.data = ds.read_bytes(self.size)
        # Store the hex representation for later output
        # Hash it with dcrypt to get the block hash and note the value

        self.blockhash = dcrypt_hash.getPoWHash(self.data)
        # **REDO FROM START**
        ds.seek_file(self.start)
        # Parse the data
        # Standard block rubric

        self.version = struct.pack('>l', *struct.unpack('<l', ds.read_bytes(4))).hex()
        self.prev_header_hash = ds.read_bytes(32)[::-1].hex()
        self.merkle_root_hash = ds.read_bytes(32)[::-1].hex()
        timestampbits = ds.read_bytes(4)[::-1].hex()
        self.timestamp = int(timestampbits, 16)
        self.nbits = ds.read_bytes(4)[::-1].hex()
        self.nonce = ds.read_bytes(4)[::-1].hex()
        self.nonceint = int(self.nonce, 16)

        # Proof of burn stuff
        self.fproofofburn = ds.read_boolean()
        self.hashburnblock = ds.read_bytes(32)[::-1].hex()
        self.burnhash = ds.read_bytes(32)[::-1].hex()
        self.burnblockheight = ds.read_int32()
        self.burnctx = ds.read_int32()
        self.burnctxout = ds.read_int32()
        self.neffectiveburncoins = ds.read_int64()
        self.nburnbits = ds.read_uint32()

        # Transactions 
        self.tx_count = int(ds.read_compact_size())
        self.txs = [
            dict(
                tx_version = ds.read_bytes(4)[::-1].hex(),
                tx_ntime = int(ds.read_bytes(4)[::-1].hex(), 16),
                # tx_input = int(ds.read_compact_size()),
                vins=[
                    self.read_txin(ds)
                    for vin in range(0, int(ds.read_compact_size()))],
                vouts = [
                    self.read_txout(ds)
                    for vout in range(0, int(ds.read_compact_size()))],
                tnburnbits=ds.read_bytes(4)[::-1].hex())
            for txnum in range(1, self.tx_count+1)]

        # Block signature
        self.block_sig_length = ds.read_bytes(1)[::-1].hex()
        block_sig_lengthint = int(self.block_sig_length, 16)
        self.block_sig = ds.read_bytes(block_sig_lengthint).hex()

    def blockhex(self):
        return self.data.hex()

    def read_txin(self, ds):
        tx_prev_output_hash = ds.read_bytes(32)[::-1].hex()
        tx_prev_output_sequence = ds.read_bytes(4)[::-1].hex()
        coinbase_tx_length = ds.read_bytes(1)[::-1].hex()
        coinbase_tx_lengthint = int(coinbase_tx_length, 16)
        coinbase_tx = ds.read_bytes(int((coinbase_tx_length), 16)).hex()
        sequence = ds.read_bytes(4)[::-1].hex()
        return dict(
            tx_prev_output_hash=tx_prev_output_hash,
            tx_prev_output_sequence=tx_prev_output_sequence,
            coinbase_tx=coinbase_tx,
            sequence=sequence,
            )

    def read_txout(self, ds):
        btc_amt = ds.read_bytes(8)[::-1].hex()
        btc_amtint = int(btc_amt, 16)
        pk_script_len = ds.read_compact_size()
        pk_script_lenint = int(pk_script_len)
        pk_script = ds.read_bytes(int(pk_script_lenint)).hex()
        return dict(
            btc_amt=btc_amtint,
            pk_script=pk_script
        )

    def render_as_dict(self):
        return dict(
            blockhex=self.data.hex(),
            blockhash=self.blockhash[::-1].hex(),
            version=self.version,
            prev_header_hash=self.prev_header_hash,
            merkle_root_hash=self.merkle_root_hash,
            timestamp=self.timestamp,
            nbits=self.nbits,
            nonce=self.nonceint,
            fproofofburn=self.fproofofburn,
            hashburnblock=self.hashburnblock,
            burnhash=self.burnhash,
            burnblockheight=self.burnblockheight,
            burnctx=self.burnctx,
            burnctxout=self.burnctxout,
            neffectiveburncoins=self.neffectiveburncoins,
            nburnbits=self.nburnbits,
            tx_count=self.tx_count,
            txs=self.txs,
            block_sig=self.block_sig
        )

class TestMyView(unittest.TestCase):
    def setUp(self):
        self.conn = sqlite3.connect('slmchain.db')

    def tearDown(self):
        self.conn.close()

    def read_block_at_height(self, blockheight):
        # Get offset from lookuptable of sequential offsets
        offset = int(subprocess.getoutput('head -%s %s | tail -1' % (blockheight, os.getcwd() + '/slmoffsets.txt')))
        # Position the read cursor at the offset and return the result of
        # calling read_block at that point in bootstrap.dat
        ds = BCDataStream()
        with open(datadir, "rb") as file:
            ds.map_file(file, 0)
            ds.seek_file(offset)
            data = self.read_block(ds, blockheight)
        file.close()
        return data

    def read_block(self, ds, blockheight=0):
        return Block(ds)

    def read_block_steps(self, ds, blockheight=0):
        # Original approach, superseded by Block class approach
        # Read file
        # https://bitcoin.org/en/developer-reference#block-headers
        # https://en.bitcoin.it/wiki/Protocol_specification#block
        magic = ds.read_bytes(4).hex()
        # Acid test
        if magic != "6e8b92a5":
            print("FAILED {}\n{}".format(magic, ds.read_bytes(512).hex()))
            return {}
        # Good to go, find out the length of this block
        block_size = int(struct.pack('>l', *struct.unpack('<l', ds.read_bytes(4))).hex(), 16)
        # Save point
        blockstart = ds.read_cursor
        # Read the raw data for the block
        blockhex_bin = ds.read_bytes(block_size)
        # Store the hex representation for later output
        blockhex = blockhex_bin.hex()
        # Hash it with dcrypt to get the block hash and note the value
        bhash = dcrypt_hash.getPoWHash(blockhex_bin)
        blockhash = bhash[::-1].hex()
        # **REDO FROM START**
        ds.seek_file(blockstart)
        # Parse the data
        # Standard block rubric
        version = struct.pack('>l', *struct.unpack('<l', ds.read_bytes(4))).hex()
        prev_header_hash = ds.read_bytes(32)[::-1].hex()
        merkle_root_hash = ds.read_bytes(32)[::-1].hex()
        timestamp = ds.read_bytes(4)[::-1].hex()
        timestampint = int(timestamp, 16)
        formatted_timestamp = datetime.fromtimestamp(int(timestamp, 16)).isoformat()
        nbits = ds.read_bytes(4)[::-1].hex()
        nonce = ds.read_bytes(4)[::-1].hex()
        nonceint = int(nonce, 16)

        # Proof of burn stuff
        fproofofburn = ds.read_boolean()
        hashburnblock = ds.read_bytes(32)[::-1].hex()
        burnhash = ds.read_bytes(32)[::-1].hex()
        burnblockheight = ds.read_int32()
        burnctx = ds.read_int32()
        burnctxout = ds.read_int32()
        neffectiveburncoins = ds.read_int64()
        nburnbits = ds.read_uint32()

        # Transactions 
        txcount = ds.read_compact_size()
        txcountint = int(txcount)

        txstr = ""
        # Iterate over txs
        for txnum in range(1, txcountint+1):
            tx_version = ds.read_bytes(4)[::-1].hex()
            tx_ntime = ds.read_bytes(4)[::-1].hex()
            tx_ntimeint = int(tx_ntime, 16)
            formatted_tx_ntime = datetime.fromtimestamp(int(tx_ntime, 16)).isoformat()
            tx_input = ds.read_compact_size()
            input = int(tx_input)
            vinstr = ""
            # Iterate over tx inputs
            for txin in range(0, input):
                tx_prev_output_hash = ds.read_bytes(32)[::-1].hex()
                tx_prev_output_sequence = ds.read_bytes(4)[::-1].hex()
                coinbase_tx_length = ds.read_bytes(1)[::-1].hex()
                coinbase_tx_lengthint = int(coinbase_tx_length, 16)
                coinbase_tx = ds.read_bytes(int((coinbase_tx_length), 16)).hex()
                sequence = ds.read_bytes(4)[::-1].hex()
                vinstr += vin_template.format(**locals())
            no_of_outputs = ds.read_compact_size()
            voutstr = ""
            # Iterate over tx outputs
            for j in range(0, int(no_of_outputs)):
                btc_amt = ds.read_bytes(8)[::-1].hex()
                btc_amtint = int(btc_amt, 16)
                pk_script_len = ds.read_compact_size()
                pk_script_lenint = int(pk_script_len)
                pk_script = ds.read_bytes(int(pk_script_lenint)).hex()
                voutstr += vout_template.format(**locals())
            # Allegedly
            tnburnbits = ds.read_bytes(4)[::-1].hex()
            # Append to output string
            txstr += tx_template.format(**locals())
        # Block signature length and data
        block_sig_length = ds.read_bytes(1)[::-1].hex()
        block_sig_lengthint = int(block_sig_length, 16)
        block_sig = ds.read_bytes(int(block_sig_length, 16)).hex()
        # print(block_template.format(**locals()),txstr,cbs_template.format(**locals()))
        # return the serlialised data
        return '\n'.join([block_template.format(**locals()), txstr, cbs_template.format(**locals())])

    def read_genesis_block(self, ds):
        # Read file
        # https://bitcoin.org/en/developer-reference#block-headers
        # https://en.bitcoin.it/wiki/Protocol_specification#block
        blockheight = 0
        magic = ds.read_bytes(4).hex()
        if magic != "6e8b92a5":
            print("FAILED #{}\n{}\n{}".format(blockheight, magic, ds.read_bytes(512).hex()))
            exit(0)
        block_size = int(struct.pack('>l', *struct.unpack('<l', ds.read_bytes(4))).hex(), 16)
        version = struct.pack('>l', *struct.unpack('<l', ds.read_bytes(4))).hex()
        prev_header_hash = ds.read_bytes(32)[::-1].hex()
        merkle_root_hash = ds.read_bytes(32)[::-1].hex()
        timestamp = ds.read_bytes(4)[::-1].hex()
        timestampint = int(timestamp, 16)
        formatted_timestamp = datetime.fromtimestamp(int(timestamp, 16)).isoformat()
        nbits = ds.read_bytes(4)[::-1].hex()
        nonce = ds.read_bytes(4)[::-1].hex()
        nonceint = int(nonce, 16)

        fproofofburn = ds.read_boolean()
        hashburnblock = ds.read_bytes(32)[::-1].hex()
        burnhash = ds.read_bytes(32)[::-1].hex()
        burnblockheight = ds.read_int32()
        burnctx = ds.read_int32()
        burnctxout = ds.read_int32()
        neffectiveburncoins = ds.read_int64()
        nburnbits = ds.read_uint32()

        txcount = ds.read_compact_size()
        txcountint = int(txcount)
        txstr = ""
        for txnum in range(1, txcountint+1):
            tx_version = ds.read_bytes(4)[::-1].hex()
            tx_ntime = ds.read_bytes(4)[::-1].hex()
            tx_ntimeint = int(tx_ntime, 16)
            formatted_tx_ntime = datetime.fromtimestamp(int(tx_ntime, 16)).isoformat()
            tx_input = ds.read_compact_size()
            input = int(tx_input)
            vinstr = ""
            for txin in range(0, input):
                tx_prev_output_hash = ds.read_bytes(32)[::-1].hex()
                tx_prev_output_sequence = ds.read_bytes(4)[::-1].hex()
                coinbase_tx_length = ds.read_bytes(1)[::-1].hex()
                coinbase_tx_lengthint = int(coinbase_tx_length, 16)
                coinbase_tx = ds.read_bytes(int((coinbase_tx_length), 16)).hex()
                sequence = ds.read_bytes(4)[::-1].hex()
                vinstr += vin_template.format(**locals())
            no_of_outputs = ds.read_compact_size()
            voutstr = ""
            for j in range(0, int(no_of_outputs)):
                btc_amt = ds.read_bytes(8)[::-1].hex()
                btc_amtint = int(btc_amt, 16)
                pk_script_len = ds.read_compact_size()
                pk_script_lenint = int(pk_script_len)
                pk_script = ds.read_bytes(int(pk_script_lenint)).hex()
                voutstr += vout_template.format(**locals())
            tnburnbits = ds.read_bytes(4).hex()
            txstr += tx_template.format(**locals())
        block_sig_length = ds.read_bytes(1)[::-1].hex()
        block_sig_lengthint = int(block_sig_length, 16)
        block_sig = ds.read_bytes(int(block_sig_length, 16)).hex()
        return '\n'.join([block_template.format(**locals()),txstr,cbs_template.format(**locals())])
    # @unittest.skip("Suspended, skipping")
    def test_check_parse_chain(self):
        ds = BCDataStream()
        with open(datadir, "rb") as file:
            ds.map_file(file, 0)
            self.read_genesis_block(ds)
            cnt = 1
            while True:
                try:
                    _ = self.read_block(ds, blockheight=cnt)
                except:
                    break
                cnt += 1
        ds.close_file()
        print("Done")

    @unittest.skip("Suspended, skipping")
    def test_create_sqlite_tables(self):
        dbfile = os.getcwd() + "/slmchain.db"
        db = sqlite3.connect(dbfile)
        c = db.cursor()
        c.execute('''create table block(id integer primary key, blockhash text, blockhex text, version integer, prev_header_hash text, merkle_root_hash text, timestamp timestamp, nbits text, nonce integer, fproofofburn boolean, hashburnblock text, burnhash text, burnblockheight text, burnctx text, burnctxout text, neffectiveburncoins text, nburnbits integer, tx_count integer, block_sig text, offset integer);''')
        c.execute('''create table "main"."transaction"(id integer, blockid integer primary key default NULL, hash text, tx_version integer, tx_ntime integer, no_of_outputs integer, nburnbits text, foreign key (blockid) references block(id));''')
        c.execute('''create table vout(txid integer primary key default NULL, btc_amt integer, pk_script_len integer, pk_script text, foreign key (txid) references "transaction"(id));''')
        c.execute('''create table vin(txid integer primary key default NULL, prev_output_hash text, prev_output_sequence text, coinbase_tx text, sequence text, foreign key(txid) references "transaction"(id));''')

    @unittest.skip("Suspended, skipping")
    def test_recreate_sqlite_tables(self):
        dbfile = os.getcwd() + "/slmchain.db"
        db = sqlite3.connect(dbfile)
        c = db.cursor()
        c.execute('''drop table block;''')
        c.execute('''drop table "main"."transaction";''')
        c.execute('''drop table vout;''')
        c.execute('''drop table vin;''')
        c.execute('''create table block(id integer primary key, blockhash text, blockhex text, version integer, prev_header_hash text, merkle_root_hash text, timestamp timestamp, nbits text, nonce integer, fproofofburn boolean, hashburnblock text, burnhash text, burnblockheight text, burnctx text, burnctxout text, neffectiveburncoins text, nburnbits integer, tx_count integer, block_sig text, offset integer);''')
        c.execute('''create table "main"."transaction"(id integer, blockid integer primary key default NULL, hash text, tx_version integer, tx_ntime integer, no_of_outputs integer, nburnbits text, foreign key (blockid) references block(id));''')
        c.execute('''create table vout(id integer, txid integer primary key default NULL, btc_amt integer, pk_script text, foreign key (txid) references "transaction"(id));''')
        c.execute('''create table vin(id integer, txid integer primary key default NULL, prev_output_hash text, prev_output_sequence text, coinbase_tx text, sequence text, foreign key(txid) references "transaction"(id));''')

    @unittest.skip("Suspended, skipping")
    def test_delete_sqlite_tables(self):
        dbfile = os.getcwd() + "/slmchain.db"
        db = sqlite3.connect(dbfile)
        c = db.cursor()
        c.execute('''drop table block;''')
        c.execute('''drop table "main"."transaction";''')
        c.execute('''drop table vout;''')
        c.execute('''drop table vib;''')

    @unittest.skip("Suspended, skipping")
    def test_create_offsetlookup(self):
        """
        Write a text file of sequential offsets to blocks in the bootstrap
        file for use as a lookup table (see test_read_block_at_1100000,
        below, for usage example). Cheap and cheerful.
        """
        ds = BCDataStream()
        with open(datadir, "rb") as file, open('slm-offsets', 'w') as ofile:
            ds.map_file(file, 0)
            ofile.write("{}\n".format(ds.read_cursor))
            self.read_genesis_block(ds)
            cnt = 1
            while True:
                try:
                    # Only interested in offset so ignore the output
                    _ = self.read_block(ds)
                    ofile.write("{}\n".format(ds.read_cursor))
                except:
                    break
                cnt += 1
        ds.close_file()
        print("Done")

    @unittest.skip("Passed, skipping")
    def test_read_block_at_1100000(self):
        blockheight = 1100000
        data = self.read_block_at_height(blockheight)
        print(data)

    @unittest.skip("Passed, skipping")
    def test_render_blockchain_as_sqlite3(self):
        db = sqlite3.connect(os.getcwd() + "/slmchain.db")
        c = db.cursor()
        ds = BCDataStream()
        with open(datadir, "rb") as file, open(os.getcwd() + '/slmoffsets.txt', 'r') as foff:
            ds.map_file(file, 0)
            blockheight = 0
            txidcnt = 0
            vinidcnt = 0
            voutidcnt = 0
            for offset in foff.readlines()[:1]:
                print("Reading block {} at offset {}".format(blockheight, offset))
                ds.seek_file(int(offset))
                block = self.read_block(ds, blockheight=blockheight)
                blockdict = block.render_as_dict()
                # Do block data first
                txs = blockdict.get('txs')
                del blockdict['txs']
                blockdict["id"] = blockheight
                blockdict["offset"] = int(offset)
                print(json.dumps(blockdict, sort_keys=True, indent=2, separators=(',', ': ')))
                query = "insert into block (id, blockhash, blockhex, burnblockheight, burnctx, burnctxout, burnhash, fproofofburn, hashburnblock, merkle_root_hash, nbits, nburnbits, neffectiveburncoins, nonce, prev_header_hash, timestamp, tx_count, version, block_sig, offset) values (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"
                columns = [
                    "id", "blockhash", "blockhex", "burnblockheight", "burnctx", "burnctxout",
                    "burnhash", "fproofofburn", "hashburnblock", "merkle_root_hash", "nbits",
                    "nburnbits", "neffectiveburncoins", "nonce", "prev_header_hash",
                    "timestamp", "tx_count", "version", "block_sig", "offset"
                ]
                keys = tuple(blockdict[c] for c in columns)
                c.execute(query, keys)

                # Now handle the transactions

                for tx in txs:
                    tx["no_of_outputs"] = len(tx.get('vouts'))
                    tx["id"] = txidcnt
                    query = '''insert into "transaction" (id, tx_version, tx_ntime, no_of_outputs, nburnbits) values (?, ?, ?, ?, ?)'''
                    columns = [
                        "id", "tx_version", "tx_ntime", "no_of_outputs", "tnburnbits"
                    ]
                    keys = tuple(tx[c] for c in columns)
                    c.execute(query, keys)

                    # Inputs
                    for vin in tx['vins']:
                        vin["id"] = vinidcnt
                        vin["txid"] = txidcnt
                        query = '''insert into vin (id, txid, prev_output_hash, prev_output_sequence, coinbase_tx, sequence) values (?, ?, ?, ?, ?, ?)'''
                        columns = [
                            "id", "txid", "tx_prev_output_hash", "tx_prev_output_sequence",
                            "coinbase_tx", "sequence"
                        ]
                        keys = tuple(vin[c] for c in columns)
                        c.execute(query, keys)
                        vinidcnt += 1

                    # Outputs
                    for vout in tx['vouts']:
                        vout["id"] = voutidcnt
                        vout["txid"] = txidcnt
                        query = '''insert into vout (id, txid, btc_amt, pk_script) values (?, ?, ?, ?)'''
                        columns = [
                            "id", "txid", "btc_amt", "pk_script"
                        ]
                        keys = tuple(vout[c] for c in columns)
                        c.execute(query, keys)
                        voutidcnt += 1

                    txidcnt += 1
                blockheight += 1
        db.commit()
        c.close()

    @unittest.skip("Passed, skipping")
    def test_read_db_block(self):
        t = ('1144355',)
        c = self.conn.cursor()
        c.execute('SELECT offset FROM block WHERE id=?', t)
        offset = c.fetchone()
        print(offset[0])
        ds = BCDataStream()
        with open(datadir, "rb") as file:
            ds.map_file(file, 0)
            ds.seek_file(offset[0])
            data = self.read_block_n(ds, blockheight=1144355)
            file.close()

    @unittest.skip("Passed, skipping")
    def test_duffblock(self):
        # hexstr = "6e8b92a501000000ea1bd6186fea31332b9173c6f4c533b4a22b8da9850b044cb74b5abe660700008b7c2f44babc66fa32f09b688fa0640d066a856936d987a466a8687c2d57189f73518653ffff071efe2101000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000ffffffffffffffffffffffff00000000000000000000000001010000009d508653010000000000000000000000000000000000000000000000000000000000000000ffffffff0e049e5086530101062f503253482fffffffff01c07a8102000000002321038a52f85595a8d8e7c1d8c256baeee2c9ea7ad0bf7fe534575be4eb47cdbf18f6ac00000000473045022100e86e6a211795858134d09d2febfb82ec5a4a2b9dce36ae92a67053ec80d634c902206c233b82d033f73c0452128792aafe79ec094676f358e6ad7631b51d60ec1995"
        # print(', '.join(["0x{}".format(hexstr[i:i+2]) for i in range(0, len(hexstr), 2)]))
        data = bytes([
            0x6e, 0x8b, 0x92, 0xa5, 0x63, 0x01, 0x01, 0x00, 0x00, 0x00, 0xea, 0x1b, 0xd6,
            0x18, 0x6f, 0xea, 0x31, 0x33, 0x2b, 0x91, 0x73, 0xc6, 0xf4, 0xc5,
            0x33, 0xb4, 0xa2, 0x2b, 0x8d, 0xa9, 0x85, 0x0b, 0x04, 0x4c, 0xb7,
            0x4b, 0x5a, 0xbe, 0x66, 0x07, 0x00, 0x00, 0x8b, 0x7c, 0x2f, 0x44,
            0xba, 0xbc, 0x66, 0xfa, 0x32, 0xf0, 0x9b, 0x68, 0x8f, 0xa0, 0x64,
            0x0d, 0x06, 0x6a, 0x85, 0x69, 0x36, 0xd9, 0x87, 0xa4, 0x66, 0xa8,
            0x68, 0x7c, 0x2d, 0x57, 0x18, 0x9f, 0x73, 0x51, 0x86, 0x53, 0xff,
            0xff, 0x07, 0x1e, 0xfe, 0x21, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff,
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00,
            0x00, 0x00, 0x9d, 0x50, 0x86, 0x53, 0x01, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x0e,
            0x04, 0x9e, 0x50, 0x86, 0x53, 0x01, 0x01, 0x06, 0x2f, 0x50, 0x32,
            0x53, 0x48, 0x2f, 0xff, 0xff, 0xff, 0xff, 0x01, 0xc0, 0x7a, 0x81,
            0x02, 0x00, 0x00, 0x00, 0x00, 0x23, 0x21, 0x03, 0x8a, 0x52, 0xf8,
            0x55, 0x95, 0xa8, 0xd8, 0xe7, 0xc1, 0xd8, 0xc2, 0x56, 0xba, 0xee,
            0xe2, 0xc9, 0xea, 0x7a, 0xd0, 0xbf, 0x7f, 0xe5, 0x34, 0x57, 0x5b,
            0xe4, 0xeb, 0x47, 0xcd, 0xbf, 0x18, 0xf6, 0xac, 0x00, 0x00, 0x00,
            0x00, 0x47, 0x30, 0x45, 0x02, 0x21, 0x00, 0xe8, 0x6e, 0x6a, 0x21,
            0x17, 0x95, 0x85, 0x81, 0x34, 0xd0, 0x9d, 0x2f, 0xeb, 0xfb, 0x82,
            0xec, 0x5a, 0x4a, 0x2b, 0x9d, 0xce, 0x36, 0xae, 0x92, 0xa6, 0x70,
            0x53, 0xec, 0x80, 0xd6, 0x34, 0xc9, 0x02, 0x20, 0x6c, 0x23, 0x3b,
            0x82, 0xd0, 0x33, 0xf7, 0x3c, 0x04, 0x52, 0x12, 0x87, 0x92, 0xaa,
            0xfe, 0x79, 0xec, 0x09, 0x46, 0x76, 0xf3, 0x58, 0xe6, 0xad, 0x76, 
            0x31, 0xb5, 0x1d, 0x60, 0xec, 0x19, 0x95])
        ds = BCDataStream()
        ds.input = data
        res = self.read_block(ds, blockheight=0).render_as_dict()
        print(res)

    @unittest.skip("Suspended, skipping")
    def test_render_as_jsonld(self):
        ds = BCDataStream()
        with open(datadir, "rb") as file, open(os.getcwd() + '/slmoffsets.txt', 'r') as foff, open(os.getcwd() + '/slmchain.json', 'w') as fjson:
            fjson.write('''"blocks": [\n''')
            ds.map_file(file, 0)
            blockheight = 0
            txidcnt = 0
            vinidcnt = 0
            voutidcnt = 0
            for offset in foff.readlines():
                ds.seek_file(int(offset))
                block = self.read_block(ds, blockheight=blockheight)
                blockdict = block.render_as_dict()
                # Do block data first
                transactions = blockdict['txs']
                del blockdict['txs']
                blockdict["@context"] = "https://minkiz.co/contexts/slimcoin.jsonld"
                blockdict["@type"] = "Block"
                blockdict["@id"] = "{}".format(blockdict['blockhash'])
                blockdict["offset"] = "{}".format(int(offset))
                blockdict["height"] = "{}".format(blockheight)
                blockdict["transactions"] = [
                    dict(transaction=[
                        {
                            "@type": "Transaction",
                            "@id": "{}-{}".format(blockdict['blockhash'], txidcnt + txidinc),
                            # "hash": "{}".format(tx.get['hash']),
                            "version": "{}".format(tx.get('tx_version')),
                            "ntime": "{}".format(tx.get('tx_ntime')),
                            "no_of_outputs": "{}".format(len(tx.get('vouts'))),
                            "tnburnbits": "{}".format(tx.get('tnburnbits')),
                            # Inputs
                            "vins": [
                                {
                                "@type": "TransactionInput",
                                "@id": "{}-{}-{}".format(blockdict['blockhash'], txidcnt + txidinc, vinidcnt + vinidinc),
                                "prev_output_hash": "{}".format(vin.get('tx_prev_output_hash')),
                                "prev_output_sequence": "{}".format(vin.get('tx_prev_output_sequence')),
                                "coinbase_tx": "{}".format(vin.get('coinbase_tx')),
                                "sequence":"{}".format(vin.get('sequence'))
                                } for vinidinc, vin in enumerate(tx.get('vins'))
                            ],
                            # Outputs
                            "vouts": [
                                {
                                "@type": "TransactionOutput",
                                "@id": "{}-{}-{}".format(blockdict['blockhash'], txidcnt + txidinc, voutidcnt + voutidinc),
                                "btc_amt": "{}".format(vout.get('btc_amt', 0)),
                                "pk_script": "{}".format(vout.get('pk_script', ''))
                                } for voutidinc, vout in enumerate(tx.get('vouts'))
                            ]
                    } for txidinc, tx in enumerate(transactions)
                    ])
                ]
                txidcnt += len(transactions)
                vinidcnt += sum([len(tx['vins']) for tx in transactions])
                voutidcnt += sum([len(tx['vouts']) for tx in transactions])
                fjson.write(json.dumps(blockdict, sort_keys=True, indent=2, separators=(',', ': ')) + ',\n')
                blockheight += 1
            fjson.write(''']\n''')


if __name__ == "__main__":
    unittest.main()
