#! /usr/bin/env python
"""
/* ------------------------------------------------------------ */
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
except ImportError:
    from io import StringIO
import os
import sys
import mmap  # mutable string
from binascii import unhexlify, hexlify
from datetime import datetime
import hashlib
import sqlite3
import dcrypt_hash

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


def import_blkdat():
    pass

# datadir = "/Users/{}/Library/Application Support/SLIMCoin/blk0001.dat".format(os.environ['USER']) if sys.platform == 'darwin' else "/home/{}/.slimcoin/blk0001.dat".format(os.environ['USER'])
datadir = "/home/gjh/minkiz/fabshop/SlimCoinWork/slimcoin-development/bootstrap.dat"

block_template = """
===================== Details of Block {blockheight}: ======================
hex: {blockhex}
magic: {magic} (6e, 8b, 92, a5)
block_size: {block_size}
blockhash: {blockhash}
version: {version}
prevhash: {prev_header_hash}
merkle_root: {merkle_root_hash}
timestamp: {timestampint} ({formatted_timestamp})
nbits: {nbits}
nonce: {nonceint}
fproofofburn: {fproofofburn}
hashburnblock: {hashburnblock}
burnhash: {burnhash}
burnblockheight: {burnblockheight}
burnctx: {burnctx}
burnctxout: {burnctxout}
neffectiveburncoins: {neffectiveburncoins}
nburnbits: {nburnbits}
txcount: {txcountint}
"""

tx_template = """
------------------ Details of Transaction {txnum}: ------------------
tx_version: {tx_version}
tx_ntime: {tx_ntimeint} ({tx_ntime}) {formatted_tx_ntime}
input: {input}
{vinstr}
no_of_outputs#: {no_of_outputs}
{voutstr}
nburnbits: {tnburnbits}
"""

vin_template = """
tx_prev_output_hash: {tx_prev_output_hash}
tx_prev_output_sequence: {tx_prev_output_sequence}
coinbase_tx_length: {coinbase_tx_lengthint}
coinbase_tx: {coinbase_tx}
sequence: {sequence}
"""

vout_template = """
btc_amt: {btc_amtint}
pk_script_len: {pk_script_lenint}
pk_script: {pk_script}
"""

cbs_template = """
block_sig_length: {block_sig_lengthint}
block_sig: {block_sig}
"""

# duffblock = 0x01000000ea1bd6186fea31332b9173c6f4c533b4a22b8da9850b044cb74b5abe6607000022cc36cea95004fdecf21c0e912138475a463ebfdd1abafa37e2ed317d96e1e02823705bffff071e90575e00000000000000000000000000000000000000000000000000000000000000000000ffffffffffffffffffffffff0000000000000000000000000101000000d322705b010000000000000000000000000000000000000000000000000000000000000000ffffffff0f042823705b02b915062f503253482fffffffff01c07a810200000000232103d2ee359d7f7ae07de052a4299bc9761d6426283bf0ce8d7f7eb378bd6cd0f40dac00000000463044022017f087462acae454826f807701a698d70cabb8d79c28299a4f6ddebd362224190220488117183bbd51fd38b321610a0f6ea28897bdf8f840273b31dfbcd60513ca0f

class TestMyView(unittest.TestCase):
    def setUp(self):
        self.conn = sqlite3.connect('slmchain.db')

    def tearDown(self):
        self.conn.close()

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

    def read_block(self, ds, blockheight=0):
        # Read file
        # https://bitcoin.org/en/developer-reference#block-headers
        # https://en.bitcoin.it/wiki/Protocol_specification#block
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
        # if blockheight > 1144350:
        #     print(block_template.format(**locals()),txstr,cbs_template.format(**locals()))

    def read_datablock(self, offset):
        # Read file
        # https://bitcoin.org/en/developer-reference#block-headers
        # https://en.bitcoin.it/wiki/Protocol_specification#block
        ds = BCDataStream()
        blockheight = 9999999
        with open(datadir, "rb") as file:
            ds.map_file(file, 0)
            ds.seek_file(offset)
            magic = ds.read_bytes(4).hex()
            if magic != "6e8b92a5":
                print("FAILED {}\n{}".format(magic, ds.read_bytes(512).hex()))
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
                tnburnbits = ds.read_bytes(4)[::-1].hex()
                txstr += tx_template.format(**locals())
            block_sig_length = ds.read_bytes(1)[::-1].hex()
            block_sig_lengthint = int(block_sig_length, 16)
            block_sig = ds.read_bytes(int(block_sig_length, 16)).hex()
            print(block_template.format(**locals()),txstr,cbs_template.format(**locals()))
        file.close()

    def read_blockdata(self, ds, blockheight=1):
        # Read file
        # https://bitcoin.org/en/developer-reference#block-headers
        # https://en.bitcoin.it/wiki/Protocol_specification#block
        magic = ds.read_bytes(4).hex()
        if magic != "6e8b92a5":
            print("FAILED {}\n{}".format(magic, ds.read_bytes(512).hex()))
            return {}
        block_size = int(struct.pack('>l', *struct.unpack('<l', ds.read_bytes(4))).hex(), 16)
        blockstart = ds.read_cursor
        blockhex_bin = ds.read_bytes(block_size)
        blockhex = blockhex_bin.hex()
        bhash = dcrypt_hash.getPoWHash(blockhex_bin)
        blockhash = bhash[::-1].hex()
        ds.seek_file(blockstart)
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
            tnburnbits = ds.read_bytes(4)[::-1].hex()
            txstr += tx_template.format(**locals())
        block_sig_length = ds.read_bytes(1)[::-1].hex()
        block_sig_lengthint = int(block_sig_length, 16)
        block_sig = ds.read_bytes(int(block_sig_length, 16)).hex()
        # print(block_template.format(**locals()),txstr,cbs_template.format(**locals()))
        return '\n'.join([block_template.format(**locals()), txstr, cbs_template.format(**locals())])

    def read_block_n(self, ds, blockheight=1):
        cnt = 0
        # Read file
        # https://bitcoin.org/en/developer-reference#block-headers
        # https://en.bitcoin.it/wiki/Protocol_specification#block
        while cnt < blockheight:
            magic = ds.read_bytes(4).hex()
            if magic != "6e8b92a5":
                print("FAILED #{}\n{}\n{}".format(blockheight, magic, ds.read_bytes(512).hex()))
                exit(0)
            block_size = int(struct.pack('>l', *struct.unpack('<l', ds.read_bytes(4))).hex(), 16)
            blockdata = ds.read_bytes(block_size)
            cnt += 1
        # return self.read_blockdata(ds)
        return self.read_blockdata(ds, blockheight=blockheight)

    @unittest.skip("Suspended, skipping")
    def test_read_chain(self):
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
    def test_create_offsetlookup(self):
        ds = BCDataStream()
        with open(datadir, "rb") as file, open('slm-offsets', 'w') as ofile:
            ds.map_file(file, 0)
            ofile.write("{}\n".format(ds.read_cursor))
            self.read_genesis_block(ds)
            cnt = 1
            while True:
                try:
                    _ = self.read_block(ds, blockheight=cnt)
                    ofile.write("{}\n".format(ds.read_cursor))
                except:
                    break
                cnt += 1
        ds.close_file()
        print("Done")

    @unittest.skip("Passed, skipping")
    def test_read_csv_chain(self):
        # with open(os.getcwd + 'slmchain.csv', 'r', 1) as fp:
        #     for row in csv.reader(iter(fp.readline())):
        #         print("Read: ({}) {!r}".format(time.time(), row))
        res = subprocess.getoutput('head -%s %s | tail -1' % (1341097, os.getcwd() + '/slmchain.csv'))
        print(res)

    # @unittest.skip("Passed, skipping")
    def test_read_block_at_1100000(self):
        bheight = 1100000
        offset = int(subprocess.getoutput('head -%s %s | tail -1' % (bheight, os.getcwd() + '/slmoffsets.txt')))
        ds = BCDataStream()
        with open(datadir, "rb") as file:
            ds.map_file(file, 0)
            ds.seek_file(offset)
            data = self.read_blockdata(ds, bheight)
            print(data)
            file.close()

    @unittest.skip("Passed, skipping")
    def test_read_db_block(self):
        t = ('1144355',)
        c = self.conn.cursor()
        c.execute('SELECT offset FROM slmchain WHERE id=?', t)
        offset = c.fetchone()
        print(offset[0])
        self.read_datablock(int(offset[0]), 1144355)

    @unittest.skip("Passed, skipping")
    def test_get_db_block_hash(self):
        t = ('1144230',)
        c = self.conn.cursor()
        c.execute('SELECT offset FROM slmchain WHERE id=?', t)
        offset = int(c.fetchone()[0])
        print(offset)
        ds = BCDataStream()
        with open(datadir, "rb") as file:
            ds.map_file(file, 0)
            ds.seek_file(offset)
            # print(ds.read_bytes(512).hex())
            self.read_data_block(ds, offset, 1144229)
            file.close()

    @unittest.skip("Passed, skipping")
    def test_tally(self):
        ds = BCDataStream()
        wantblock = 1800000
        with open(datadir, "rb") as file:
            blockheight = 0
            ds.map_file(file, blockheight)
            while True:
                print("block {}, offset {}".format(blockheight, ds.read_cursor))
                d = self.read_blockdata(ds, blockheight=blockheight)
                blockheight += 1

    @unittest.skip("Passed, skipping")
    def test_rbl1(self):
        ds = BCDataStream()
        with open(datadir, "rb") as file:
            ds.map_file(file, 0)
            data = self.read_block_n(ds, blockheight=2)
            file.close()
        print(data)

    @unittest.skip("Passed, skipping")
    def test_rbl2(self):
        # import io
        # data = io.BytesIO(bytes(0x01000000ea1bd6186fea31332b9173c6f4c533b4a22b8da9850b044cb74b5abe660700008b7c2f44babc66fa32f09b688fa0640d066a856936d987a466a8687c2d57189f73518653ffff071efe2101000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000ffffffffffffffffffffffff00000000000000000000000001010000009d508653010000000000000000000000000000000000000000000000000000000000000000ffffffff0e049e5086530101062f503253482fffffffff01c07a8102000000002321038a52f85595a8d8e7c1d8c256baeee2c9ea7ad0bf7fe534575be4eb47cdbf18f6ac00000000473045022100e86e6a211795858134d09d2febfb82ec5a4a2b9dce36ae92a67053ec80d634c902206c233b82d033f73c0452128792aafe79ec094676f358e6ad7631b51d60ec1995))
        ds = BCDataStream()
        with open(datadir, "rb") as file:
            ds.map_file(file, 0)
            data = self.read_block_n(ds, blockheight=1471400)
            file.close()
        print(data)


if __name__ == "__main__":
    unittest.main()

# 000006e022fc5e432e55cd61885d6ab9bb2ad6d5cef943f0e397ee21fe37b5db