// Copyright (c) 2013-2014 The Slimcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <stdlib.h>        //malloc, free, realloc
#include <math.h>          //pow

#include "uint256.h"
#include "main.h"
#include "dcrypt.h"

//the base size for malloc/realloc will be 1KB
#define REALLOC_BASE_SZ   (1024)

typedef struct
{
  uint8_t *array;
  unsigned long long actual_array_sz;
  uint32_t times_realloced;

} Extend_Array;

inline void Extend_Array_init(Extend_Array *ExtArray)
{
  //initial values
  ExtArray->array = 0;
  ExtArray->actual_array_sz = 0;
  ExtArray->times_realloced = 0;
  return;
}

uint32_t hex_char_to_int(uint8_t c)
{
  if(c >= '0' && c <= '9')
    return c - '0';

  if(c >= 'a' && c <= 'f')
    return 10 + c - 'a';

  if(c >= 'A' && c <= 'F')
    return 10 + c - 'A';

  return -1;
}

inline void join_to_array(uint8_t *array, uint8_t join)
{
  *(array + SHA256_LEN) = join;
  return;
}

void extend_array(Extend_Array *extend_array, unsigned long long used_array_sz, 
                  uint8_t *extend, uint32_t extend_sz, uint8_t hashed_end)
{
  if(!extend_array)
    return;

  //make a few references
  unsigned long long *actual_array_sz = &extend_array->actual_array_sz;
  uint32_t *times_realloced = &extend_array->times_realloced;

  //if there is not enough room
  if((*actual_array_sz - used_array_sz) < (extend_sz + hashed_end))
  {
    //if extend_array->array has already been malloc'd
    if(*times_realloced)
    {
      //reallocate on an exponential curve, modern computers have plenty ram
      *actual_array_sz += pow(2, (*times_realloced)++) * REALLOC_BASE_SZ;
      extend_array->array = (uint8_t*)realloc(extend_array->array, *actual_array_sz);
    }else{
      //allocate the base size
      *actual_array_sz += REALLOC_BASE_SZ;
      (*times_realloced)++;

      extend_array->array = (uint8_t*)malloc(*actual_array_sz); //if we have not allocated anything, malloc
    }
  }

  //copy the data to be extended
  memcpy(extend_array->array + used_array_sz, extend, extend_sz);

  if(hashed_end)   
    *(extend_array->array + used_array_sz + extend_sz) = 0; //add the final \000 of the whole string array

  return;
}

uint64 mix_hashed_nums(uint8_t *hashed_nums, const uint8_t *unhashedData, size_t unhashed_sz,
                       uint8_t **mixed_hash, uint8_t *hash_digest)
{
  uint32_t i, index = 0;
  const uint32_t hashed_nums_len = SHA256_LEN;

  uint64 count;
  uint8_t tmp_val, tmp_array[SHA256_LEN + 2];

  //initialize the class for the extend hash
  Extend_Array new_hash;
  Extend_Array_init(&new_hash);

  //set the first hash length in the temp array to all 0xff
  memset(tmp_array, 0xff, SHA256_LEN);
  //set the last two bytes to \000
  *(tmp_array + SHA256_LEN) = *(tmp_array + SHA256_LEN + 1) = 0;

  for(count = 0;; count++)
  {
    //+1 to keeps a 0 value of *(hashed_nums + index) moving on
    i = hex_char_to_int(*(hashed_nums + index)) + 1;
    index += i;
    
    //if we hit the end of the hash, rehash it
    if(index >= hashed_nums_len)
    {
      index = index % hashed_nums_len;
      sha256_to_str(hashed_nums, hashed_nums_len, hashed_nums, hash_digest); //rescramble
    }
    
    tmp_val = *(hashed_nums + index);

    join_to_array(tmp_array, tmp_val); //plop tmp_val at the end of tmp_array
    sha256_to_str(tmp_array, SHA256_LEN + 1, tmp_array, hash_digest);

    //extend the expanded hash to the array
    extend_array(&new_hash, count * SHA256_LEN, tmp_array, SHA256_LEN, false);

    //check if the last value of hashed_nums is the same as the last value in tmp_array
    if(index == hashed_nums_len - 1)
      if(tmp_val == *(tmp_array + SHA256_LEN - 1))
      {
        //add to count since we extended the array, but break will exit the for loop and count
        // will not get incremenented by the for loop
        count++;
        break;
      }

  }

  //extend the unhashed data to the end and add the \000 to the end
  extend_array(&new_hash, count * SHA256_LEN, (u8int*)unhashedData, unhashed_sz, true);

  //assign the address of new_hash's array to mixed_hash
  *mixed_hash = new_hash.array;

  return count * SHA256_LEN + unhashed_sz;
}

uint256 dcrypt(const uint8_t *data, size_t data_sz, uint8_t *hash_digest)
{
  //dcrypt is really intense, don't use it for testnet hashes, 
  // it is too slow and takes more time to test, just use the traditional double sha256
  if(fTestNet)
  {
    uint256 hash1, hash2;

    sha256(data, data_sz, &hash1);
    sha256((const u8int*)&hash1, sizeof(uint256), &hash2);

    return hash2;
  }
  
  uint8_t hashed_nums[SHA256_LEN + 1], *mix_hash;
  uint256 hash;

  bool allocDigest = false;
  if(!hash_digest)
  {
    hash_digest = (uint8_t*)malloc(DCRYPT_DIGEST_LENGTH);
    allocDigest = true;
  }

  sha256_to_str(data, data_sz, hashed_nums, hash_digest);

  //mix the hashes up, magority of the time takes here
  uint64 mix_hash_len = mix_hashed_nums(hashed_nums, data, data_sz, &mix_hash, hash_digest);

  //apply the final hash to the output
  sha256((const uint8_t*)mix_hash, mix_hash_len, &hash);

  free(mix_hash);

  if(allocDigest)
    free(hash_digest);

  //sucess
  return hash;
}
