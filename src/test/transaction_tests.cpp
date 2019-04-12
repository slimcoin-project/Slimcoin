#include <map>
#include <string>
//#include <iostreamp>
#include <fstream>
#include <vector>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/test/unit_test.hpp>
#include "json/json_spirit_reader_template.h"
#include "json/json_spirit_writer_template.h"
#include "json/json_spirit_utils.h"

#include "main.h"
#include "wallet.h"
#include "script.h"

using namespace std;
using namespace boost;
using namespace json_spirit;
// using namespace boost::algorithm;

struct F {
    F() : i( 0 ) { 
                    // BOOST_TEST_MESSAGE( "Tx fixture setup" );
                }
    ~F()         {
                    // BOOST_TEST_MESSAGE( "Tx fixture teardown" );
                }
    void setup() {
        static map<string, unsigned int> mapFlagNames = boost::assign::map_list_of
            (string("NONE"),(unsigned int)SCRIPT_VERIFY_NONE)
            (string("P2SH"),(unsigned int)SCRIPT_VERIFY_P2SH)
            (string("STRICTENC"),(unsigned int)SCRIPT_VERIFY_STRICTENC)
            (string("DISCOURAGE_UPGRADABLE_NOPS"),(unsigned int)SCRIPT_VERIFY_DISCOURAGE_UPGRADABLE_NOPS)
            (string("CHECKLOCKTIMEVERIFY"),(unsigned int)SCRIPT_VERIFY_CHECKLOCKTIMEVERIFY);
    };
    int i;
};

// In script_tests.cpp
extern Array read_json(const string& filename);
extern CScript ParseScript(string s);

BOOST_FIXTURE_TEST_SUITE(transaction_tests, F)

static map<string, unsigned int> mapFlagNames = boost::assign::map_list_of
    (string("NONE"), (unsigned int)SCRIPT_VERIFY_NONE)
    (string("P2SH"), (unsigned int)SCRIPT_VERIFY_P2SH)
    (string("STRICTENC"), (unsigned int)SCRIPT_VERIFY_STRICTENC)
    (string("DISCOURAGE_UPGRADABLE_NOPS"), (unsigned int)SCRIPT_VERIFY_DISCOURAGE_UPGRADABLE_NOPS)
    (string("CHECKLOCKTIMEVERIFY"), (unsigned int)SCRIPT_VERIFY_CHECKLOCKTIMEVERIFY);


unsigned int ParseScriptFlags(string strFlags)
{
    if (strFlags.empty()) {
        return 0;
    }
    unsigned int flags = 0;
    vector<string> words;
    boost::algorithm::split(words, strFlags, boost::algorithm::is_any_of(","));

    BOOST_FOREACH(string word, words)
    {
        if (!mapFlagNames.count(word))
            BOOST_ERROR("Bad test: unknown verification flag '" << word << "'");
        flags |= mapFlagNames[word];
    }

    return flags;
}

string FormatScriptFlags(unsigned int flags)
{
    if (flags == 0) {
        return "";
    }
    string ret;
    std::map<string, unsigned int>::const_iterator it = mapFlagNames.begin();
    while (it != mapFlagNames.end()) {
        if (flags & it->second) {
            ret += it->first + ",";
        }
        it++;
    }
    return ret.substr(0, ret.size() - 1);
}

// BOOST_AUTO_TEST_SUITE(transaction_tests)

BOOST_AUTO_TEST_CASE(tx_valid)
{
    // Read tests from test/data/tx_valid.json
    // Format is an array of arrays
    // Inner arrays are either [ "comment" ]
    // or [[[prevout hash, prevout index, prevout scriptPubKey], [input 2], ...],"], serializedTransaction, verifyFlags
    // ... where all scripts are stringified scripts.
    //
    // verifyFlags is a comma separated list of script verification flags to apply, or "NONE"
    // json_spirit::Array tests = read_json(std::string(json_tests::tx_valid, json_tests::tx_valid + sizeof(json_tests::tx_valid)));
    Array tests = read_json("tx_valid.json");

    // ScriptError err;
    BOOST_FOREACH(Value& tv, tests)
    {
        Array test = tv.get_array();
        string strTest = write_string(tv, false);
        if (test[0].type() == array_type)
        {
            if (test.size() != 3 || test[1].type() != str_type || test[2].type() != /*str*/bool_type)
            {
                BOOST_ERROR("Bad test: " << strTest);
                continue;
            }
            map<COutPoint, CScript> mapprevOutScriptPubKeys;
            Array inputs = test[0].get_array();
            bool fValid = true;
            BOOST_FOREACH(Value& input, inputs)
            {
                if (input.type() != array_type)
                {
                    fValid = false;
                    break;
                }
                Array vinput = input.get_array();
                if (vinput.size() != 3)
                {
                    fValid = false;
                    break;
                }

                mapprevOutScriptPubKeys[COutPoint(uint256(vinput[0].get_str()), vinput[1].get_int())] = ParseScript(vinput[2].get_str());
            }
            if (!fValid)
            {
                BOOST_ERROR("Bad test: " << strTest);
                continue;
            }
            // BOOST_TEST_MESSAGE("Parsed transaction: " << strTest << " should be valid.");
            try
            {
                string transaction = test[1].get_str();
                CDataStream stream(ParseHex(transaction), SER_NETWORK, PROTOCOL_VERSION);
                CTransaction tx;
                stream >> tx;

                // CValidationState state;
                strTest = "Simple deserialized transaction should be valid but isn't.";
                BOOST_CHECK_MESSAGE(tx.CheckTransaction(), strTest);

                for (unsigned int i = 0; i < tx.vin.size(); i++)
                {
                    if (!mapprevOutScriptPubKeys.count(tx.vin[i].prevout))
                    {
                        BOOST_ERROR("Bad test: " << strTest);
                        break;
                    }

                    unsigned int verify_flags = ParseScriptFlags(test[2].get_str());
                    BOOST_CHECK_MESSAGE(
                                VerifyScript(tx.vin[i].scriptSig, mapprevOutScriptPubKeys[tx.vin[i].prevout], tx, i, true, verify_flags, 0),
                            strTest);
                    // BOOST_CHECK_MESSAGE(VerifyScript(tx.vin[i].scriptSig, mapprevOutScriptPubKeys[tx.vin[i].prevout],verify_flags, TransactionSignatureChecker(&tx, i), &err), strTest);
                    // BOOST_CHECK_MESSAGE(err == SCRIPT_ERR_OK, ScriptErrorString(err));
                }
            }
              catch (std::exception& e) {
                  BOOST_ERROR("iostream error, bad parse of: " << test[1].get_str());
                  break;
            } catch (...) {
                  BOOST_ERROR("Bad parse of: " << strTest);
                  break;
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(tx_invalid)
{
    // Read tests from test/data/tx_invalid.json
    // Format is an array of arrays
    // Inner arrays are either [ "comment" ]
    // or [[[prevout hash, prevout index, prevout scriptPubKey], [input 2], ...],"], serializedTransaction, verifyFlags
    // ... where all scripts are stringified scripts.
    //
    // verifyFlags is a comma separated list of script verification flags to apply, or "NONE"
    Array tests = read_json("tx_invalid.json");

    // ScriptError err;
    BOOST_FOREACH(Value& tv, tests)
    {
        Array test = tv.get_array();
        string strTest = write_string(tv, false);
        if (test[0].type() == array_type)
        {
            if (test.size() != 3 || test[1].type() != str_type || test[2].type() != /*str*/bool_type)
            {
                BOOST_ERROR("Bad test: " << strTest);
                continue;
            }

            map<COutPoint, CScript> mapprevOutScriptPubKeys;
            Array inputs = test[0].get_array();
            bool fValid = true;
            BOOST_FOREACH(Value& input, inputs)
            {
                if (input.type() != array_type)
                {
                    fValid = false;
                    break;
                }
                Array vinput = input.get_array();
                if (vinput.size() != 3)
                {
                    fValid = false;
                    break;
                }

                mapprevOutScriptPubKeys[COutPoint(uint256(vinput[0].get_str()), vinput[1].get_int())] = ParseScript(vinput[2].get_str());
            }
            if (!fValid)
            {
                BOOST_ERROR("Bad test: " << strTest);
                continue;
            }
            try
            {
                string transaction = test[1].get_str();
                CDataStream stream(ParseHex(transaction), SER_NETWORK, PROTOCOL_VERSION);
                CTransaction tx;
                stream >> tx;

                // CValidationState state;
                // fValid = tx.CheckTransaction(state) && state.IsValid();
                fValid = tx.CheckTransaction();

                for (unsigned int i = 0; i < tx.vin.size() && fValid; i++)
                {
                    if (!mapprevOutScriptPubKeys.count(tx.vin[i].prevout))
                    {
                        BOOST_ERROR("Bad test: " << strTest);
                        break;
                    }

                    unsigned int verify_flags = ParseScriptFlags(test[2].get_str());
                    // fValid = VerifyScript(tx.vin[i].scriptSig, mapprevOutScriptPubKeys[tx.vin[i].prevout], verify_flags, TransactionSignatureChecker(&tx, i), &err);
                    fValid = VerifyScript(tx.vin[i].scriptSig, mapprevOutScriptPubKeys[tx.vin[i].prevout], tx, i, true, verify_flags, 0);
                    strTest = "Unverified script";
                }

                BOOST_CHECK_MESSAGE(!fValid, strTest);
                // BOOST_CHECK_MESSAGE(err != SCRIPT_ERR_OK, ScriptErrorString(err));
            }
              catch (std::exception& e) {
                  BOOST_ERROR("iostream error, bad parse of: " << test[1].get_str());
                  break;
            } catch (...) {
                  BOOST_ERROR("Bad parse of: " << strTest);
                  break;
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(basic_transaction_tests)
{
    // Random real transaction (f505d79e4a87a1d891975a62562aa6666b0ef0488f95fa7b2a97f2b27286df61)
    string transaction = "01000000f136c258010000000000000000000000000000000000000000000000000000000000000000ffffffff0e04f136c2580101062f503253482fffffffff01f0c7e70000000000232103fad597f3ea9b03a43e48b8f804face48ca78f57777fe41b9f35e623dbc521b28ac00000000";
    CDataStream stream(ParseHex(transaction), SER_NETWORK, PROTOCOL_VERSION);
    CTransaction tx;
    stream >> tx;
    BOOST_CHECK_MESSAGE(tx.CheckTransaction(), "A simple deserialized real transaction should be valid.");

    // Check that duplicate txins fail
    tx.vin.push_back(tx.vin[0]);
    BOOST_CHECK_MESSAGE(!tx.CheckTransaction(), "Transaction with duplicate txins should be invalid.");
}

//
// Helper: create two dummy transactions, each with
// two outputs.  The first has 11 and 50 CENT outputs
// paid to a TX_PUBKEY, the second 21 and 22 CENT outputs
// paid to a TX_PUBKEYHASH.
//
static std::vector<CTransaction>
SetupDummyInputs(CBasicKeyStore& keystoreRet, MapPrevTx& inputsRet)
{
    std::vector<CTransaction> dummyTransactions;
    dummyTransactions.resize(2);

    // Add some keys to the keystore:
    CKey key[4];
    for (int i = 0; i < 4; i++)
    {
        key[i].MakeNewKey(i % 2);
        keystoreRet.AddKey(key[i]);
    }

    // Create some dummy input transactions
    dummyTransactions[0].vout.resize(2);
    dummyTransactions[0].vout[0].nValue = 11*CENT;
    dummyTransactions[0].vout[0].scriptPubKey << key[0].GetPubKey() << OP_CHECKSIG;
    dummyTransactions[0].vout[1].nValue = 50*CENT;
    dummyTransactions[0].vout[1].scriptPubKey << key[1].GetPubKey() << OP_CHECKSIG;
    inputsRet[dummyTransactions[0].GetHash()] = make_pair(CTxIndex(), dummyTransactions[0]);

    dummyTransactions[1].vout.resize(2);
    dummyTransactions[1].vout[0].nValue = 21*CENT;
    dummyTransactions[1].vout[0].scriptPubKey.SetDestination(key[2].GetPubKey().GetID());
    dummyTransactions[1].vout[1].nValue = 22*CENT;
    dummyTransactions[1].vout[1].scriptPubKey.SetDestination(key[3].GetPubKey().GetID());
    inputsRet[dummyTransactions[1].GetHash()] = make_pair(CTxIndex(), dummyTransactions[1]);

    return dummyTransactions;
}

BOOST_AUTO_TEST_CASE(test_Get)
{
  CBasicKeyStore keystore;
  MapPrevTx dummyInputs;
  std::vector<CTransaction> dummyTransactions = SetupDummyInputs(keystore, dummyInputs);

  CTransaction t1;
  t1.vin.resize(3);
  t1.vin[0].prevout.hash = dummyTransactions[0].GetHash();
  t1.vin[0].prevout.n = 1;
  t1.vin[0].scriptSig << std::vector<unsigned char>(65, 0);
  t1.vin[1].prevout.hash = dummyTransactions[1].GetHash();
  t1.vin[1].prevout.n = 0;
  t1.vin[1].scriptSig << std::vector<unsigned char>(65, 0) << std::vector<unsigned char>(33, 4);
  t1.vin[2].prevout.hash = dummyTransactions[1].GetHash();
  t1.vin[2].prevout.n = 1;
  t1.vin[2].scriptSig << std::vector<unsigned char>(65, 0) << std::vector<unsigned char>(33, 4);
  t1.vout.resize(2);
  t1.vout[0].nValue = 90*CENT;
  t1.vout[0].scriptPubKey << OP_1;

  BOOST_CHECK(t1.AreInputsStandard(dummyInputs));
  BOOST_CHECK_EQUAL(t1.GetValueIn(dummyInputs), (50+21+22)*CENT);

  // Adding extra junk to the scriptSig should make it non-standard:
  t1.vin[0].scriptSig << OP_11;
  BOOST_CHECK(!t1.AreInputsStandard(dummyInputs));

  // ... as should not having enough:
  t1.vin[0].scriptSig = CScript();
  BOOST_CHECK(!t1.AreInputsStandard(dummyInputs));
}

BOOST_AUTO_TEST_CASE(test_IsStandard)
{
    CBasicKeyStore keystore;
    MapPrevTx dummyInputs;
    std::vector<CTransaction> dummyTransactions = SetupDummyInputs(keystore, dummyInputs);

    CTransaction t1;
    t1.vin.resize(1);
    t1.vin[0].prevout.hash = dummyTransactions[0].GetHash();
    t1.vin[0].prevout.n = 1;
    t1.vin[0].scriptSig << std::vector<unsigned char>(65, 0);
    t1.vout.resize(1);
    t1.vout[0].nValue = 90*CENT;
    CKey key;
    key.MakeNewKey(true);
    t1.vout[0].scriptPubKey.SetDestination(key.GetPubKey().GetID());
    BOOST_CHECK(t1.IsStandard());

    t1.vout[0].scriptPubKey = CScript() << OP_1;
    BOOST_CHECK(!t1.IsStandard());

    // 100-byte TX_NULL_DATA (standard)
    t1.vout[0].scriptPubKey = CScript() << OP_RETURN << ParseHex("172f12a1415ccc83391ede6b8f85119f810891c504678afdb0fe5548271967f1a67130b7105cd6a828e03909a67962e0ea1f61deb649f6bc3f4cef3804678afdb0fe5548271967f1a67130b7105cd6a828e03909a67962e0ea1f61deb649f6bc3f4cef38");
    BOOST_CHECK(t1.IsStandard());

    // 101-byte TX_NULL_DATA (non-standard)
    t1.vout[0].scriptPubKey = CScript() << OP_RETURN << ParseHex("45172f12a1415ccc83391ede6b8f85119f810891c504678afdb0fe5548271967f1a67130b7105cd6a828e03909a67962e0ea1f61deb649f6bc3f4cef3804678afdb0fe5548271967f1a67130b7105cd6a828e03909a67962e0ea1f61deb649f6bc3f4cef38");
    BOOST_CHECK(!t1.IsStandard());

    // TX_NULL_DATA w/o PUSHDATA
    t1.vout.resize(1);
    t1.vout[0].scriptPubKey = CScript() << OP_RETURN;
    BOOST_CHECK(t1.IsStandard());

    // Only one TX_NULL_DATA permitted in all cases
    t1.vout.resize(2);
    t1.vout[0].scriptPubKey = CScript() << OP_RETURN << ParseHex("172f12a1415ccc83391ede6b8f85119f810891c504678afdb0fe5548271967f1a67130b7105cd6a828e03909a67962e0ea1f61deb649f6bc3f4cef3804678afdb0fe5548271967f1a67130b7105cd6a828e03909a67962e0ea1f61deb649f6bc3f4cef38");
    t1.vout[1].scriptPubKey = CScript() << OP_RETURN << ParseHex("172f12a1415ccc83391ede6b8f85119f810891c504678afdb0fe5548271967f1a67130b7105cd6a828e03909a67962e0ea1f61deb649f6bc3f4cef3804678afdb0fe5548271967f1a67130b7105cd6a828e03909a67962e0ea1f61deb649f6bc3f4cef38");
    BOOST_CHECK(!t1.IsStandard());

    t1.vout[0].scriptPubKey = CScript() << OP_RETURN << ParseHex("172f12a1415ccc83391ede6b8f85119f810891c504678afdb0fe5548271967f1a67130b7105cd6a828e03909a67962e0ea1f61deb649f6bc3f4cef3804678afdb0fe5548271967f1a67130b7105cd6a828e03909a67962e0ea1f61deb649f6bc3f4cef38");
    t1.vout[1].scriptPubKey = CScript() << OP_RETURN;
    BOOST_CHECK(!t1.IsStandard());

    t1.vout[0].scriptPubKey = CScript() << OP_RETURN;
    t1.vout[1].scriptPubKey = CScript() << OP_RETURN;
    BOOST_CHECK(!t1.IsStandard());
}

BOOST_AUTO_TEST_CASE(test_GetThrow)
{
    CBasicKeyStore keystore;
    MapPrevTx dummyInputs;
    std::vector<CTransaction> dummyTransactions = SetupDummyInputs(keystore, dummyInputs);

    MapPrevTx missingInputs;

    CTransaction t1;
    t1.vin.resize(3);
    t1.vin[0].prevout.hash = dummyTransactions[0].GetHash();
    t1.vin[0].prevout.n = 0;
    t1.vin[1].prevout.hash = dummyTransactions[1].GetHash();
    t1.vin[1].prevout.n = 0;
    t1.vin[2].prevout.hash = dummyTransactions[1].GetHash();
    t1.vin[2].prevout.n = 1;
    t1.vout.resize(2);
    t1.vout[0].nValue = 90*CENT;
    t1.vout[0].scriptPubKey << OP_1;

    t1.vout[0].scriptPubKey = CScript() << OP_1;
    BOOST_CHECK_THROW(t1.AreInputsStandard(missingInputs), runtime_error);
    BOOST_CHECK_THROW(t1.GetValueIn(missingInputs), runtime_error);
}

BOOST_AUTO_TEST_SUITE_END()
