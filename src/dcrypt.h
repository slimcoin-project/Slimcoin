// Copyright (c) 2013-2014 The Slimcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef DCRYPT_H
#define DCRYPT_H

#include "sha256.h"

#define DCRYPT_DIGEST_LENGTH SHA256_DIGEST_LENGTH 

//the dcrypt hashing algorithm for a single piece of data
uint256 dcrypt(const uint8_t *data, size_t data_sz, uint8_t *hash_digest = 0);

#endif
