// Copyright (c) 2014 The Fusioncoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>

using namespace std;
using namespace boost;

#include "keystore.h"
#include "bignum.h"
#include "key.h"
#include "main.h"
// #include "sync.h"
#include "util.h"
#include "smalldata.h"
#include "db.h"
// #include "txdb.h"

bool fAdEnabled = false;

static unsigned char pchSmallDataHeader1[] = { 0xfa, 0xce, SMALLDATA_TYPE_PLAINTEXT, 0, 0} ;
static unsigned char pchSmallDataHeader2[] = { 0xfa, 0xce, SMALLDATA_TYPE_BROADCAST, 0, 0} ;
const unsigned char *GetSmallDataHeader(int type)
{
    switch (type)
    {
    case SMALLDATA_TYPE_PLAINTEXT:
        return pchSmallDataHeader1;
    case SMALLDATA_TYPE_BROADCAST:
        return pchSmallDataHeader2;
    default:
        break;
    }

    return NULL;
}

bool GetTxMessage(CTransaction &tx, std::string &msg, std::string &addr, bool &isBroadcast)
{
    txnouttype whichType;
    BOOST_FOREACH(const CTxOut& txout, tx.vout) {
        // if ( 0 != txout.nValue )
        //    continue;

        vector<valtype> vSolutions;
        txnouttype whichType;
        if (!Solver(txout.scriptPubKey, whichType, vSolutions))
            return false;

        if (whichType == TX_NULL_DATA)
        {
            char start = 2;
            if ( txout.scriptPubKey[1] == 0x4c )
                start = 3;

            if ( txout.scriptPubKey[start] != 0xfa || txout.scriptPubKey[start + 1] != 0xce )
                return false;

            if ( txout.scriptPubKey[start + 2] == SMALLDATA_TYPE_PLAINTEXT )
                isBroadcast = false;
            else if ( txout.scriptPubKey[start + 2] == SMALLDATA_TYPE_BROADCAST )
                isBroadcast = true;
            else 
                return false;

            std::string ret_str(txout.scriptPubKey.begin() + start + 4, txout.scriptPubKey.end() - 36);
            std::string ret_addr(txout.scriptPubKey.begin() + start + 4 + ret_str.length() + 2, txout.scriptPubKey.end());
            msg = ret_str;
            addr = ret_addr;
            return true;
        }
    }
    
    return false;
}
