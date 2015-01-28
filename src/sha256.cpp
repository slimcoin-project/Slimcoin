// Copyright (c) 2013-2014 The OpenSSL developers
// Copyright (c) 2013-2014 The Slimcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <string.h>        //for strlen

#include "sha256.h"

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
