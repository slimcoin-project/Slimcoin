// Copyright (c) 2014 The Fusioncoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef H_SMALL_DATA_FUSIONCOIN
#define H_SMALL_DATA_FUSIONCOIN

#include <string>
#include <vector>

#include <boost/foreach.hpp>
#include <boost/variant.hpp>

#include "keystore.h"
#include "bignum.h"

class CTransaction;

enum{
    SMALLDATA_TYPE_NULL,
    SMALLDATA_TYPE_PLAINTEXT,
    SMALLDATA_TYPE_BROADCAST,
};

const unsigned char *GetSmallDataHeader(int type);

bool GetTxMessage(CTransaction &tx, std::string &msg, bool &isBroadcast);

#endif // H_SMALL_DATA_FUSIONCOIN
