// Copyright (c) 2013-2014 The OpenSSL developers
// Copyright (c) 2013-2014 The Slimcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <string.h>        //for strlen

#include "sha256.h"

/*
static void digest_to_string(u8int *hash_digest, u8int *string)
{
  register u8int tmp_val;

  uint32_t i = 0;
  for(; i < SHA256_DIGEST_LENGTH; i++)
  {
    //format the most (left-most) significant 4bit hex
    tmp_val = *(hash_digest + i) >> 4;

    //get the integer into a char
    if(tmp_val <= 9)
      *(string + (i * 2)) = tmp_val + '0';
    else //add 87 to get the integer into the a-f of a hex
      *(string + (i * 2)) = tmp_val + 87; //ex: if tmp_val == 10 (0xa) then + 87 equals char 'a'

    //format the least (right-most) significant 4bit hex
    tmp_val = *(hash_digest + i) & 0x0F;

    //get the integer into a char
    if(tmp_val <= 9)
      *(string + (i * 2) + 1) = tmp_val + '0';
    else //add 87 to get the integer into the a-f of a hex
      *(string + (i * 2) + 1) = tmp_val + 87; //ex: if tmp_val == 10 (0xa) then + 87 equals char 'a'

  }

  //add the termination \000 to the string
  *(string + SHA256_LEN) = 0;
  
  return;
}
*/
const char * byte_to_hex =
        "000102030405060708090a0b0c0d0e0f"
        "101112131415161718191a1b1c1d1e1f"
        "202122232425262728292a2b2c2d2e2f"
        "303132333435363738393a3b3c3d3e3f"
        "404142434445464748494a4b4c4d4e4f"
        "505152535455565758595a5b5c5d5e5f"
        "606162636465666768696a6b6c6d6e6f"
        "707172737475767778797a7b7c7d7e7f"
        "808182838485868788898a8b8c8d8e8f"
        "909192939495969798999a9b9c9d9e9f"
        "a0a1a2a3a4a5a6a7a8a9aaabacadaeaf"
        "b0b1b2b3b4b5b6b7b8b9babbbcbdbebf"
        "c0c1c2c3c4c5c6c7c8c9cacbcccdcecf"
        "d0d1d2d3d4d5d6d7d8d9dadbdcdddedf"
        "e0e1e2e3e4e5e6e7e8e9eaebecedeeef"
        "f0f1f2f3f4f5f6f7f8f9fafbfcfdfeff";

void digest_to_string(u8int *hash_digest, u8int *str)
{
  register int si = 0;
  register int i = 0;
  for(; i < SHA256_DIGEST_LENGTH; i++)
  {
    memcpy(str+si,byte_to_hex + hash_digest[i]*2,2);
    si+=2;
  }
  str[SHA256_LEN] = 0;
  return;
}

void sha256_to_str(const u8int *data, size_t data_sz, u8int *outputBuffer, u8int *hash_digest)
{
  SHA256_CTX sha256;
  static u8int __digest__[SHA256_DIGEST_LENGTH];

  if(hash_digest == NULL)
    hash_digest = __digest__;

  SHA256_Init(&sha256);
  SHA256_Update(&sha256, data, data_sz);
  SHA256_Final(hash_digest, &sha256);

  //convert the digest to a string
  digest_to_string(hash_digest, outputBuffer);

  //sucess!
  return;
}

//optional arg: hash_digest
// same code from openssl lib, just a bit more specialized
uint256 sha256(const u8int *data, size_t data_sz, uint256 *hash_digest)
{
  SHA256_CTX hash;
  static uint256 m;

  if(!hash_digest)
    hash_digest = &m;

  SHA256_Init(&hash);
  SHA256_Update(&hash, data, data_sz);
  SHA256_Final((u8int*)hash_digest, &hash);
  //~ OPENSSL_cleanse(&hash, sizeof(hash));

  return *hash_digest;
}

void sha256_salt_to_str(const u8int *data, size_t data_sz, u8int *salt, size_t salt_sz, 
                        u8int *outputBuffer, u8int *hash_digest)
{
  SHA256_CTX sha256;
  SHA256_Init(&sha256);
  SHA256_Update(&sha256, data, data_sz);
  SHA256_Update(&sha256, salt, salt_sz);
  SHA256_Final(hash_digest, &sha256);

  //convert the digest to a string
  digest_to_string(hash_digest, outputBuffer);    

  //sucess!
  return;
}
