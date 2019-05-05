#!/usr/bin/python
#
# Copyright (c) 2011 The Bitcoin developers
# Distributed under the MIT/X11 software license, see the accompanying
# file license.txt or http://www.opensource.org/licenses/mit-license.php.
#

"""Tests of blockchain serialisation to RDF."""
import unittest
import json
import base64
import requests
import os
from datetime import datetime


host = '127.0.0.1'
port = 41682
rpc_port = 41683
testnet_port = 41684
testnet_rpc_port = 41685
username = os.environ["USER"]       # or near offer
password = os.environ["PASSWORD"]   # ditto
oldmaxcheckpointblockheight = 904630


class BitcoinRPC:
    OBJID = 1

    def __init__(self, host, port, username, password):
        # authpair = username + b":" + password
        # bp = base64.b64encode(authpair)
        # self.authhdr = "Basic {}".format(bp).encode('utf-8')
        # self.conn = client.HTTPConnection(host, port, False, 30)
        self.conn = requests.Session()
 
    def rpc(self, method, params=None):
        self.OBJID += 1
        obj = {
            'version': '1.1',
            'method': method,
            'id': self.OBJID}
        if params is None:
            obj['params'] = []
        else:
            obj['params'] = params
        resp = self.conn.get(
            'http://127.0.0.1:{}'.format(rpc_port),
            auth=(username, password),
            data=json.dumps(obj))

        if resp is None:
            print("JSON-RPC: no response")
            return None

        resp_obj = resp.json()
        if resp_obj is None:
            print("JSON-RPC: cannot JSON-decode body")
            return None
        if 'error' in resp_obj and resp_obj['error'] is not None:
            return resp_obj['error']
        if 'result' not in resp_obj:
            print("JSON-RPC: no result in object")
            return None

        return resp_obj['result']

    def getblockcount(self):
        return self.rpc('getblockcount')

    def getblockhash(self, params):
        return self.rpc('getblockhash', params=params)

    def getblock(self, params):
        return self.rpc('getblock', params=params)


class AcmeTests(unittest.TestCase):
    """Test Acme responses."""

    def setUp(self):
        """Setup."""
        self.rpc = BitcoinRPC(host, rpc_port, username, password)
        if self.rpc is None:
            print("No RPC connection")

    def tearDown(self):
        """Teardown."""
        pass

    def get_candidate(self, height):
        blockhash = self.rpc.getblockhash(params=[height])
        blockdata = self.rpc.getblock(params=[blockhash, True, True])
        # print(json.dumps(blockdata, sort_keys=True, indent=2, separators=(',', ': ')))
        blockdate = blockdata['time']
        dt = datetime.strptime(blockdate, "%Y-%m-%d %H:%M:%S %Z")  # "2017-03-28 02:52:36 UTC"
        return blockhash, blockdata, dt

    # @unittest.skip("Passed, skipping")
    def test01(self):
        current_height = self.rpc.getblockcount()
        s = int(((current_height - oldmaxcheckpointblockheight) + oldmaxcheckpointblockheight) / 12)
        for height in range(oldmaxcheckpointblockheight, current_height, s):
            bprev = height - 1
            bnext = height + 1
            blockhash, blockdata, dt = self.get_candidate(height)
            bpblockhash, bpblockdata, bpdt = self.get_candidate(bprev)
            bnblockhash, bnblockdata, bndt = self.get_candidate(bnext)
            if not (bpdt < dt):
                bpblockhash, bpblockdata, bpdt = self.get_candidate(bprev - 1)
            elif not (bndt > dt):
                bnblockhash, bnblockdata, bndt = self.get_candidate(bnext + 1)
            if (bpdt < dt) and (bndt > dt):
                print(dt.isoformat(), blockhash)
                print('''({},   uint256("0x{}"))'''.format(height, blockhash))
                print(json.dumps(blockdata, sort_keys=True, indent=2, separators=(',', ': ')))
            else:
                print("Failed with {}".format(height))


if __name__ == "__main__":
    unittest.main()

"""
Result of run of 2017-03-28 13:24, transcribed into src/checkpoints.cpp


    //
    // What makes a good checkpoint block?
    // + Is surrounded by blocks with reasonable timestamps
    //   (no blocks before with a timestamp after, none after with
    //    timestamp before)
    // + Contains no strange transactions
    //

nonstandard != strange, I guess

2014-10-11T14:25:18 6f7f1b5cf35b3be443f747ffdacbc7faba860ec82535dcbe1d1486eed6eea131
(121300,   uint256("0x6f7f1b5cf35b3be443f747ffdacbc7faba860ec82535dcbe1d1486eed6eea131"))
{
  "Formatted nEffectiveBurnCoins": "282952.367464",
  "bits": "1d1fffff",
  "difficulty": 0.03124954,
  "entropybit": 0,
  "flags": "proof-of-stake",
  "hash": "6f7f1b5cf35b3be443f747ffdacbc7faba860ec82535dcbe1d1486eed6eea131",
  "height": 121300,
  "merkleroot": "0d1edf0b4b8c9213acdd8b71ecacfbeb2cdb00fa02d8c147f6b346709998e76a",
  "mint": 0.084053,
  "modifier": "%x",
  "modifierchecksum": "b27f96f0",
  "nBurnBits": "1d00fde2",
  "nEffectiveBurnCoins": "-515474072",
  "nextblockhash": "0000000e85766e972be10689231d434b4c0933729de3db338328ff79f6e5c39d",
  "nonce": 0,
  "previousblockhash": "0a26ec7cefc5abffdd29ecb01590215e46e535e6a933ace84902049532b8b2d6",
  "proofhash": "000012de4776b4d252a0f49d421c2f0d772b5b4d0d1498cb43cb0788d9dd45da",
  "size": 545,
  "time": "2014-10-11 14:25:18 UTC",
  "tx": [
    {
      "is_coinbase": true,
      "is_coinstake": false,
      "locktime": 0,
      "txid": "a31262c54759098b1a717463c333723c288ea0ebfbddd03c52e0ffe9168057ab",
      "version": 1,
      "vin": [
        {
          "coinbase": "04ce3d39540115062f503253482f",
          "sequence": 4294967295
        }
      ],
      "vout": [
        {
          "n": 0,
          "scriptPubKey": {
            "asm": "",
            "hex": "",
            "type": "nonstandard"
          },
          "value": 0.0
        }
      ]
    },
    {
      "is_coinbase": false,
      "is_coinstake": true,
      "locktime": 0,
      "txid": "442ad3b28ab06075acfdbf7b23fc1c620665fda06d710a10a951738201b72091",
      "version": 1,
      "vin": [
        {
          "scriptSig": {
            "asm": "30440220298d522154faa8883f5cab2094af9fccaead8de01cb1bda10664e3b4a2c0cf030220168dac5d50dcdf9ec53e1d9b128b2b71567fea3787e8ae47b08474602209211201",
            "hex": "4730440220298d522154faa8883f5cab2094af9fccaead8de01cb1bda10664e3b4a2c0cf030220168dac5d50dcdf9ec53e1d9b128b2b71567fea3787e8ae47b08474602209211201"
          },
          "sequence": 4294967295,
          "txid": "26d6ad843ba23e495a55899590335cee57b39af314923c95569f37a33d28b2ab",
          "vout": 0
        }
      ],
      "vout": [
        {
          "n": 0,
          "scriptPubKey": {
            "asm": "",
            "hex": "",
            "type": "nonstandard"
          },
          "value": 0.0
        },
        {
          "n": 1,
          "scriptPubKey": {
            "addresses": [
              "SUYbP7zbLrewSshBAchpoUa186isWkvsdw"
            ],
            "asm": "0201f9b145819c5ee58a10bfc3fb57c0fe9b929ff49cbe780f14cbd522f0272a72 OP_CHECKSIG",
            "hex": "210201f9b145819c5ee58a10bfc3fb57c0fe9b929ff49cbe780f14cbd522f0272a72ac",
            "reqSig": 1,
            "type": "pubkey"
          },
          "value": 8.04
        },
        {
          "n": 2,
          "scriptPubKey": {
            "addresses": [
              "SUYbP7zbLrewSshBAchpoUa186isWkvsdw"
            ],
            "asm": "0201f9b145819c5ee58a10bfc3fb57c0fe9b929ff49cbe780f14cbd522f0272a72 OP_CHECKSIG",
            "hex": "210201f9b145819c5ee58a10bfc3fb57c0fe9b929ff49cbe780f14cbd522f0272a72ac",
            "reqSig": 1,
            "type": "pubkey"
          },
          "value": 8.054053
        }
      ]
    }
  ],
  "version": 1
}
2015-01-01T14:19:11 000000048436de97fd78b645a0ccdbd7c57be20172b49eeac42f78571ff45e9a
(199633,   uint256("0x000000048436de97fd78b645a0ccdbd7c57be20172b49eeac42f78571ff45e9a"))
{
  "Formatted nEffectiveBurnCoins": "508502.724025",
  "bits": "1d0bbbef",
  "difficulty": 0.08522029,
  "entropybit": 1,
  "flags": "proof-of-work",
  "hash": "000000048436de97fd78b645a0ccdbd7c57be20172b49eeac42f78571ff45e9a",
  "height": 199633,
  "merkleroot": "e1a22fea02a3d07ccc41801ee899abdcbb1fd5611f358491cd0776c8f9503b24",
  "mint": 11.56,
  "modifier": "%x",
  "modifierchecksum": "2e8d5a39",
  "nBurnBits": "1d00d141",
  "nEffectiveBurnCoins": "1696583097",
  "nextblockhash": "00000007854626f4e5905dad193d89bfb51c8cd09c2a028f4649a221007a227c",
  "nonce": 92344113,
  "previousblockhash": "e3de4d559566ad1702a89c65cf2ca9b07cc84f075999c94980b159d02c63d430",
  "proofhash": "000000048436de97fd78b645a0ccdbd7c57be20172b49eeac42f78571ff45e9a",
  "size": 356,
  "time": "2015-01-01 14:19:11 UTC",
  "tx": [
    {
      "is_coinbase": true,
      "is_coinstake": false,
      "locktime": 0,
      "txid": "e1a22fea02a3d07ccc41801ee899abdcbb1fd5611f358491cd0776c8f9503b24",
      "version": 1,
      "vin": [
        {
          "coinbase": "045f57a554021101062f503253482f",
          "sequence": 4294967295
        }
      ],
      "vout": [
        {
          "n": 0,
          "scriptPubKey": {
            "addresses": [
              "SVS2NvefKC4hFzviq1LMBXTtGdDzpi4FcH"
            ],
            "asm": "03045180c72ab1d5dc159bbac7003f27f2c98d14be631d114a354f4bb09f605440 OP_CHECKSIG",
            "hex": "2103045180c72ab1d5dc159bbac7003f27f2c98d14be631d114a354f4bb09f605440ac",
            "reqSig": 1,
            "type": "pubkey"
          },
          "value": 11.56
        }
      ]
    }
  ],
  "version": 1
}
2015-03-24T16:13:34 00000000328877ab62f47b55df02395e1ca425ee744c41490147d614abf50544
(277966,   uint256("0x00000000328877ab62f47b55df02395e1ca425ee744c41490147d614abf50544"))
{
  "Formatted nEffectiveBurnCoins": "520880.772817",
  "bits": "1d079876",
  "difficulty": 0.13165402,
  "entropybit": 1,
  "flags": "proof-of-work",
  "hash": "00000000328877ab62f47b55df02395e1ca425ee744c41490147d614abf50544",
  "height": 277966,
  "merkleroot": "64ef6008f19c17ab474a653d44ddad8f6e1834683315655adfaac97197eb509c",
  "mint": 10.37,
  "modifier": "%x",
  "modifierchecksum": "3c94d02a",
  "nBurnBits": "1c79170f",
  "nEffectiveBurnCoins": "1189730001",
  "nextblockhash": "59e5beaef24425ab7db837775947120323d869f8877f2c366e40b4e908eaeb12",
  "nonce": 211002710,
  "previousblockhash": "4a57c54ca401dfc70f7134c9a5f8836ccb89c870d34b48a0b679920457724b61",
  "proofhash": "00000000328877ab62f47b55df02395e1ca425ee744c41490147d614abf50544",
  "size": 356,
  "time": "2015-03-24 16:13:34 UTC",
  "tx": [
    {
      "is_coinbase": true,
      "is_coinstake": false,
      "locktime": 0,
      "txid": "64ef6008f19c17ab474a653d44ddad8f6e1834683315655adfaac97197eb509c",
      "version": 1,
      "vin": [
        {
          "coinbase": "042e8d115502d301062f503253482f",
          "sequence": 4294967295
        }
      ],
      "vout": [
        {
          "n": 0,
          "scriptPubKey": {
            "addresses": [
              "SZYaJSHNj58cU5WrZajkSSp9cuxDcAqb47"
            ],
            "asm": "029c9383f0c840bb6efd03ba443492f5c2139eacb7f9597f0d525113257a21a867 OP_CHECKSIG",
            "hex": "21029c9383f0c840bb6efd03ba443492f5c2139eacb7f9597f0d525113257a21a867ac",
            "reqSig": 1,
            "type": "pubkey"
          },
          "value": 10.37
        }
      ]
    }
  ],
  "version": 1
}
2015-06-11T04:22:41 0000003a779a6620b17a0ee0c093b2ee8e5006572685246bd9e41ed95c99e22d
(356299,   uint256("0x0000003a779a6620b17a0ee0c093b2ee8e5006572685246bd9e41ed95c99e22d"))
{
  "Formatted nEffectiveBurnCoins": "522846.291704",
  "bits": "1d5f47f5",
  "difficulty": 0.0104951,
  "entropybit": 0,
  "flags": "proof-of-work",
  "hash": "0000003a779a6620b17a0ee0c093b2ee8e5006572685246bd9e41ed95c99e22d",
  "height": 356299,
  "merkleroot": "479a8c8d13986571ec2fff3f5ac492b46cdd115e56b1737526e00ea3ca7d3a34",
  "mint": 19.53,
  "modifier": "%x",
  "modifierchecksum": "8a04ee28",
  "nBurnBits": "1d00b727",
  "nEffectiveBurnCoins": "-1139718408",
  "nextblockhash": "000000042e116823e78f8330699c7e8744ef16ef966fdc8acd46ade031fadf62",
  "nonce": 176998235,
  "previousblockhash": "de3ab0caeea0007589313a25d1494f4e74a2991b3acb7c94cf9a4e5d862b7fee",
  "proofhash": "0000003a779a6620b17a0ee0c093b2ee8e5006572685246bd9e41ed95c99e22d",
  "size": 356,
  "time": "2015-06-11 04:22:41 UTC",
  "tx": [
    {
      "is_coinbase": true,
      "is_coinstake": false,
      "locktime": 0,
      "txid": "479a8c8d13986571ec2fff3f5ac492b46cdd115e56b1737526e00ea3ca7d3a34",
      "version": 1,
      "vin": [
        {
          "coinbase": "04110d795502c100062f503253482f",
          "sequence": 4294967295
        }
      ],
      "vout": [
        {
          "n": 0,
          "scriptPubKey": {
            "addresses": [
              "SRVMHYTc7sSkJ5ig2wi12z1Z1L4iMoazvo"
            ],
            "asm": "03d3875906794b1a1506c881a58016050173bb1ebf0588416a76aefb59f4553714 OP_CHECKSIG",
            "hex": "2103d3875906794b1a1506c881a58016050173bb1ebf0588416a76aefb59f4553714ac",
            "reqSig": 1,
            "type": "pubkey"
          },
          "value": 19.53
        }
      ]
    }
  ],
  "version": 1
}
2015-09-02T02:14:20 b2e53b982227092d1fa84cf771ae2234f457600b5cf9274d30ad810150990239
(434632,   uint256("0xb2e53b982227092d1fa84cf771ae2234f457600b5cf9274d30ad810150990239"))
{
  "Formatted nEffectiveBurnCoins": "533803.09829",
  "Intermediate PoB hash": "009f071e6af1bdbeacda4c28328cd0815c9047586d9adc1fbc00327e83c61ca5",
  "bits": "1e00f467",
  "burnBlkHeight": 206413,
  "burnCTx": 3,
  "burnCTxOut": 1,
  "difficulty": 0.00409155,
  "entropybit": 0,
  "flags": "proof-of-burn",
  "hash": "b2e53b982227092d1fa84cf771ae2234f457600b5cf9274d30ad810150990239",
  "height": 434632,
  "merkleroot": "954fe8c5b529408aba8bd28e451fc8c04a0856e0c1c5c15945b58d0bfec67fcc",
  "mint": 17.74,
  "modifier": "%x",
  "modifierchecksum": "b6dfab64",
  "nBurnBits": "1d01a984",
  "nEffectiveBurnCoins": "1227153586",
  "nextblockhash": "0000008d994da2cb232fb53c02636db330dcf6d54b98e1921627bb01f1148ac9",
  "nonce": 0,
  "previousblockhash": "0000000cbe7cb28faf14aae8ccad4742234d309070e54997b39b5fb22f62986c",
  "proofhash": "000000015e080000000000000000000000000000000000000000000000000001",
  "size": 355,
  "time": "2015-09-02 02:14:20 UTC",
  "tx": [
    {
      "is_coinbase": true,
      "is_coinstake": false,
      "locktime": 0,
      "txid": "954fe8c5b529408aba8bd28e451fc8c04a0856e0c1c5c15945b58d0bfec67fcc",
      "version": 1,
      "vin": [
        {
          "coinbase": "047c5be6550102062f503253482f",
          "sequence": 4294967295
        }
      ],
      "vout": [
        {
          "n": 0,
          "scriptPubKey": {
            "addresses": [
              "SbfsGub6rjKkb3krsFeemhdLL34RWRj9DQ"
            ],
            "asm": "032f7dbbffe06c130b8ef5aaecbb9e088182eae961ab18f28f2326d395ca4af787 OP_CHECKSIG",
            "hex": "21032f7dbbffe06c130b8ef5aaecbb9e088182eae961ab18f28f2326d395ca4af787ac",
            "reqSig": 1,
            "type": "pubkey"
          },
          "value": 17.74
        }
      ]
    }
  ],
  "version": 1
}
2015-11-23T09:15:07 000000128da1ec0a285e3ec3e7a319433fe6ae58093a67133959eb7b1e58f935
(512965,   uint256("0x000000128da1ec0a285e3ec3e7a319433fe6ae58093a67133959eb7b1e58f935"))
{
  "Formatted nEffectiveBurnCoins": "943090.026063",
  "bits": "1e01f9ee",
  "difficulty": 0.00197653,
  "entropybit": 1,
  "flags": "proof-of-work",
  "hash": "000000128da1ec0a285e3ec3e7a319433fe6ae58093a67133959eb7b1e58f935",
  "height": 512965,
  "merkleroot": "cce50c65dd7134ab33d7515cc4dde950465af4c7a5920b4d3a972059c8d4e745",
  "mint": 29.64,
  "modifier": "%x",
  "modifierchecksum": "2c84a23d",
  "nBurnBits": "1c51fd85",
  "nEffectiveBurnCoins": "-1802779057",
  "nextblockhash": "5ccb235beb949cfc513167bd4623c72d436a1090cf998b55431d9d8bce5a08ef",
  "nonce": 3349476,
  "previousblockhash": "0000019298c13397a7ef1a6662cc54f28a042637cea46b49d14ce23f04d8f976",
  "proofhash": "000000128da1ec0a285e3ec3e7a319433fe6ae58093a67133959eb7b1e58f935",
  "size": 356,
  "time": "2015-11-23 09:15:07 UTC",
  "tx": [
    {
      "is_coinbase": true,
      "is_coinstake": false,
      "locktime": 0,
      "txid": "cce50c65dd7134ab33d7515cc4dde950465af4c7a5920b4d3a972059c8d4e745",
      "version": 1,
      "vin": [
        {
          "coinbase": "041bd95256015b062f503253482f",
          "sequence": 4294967295
        }
      ],
      "vout": [
        {
          "n": 0,
          "scriptPubKey": {
            "addresses": [
              "ScahphzH1A8aHZiGySexFYVtANhp94kV8Z"
            ],
            "asm": "027394c448b4c3a8b5e5c9467f1f6e4eaf3ca7d86e1fe05ccb734f70df224dfc19 OP_CHECKSIG",
            "hex": "21027394c448b4c3a8b5e5c9467f1f6e4eaf3ca7d86e1fe05ccb734f70df224dfc19ac",
            "reqSig": 1,
            "type": "pubkey"
          },
          "value": 29.64
        }
      ]
    }
  ],
  "version": 1
}
2016-02-20T19:22:12 00000053148f361c4affc7508f6c92ed4c9bf109cf609c18a9ff7c71828bb764
(591298,   uint256("0x00000053148f361c4affc7508f6c92ed4c9bf109cf609c18a9ff7c71828bb764"))
{
  "Formatted nEffectiveBurnCoins": "908028.907135",
  "bits": "1e00cd22",
  "difficulty": 0.00487482,
  "entropybit": 0,
  "flags": "proof-of-work",
  "hash": "00000053148f361c4affc7508f6c92ed4c9bf109cf609c18a9ff7c71828bb764",
  "height": 591298,
  "merkleroot": "bae8ed991b4c4fb82f46826a6bf8a8b514470943e60d471f1f0646821624cdb8",
  "mint": 23.65,
  "modifier": "%x",
  "modifierchecksum": "92660d0e",
  "nBurnBits": "1d009386",
  "nEffectiveBurnCoins": "1790807679",
  "nextblockhash": "e819409cb1b0c8dd2a1e4ae9527f7c0e91bbcc60965ae1f60b6b10ae2e75ea77",
  "nonce": 24811530,
  "previousblockhash": "000000c9f1166d3047282f8833c069033c610b57ca16b57f584d067c8de2881b",
  "proofhash": "00000053148f361c4affc7508f6c92ed4c9bf109cf609c18a9ff7c71828bb764",
  "size": 355,
  "time": "2016-02-20 19:22:12 UTC",
  "tx": [
    {
      "is_coinbase": true,
      "is_coinstake": false,
      "locktime": 0,
      "txid": "bae8ed991b4c4fb82f46826a6bf8a8b514470943e60d471f1f0646821624cdb8",
      "version": 1,
      "vin": [
        {
          "coinbase": "04e4bcc856011c062f503253482f",
          "sequence": 4294967295
        }
      ],
      "vout": [
        {
          "n": 0,
          "scriptPubKey": {
            "addresses": [
              "SNSc2JCAumM51koAd4B8udVLkWF4AkWa1K"
            ],
            "asm": "020bc29ab3c73b039acbfe50c0c647e03923391a968abaf5250cf2da82f0733e6a OP_CHECKSIG",
            "hex": "21020bc29ab3c73b039acbfe50c0c647e03923391a968abaf5250cf2da82f0733e6aac",
            "reqSig": 1,
            "type": "pubkey"
          },
          "value": 23.65
        }
      ]
    }
  ],
  "version": 1
}
2016-05-15T23:31:22 00000037214a52c13a7f50ffffc5b3bb8d8ed3be92dbe4b8b342ec527a4d7330
(669631,   uint256("0x00000037214a52c13a7f50ffffc5b3bb8d8ed3be92dbe4b8b342ec527a4d7330"))
{
  "Formatted nEffectiveBurnCoins": "852159.060489",
  "bits": "1e00980d",
  "difficulty": 0.00657665,
  "entropybit": 1,
  "flags": "proof-of-work",
  "hash": "00000037214a52c13a7f50ffffc5b3bb8d8ed3be92dbe4b8b342ec527a4d7330",
  "height": 669631,
  "merkleroot": "ec9e54a1b137c93a121e1733a78fb1df7147587cce86473cc8de3db3cf0dece4",
  "mint": 21.94,
  "modifier": "%x",
  "modifierchecksum": "2660048d",
  "nBurnBits": "1d00a713",
  "nEffectiveBurnCoins": "1755535881",
  "nextblockhash": "000000794060decc72a7aa60f4c4f49f1bbd65cd477b470e283737b035c43009",
  "nonce": 2147513635,
  "previousblockhash": "000000429ad57cbf759d98c16c901ec5f5edb23377fbba1f51eeeb70839c7179",
  "proofhash": "00000037214a52c13a7f50ffffc5b3bb8d8ed3be92dbe4b8b342ec527a4d7330",
  "size": 356,
  "time": "2016-05-15 23:31:22 UTC",
  "tx": [
    {
      "is_coinbase": true,
      "is_coinstake": false,
      "locktime": 0,
      "txid": "ec9e54a1b137c93a121e1733a78fb1df7147587cce86473cc8de3db3cf0dece4",
      "version": 1,
      "vin": [
        {
          "coinbase": "04ca06395702ad1b062f503253482f",
          "sequence": 4294967295
        }
      ],
      "vout": [
        {
          "n": 0,
          "scriptPubKey": {
            "addresses": [
              "SRSJDpm83SttTaaKkTKSA7au8YNnmi7DLX"
            ],
            "asm": "035610b10cfd1ad3aa1209a7d5f160c61cd7c148b1fbec52e103afe1363ad1e7ae OP_CHECKSIG",
            "hex": "21035610b10cfd1ad3aa1209a7d5f160c61cd7c148b1fbec52e103afe1363ad1e7aeac",
            "reqSig": 1,
            "type": "pubkey"
          },
          "value": 21.94
        }
      ]
    }
  ],
  "version": 1
}
2016-08-11T03:38:35 000000410095e63c64bd3f25ae642464653ada088483f0bc7ed69a609dfc17a0
(747964,   uint256("0x000000410095e63c64bd3f25ae642464653ada088483f0bc7ed69a609dfc17a0"))
{
  "Formatted nEffectiveBurnCoins": "809905.25189",
  "bits": "1d78b6bf",
  "difficulty": 0.00828393,
  "entropybit": 0,
  "flags": "proof-of-work",
  "hash": "000000410095e63c64bd3f25ae642464653ada088483f0bc7ed69a609dfc17a0",
  "height": 747964,
  "merkleroot": "8313bdd16840d81d8f633df556049ff986fdd1997370c491cc55a4a4d04751ef",
  "mint": 20.72,
  "modifier": "%x",
  "modifierchecksum": "01e1deef",
  "nBurnBits": "1d0118ee",
  "nEffectiveBurnCoins": "-1843567054",
  "nextblockhash": "664cdeac18c9c22e14fe9b5cd0a0b1085498fdb5bdc623ca8f8c5f8a6b2be1d4",
  "nonce": 405199837,
  "previousblockhash": "000000097da317e044c1f01ae68f3d679b27c9a35746390283cf2a4b022347fa",
  "proofhash": "000000410095e63c64bd3f25ae642464653ada088483f0bc7ed69a609dfc17a0",
  "size": 355,
  "time": "2016-08-11 03:38:35 UTC",
  "tx": [
    {
      "is_coinbase": true,
      "is_coinstake": false,
      "locktime": 0,
      "txid": "8313bdd16840d81d8f633df556049ff986fdd1997370c491cc55a4a4d04751ef",
      "version": 1,
      "vin": [
        {
          "coinbase": "043bf3ab570137062f503253482f",
          "sequence": 4294967295
        }
      ],
      "vout": [
        {
          "n": 0,
          "scriptPubKey": {
            "addresses": [
              "SXAagUjLQw47LjgfjjCmmbYPW7j9Cxfz1D"
            ],
            "asm": "024639dba09a651a7a28bbcfda392cc26d05ab5713742f2963562d77615025624b OP_CHECKSIG",
            "hex": "21024639dba09a651a7a28bbcfda392cc26d05ab5713742f2963562d77615025624bac",
            "reqSig": 1,
            "type": "pubkey"
          },
          "value": 20.72
        }
      ]
    }
  ],
  "version": 1
}
2016-11-30T10:18:02 000001af75fbae8557d20337dff0bed05222018cf496aa7b53e1fdad99800e42
(826297,   uint256("0x000001af75fbae8557d20337dff0bed05222018cf496aa7b53e1fdad99800e42"))
{
  "Formatted nEffectiveBurnCoins": "794273.732087",
  "bits": "1e03b62c",
  "difficulty": 0.00105243,
  "entropybit": 0,
  "flags": "proof-of-work",
  "hash": "000001af75fbae8557d20337dff0bed05222018cf496aa7b53e1fdad99800e42",
  "height": 826297,
  "merkleroot": "bfe0a81253496fdbfc9befc08462dfe07e4908d57023d90cb97cd6b8516ec1b0",
  "mint": 34.7,
  "modifier": "%x",
  "modifierchecksum": "bc79ef62",
  "nBurnBits": "1d011095",
  "nEffectiveBurnCoins": "-295217673",
  "nextblockhash": "1b0848051b08299819d064f16dedf57c8405b88eb916f26dff893042efbfc034",
  "nonce": 25350,
  "previousblockhash": "00000118fb5d668591b06e09815b02a9f88c9a559903e55ab1e85878020c5d63",
  "proofhash": "000001af75fbae8557d20337dff0bed05222018cf496aa7b53e1fdad99800e42",
  "size": 355,
  "time": "2016-11-30 10:18:02 UTC",
  "tx": [
    {
      "is_coinbase": true,
      "is_coinstake": false,
      "locktime": 0,
      "txid": "bfe0a81253496fdbfc9befc08462dfe07e4908d57023d90cb97cd6b8516ec1b0",
      "version": 1,
      "vin": [
        {
          "coinbase": "0430a73e580102062f503253482f",
          "sequence": 4294967295
        }
      ],
      "vout": [
        {
          "n": 0,
          "scriptPubKey": {
            "addresses": [
              "Se19799GmhQZ6swUyeRwM97M7YucQgqie1"
            ],
            "asm": "023f1899408f6666e3776e8f2e6c527f4ce21a1e91d3d77ffca61f3af09f644a4c OP_CHECKSIG",
            "hex": "21023f1899408f6666e3776e8f2e6c527f4ce21a1e91d3d77ffca61f3af09f644a4cac",
            "reqSig": 1,
            "type": "pubkey"
          },
          "value": 34.7
        }
      ]
    }
  ],
  "version": 1
}
2017-02-18T16:09:08 0000003ac05d489aa58bd9e28bebf98b612f6a7903b94a900f8b977a3b3a4e86
(904630,   uint256("0x0000003ac05d489aa58bd9e28bebf98b612f6a7903b94a900f8b977a3b3a4e86"))
{
  "Formatted nEffectiveBurnCoins": "887106.531172",
  "bits": "1d63a6f6",
  "difficulty": 0.01003475,
  "entropybit": 1,
  "flags": "proof-of-work",
  "hash": "0000003ac05d489aa58bd9e28bebf98b612f6a7903b94a900f8b977a3b3a4e86",
  "height": 904630,
  "merkleroot": "e073e115ee40c79e951d4e387fedb8c2433be79a341a30040f67bfd06db7ba68",
  "mint": 19.75,
  "modifier": "%x",
  "modifierchecksum": "31f350f6",
  "nBurnBits": "1d01509e",
  "nEffectiveBurnCoins": "-1951699100",
  "nextblockhash": "73781178fbe32f41f76ef5838d6c59de3cd864752261ff178b59a0ca3fc33565",
  "nonce": 21093775,
  "previousblockhash": "201d966e77290f4772c31dadc5b1aa9290eb5bdb8f2d873d781f1430a646fc71",
  "proofhash": "0000003ac05d489aa58bd9e28bebf98b612f6a7903b94a900f8b977a3b3a4e86",
  "size": 356,
  "time": "2017-02-18 16:09:08 UTC",
  "tx": [
    {
      "is_coinbase": true,
      "is_coinstake": false,
      "locktime": 0,
      "txid": "e073e115ee40c79e951d4e387fedb8c2433be79a341a30040f67bfd06db7ba68",
      "version": 1,
      "vin": [
        {
          "coinbase": "04a471a8580103062f503253482f",
          "sequence": 4294967295
        }
      ],
      "vout": [
        {
          "n": 0,
          "scriptPubKey": {
            "addresses": [
              "SieZU3SHwHkPH3cBw4i6dsUiFE9p6NpWkK"
            ],
            "asm": "03c4aa9c7fe4a5d34e7b00ccea58ce526b8d1c95c8350b8d7253433688e81090de OP_CHECKSIG",
            "hex": "2103c4aa9c7fe4a5d34e7b00ccea58ce526b8d1c95c8350b8d7253433688e81090deac",
            "reqSig": 1,
            "type": "pubkey"
          },
          "value": 19.75
        }
      ]
    }
  ],
  "version": 1
}
"""