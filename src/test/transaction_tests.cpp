#include <boost/test/unit_test.hpp>

#include "main.h"
#include "wallet.h"

using namespace std;

BOOST_AUTO_TEST_SUITE(transaction_tests)

BOOST_AUTO_TEST_CASE(basic_transaction_tests)
{

  // Original SLIMCoin transaction_test.cpp, inherited from bitcoin 6 years ago
  // Random real transaction (e2769b09e784f32f62ef849763d4f45b98e07ba658647343b915ff832b110436)
  /*
  {
    "result": "01000000016bff7fcd4f8565ef406dd5d63d4ff94f318fe82027fd4dc451b04474019f74b4000000008c493046022100da0dc6aecefe1e06efdf05773757deb168820930e3b0d03f46f5fcf150bf990c022100d25b5c87040076e4f253f8262e763e2dd51e7ff0be157727c4bc42807f17bd39014104e6c26ef67dc610d2cd192484789a6cf9aea9930b944b7e2db5342b9d9e5b9ff79aff9a2ee1978dd7fd01dfc522ee02283d3b06a9d03acf8096968d7dbb0f9178ffffffff028ba7940e000000001976a914badeecfdef0507247fc8f74241d73bc039972d7b88ac4094a802000000001976a914c10932483fec93ed51f5fe95e72559f2cc7043f988ac00000000",
    "error": null,
    "id": null
  }
  */
  // unsigned char ch[] = {0x01, 0x00, 0x00, 0x00, 0x01, 0x6b, 0xff, 0x7f, 0xcd, 0x4f, 0x85, 0x65, 0xef, 0x40, 0x6d, 0xd5, 0xd6, 0x3d, 0x4f, 0xf9, 0x4f, 0x31, 0x8f, 0xe8, 0x20, 0x27, 0xfd, 0x4d, 0xc4, 0x51, 0xb0, 0x44, 0x74, 0x01, 0x9f, 0x74, 0xb4, 0x00, 0x00, 0x00, 0x00, 0x8c, 0x49, 0x30, 0x46, 0x02, 0x21, 0x00, 0xda, 0x0d, 0xc6, 0xae, 0xce, 0xfe, 0x1e, 0x06, 0xef, 0xdf, 0x05, 0x77, 0x37, 0x57, 0xde, 0xb1, 0x68, 0x82, 0x09, 0x30, 0xe3, 0xb0, 0xd0, 0x3f, 0x46, 0xf5, 0xfc, 0xf1, 0x50, 0xbf, 0x99, 0x0c, 0x02, 0x21, 0x00, 0xd2, 0x5b, 0x5c, 0x87, 0x04, 0x00, 0x76, 0xe4, 0xf2, 0x53, 0xf8, 0x26, 0x2e, 0x76, 0x3e, 0x2d, 0xd5, 0x1e, 0x7f, 0xf0, 0xbe, 0x15, 0x77, 0x27, 0xc4, 0xbc, 0x42, 0x80, 0x7f, 0x17, 0xbd, 0x39, 0x01, 0x41, 0x04, 0xe6, 0xc2, 0x6e, 0xf6, 0x7d, 0xc6, 0x10, 0xd2, 0xcd, 0x19, 0x24, 0x84, 0x78, 0x9a, 0x6c, 0xf9, 0xae, 0xa9, 0x93, 0x0b, 0x94, 0x4b, 0x7e, 0x2d, 0xb5, 0x34, 0x2b, 0x9d, 0x9e, 0x5b, 0x9f, 0xf7, 0x9a, 0xff, 0x9a, 0x2e, 0xe1, 0x97, 0x8d, 0xd7, 0xfd, 0x01, 0xdf, 0xc5, 0x22, 0xee, 0x02, 0x28, 0x3d, 0x3b, 0x06, 0xa9, 0xd0, 0x3a, 0xcf, 0x80, 0x96, 0x96, 0x8d, 0x7d, 0xbb, 0x0f, 0x91, 0x78, 0xff, 0xff, 0xff, 0xff, 0x02, 0x8b, 0xa7, 0x94, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x19, 0x76, 0xa9, 0x14, 0xba, 0xde, 0xec, 0xfd, 0xef, 0x05, 0x07, 0x24, 0x7f, 0xc8, 0xf7, 0x42, 0x41, 0xd7, 0x3b, 0xc0, 0x39, 0x97, 0x2d, 0x7b, 0x88, 0xac, 0x40, 0x94, 0xa8, 0x02, 0x00, 0x00, 0x00, 0x00, 0x19, 0x76, 0xa9, 0x14, 0xc1, 0x09, 0x32, 0x48, 0x3f, 0xec, 0x93, 0xed, 0x51, 0xf5, 0xfe, 0x95, 0xe7, 0x25, 0x59, 0xf2, 0xcc, 0x70, 0x43, 0xf9, 0x88, 0xac, 0x00, 0x00, 0x00, 0x00, 0x00};

  // Real Slimcoin tx (f505d79e4a87a1d891975a62562aa6666b0ef0488f95fa7b2a97f2b27286df61)
  // unsigned char ch[] = {0x01, 0x00, 0x00, 0x00, 0xf1, 0x36, 0xc2, 0x58, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x0e, 0x04, 0xf1, 0x36, 0xc2, 0x58, 0x01, 0x01, 0x06, 0x2f, 0x50, 0x32, 0x53, 0x48, 0x2f, 0xff, 0xff, 0xff, 0xff, 0x01, 0xf0, 0xc7, 0xe7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x23, 0x21, 0x03, 0xfa, 0xd5, 0x97, 0xf3, 0xea, 0x9b, 0x03, 0xa4, 0x3e, 0x48, 0xb8, 0xf8, 0x04, 0xfa, 0xce, 0x48, 0xca, 0x78, 0xf5, 0x77, 0x77, 0xfe, 0x41, 0xb9, 0xf3, 0x5e, 0x62, 0x3d, 0xbc, 0x52, 0x1b, 0x28, 0xac, 0x00, 0x00, 0x00, 0x00};
  /*
  unsigned char ch[] = {1, 0, 0, 0, 241, 54, 194, 88, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 255, 14, 4, 241, 54, 194, 88, 1, 1, 6, 47, 80, 50, 83, 72, 47, 255, 255, 255, 255, 1, 240, 199, 231, 0, 0, 0, 0, 0, 35, 33, 3, 250, 213, 151, 243, 234, 155, 3, 164, 62, 72, 184, 248, 4, 250, 206, 72, 202, 120, 245, 119, 119, 254, 65, 185, 243, 94, 98, 61, 188, 82, 27, 40, 172, 0, 0, 0, 0};

  vector<unsigned char> vch(ch, ch + sizeof(ch) - 1);
  try
  {
    CDataStream stream(vch, SER_DISK, CLIENT_VERSION);
    CTransaction tx;
    // fatal error in "basic_transaction_tests": std::runtime_error: CDataStream::read() : end of data: iostream error
    stream >> tx;
    BOOST_CHECK_MESSAGE(tx.CheckTransaction(), "Simple deserialized transaction should be valid.");

    // Check that duplicate txins fail
    tx.vin.push_back(tx.vin[0]);
    BOOST_CHECK_MESSAGE(!tx.CheckTransaction(), "Transaction with duplicate txins should be invalid.");
  }
  catch (std::exception& e) {
      PrintException(&e, "basic_transaction_tests");
  } catch (...) {
      PrintException(NULL, "basic_transaction_tests");
  }
  */
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
  for(int i = 0; i < 4; i++)
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
  dummyTransactions[1].vout[0].scriptPubKey.SetBitcoinAddress(key[2].GetPubKey());
  dummyTransactions[1].vout[1].nValue = 22*CENT;
  dummyTransactions[1].vout[1].scriptPubKey.SetBitcoinAddress(key[3].GetPubKey());
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

  BOOST_CHECK_THROW(t1.AreInputsStandard(missingInputs), runtime_error);
  BOOST_CHECK_THROW(t1.GetValueIn(missingInputs), runtime_error);

  t1.vout[0].scriptPubKey = CScript() << OP_1;
  BOOST_CHECK(t1.IsStandard());

  /* OP_RETURN tests */
  // 80-byte TX_NULL_DATA (standard)
  t1.vout[0].scriptPubKey = CScript() << OP_RETURN << ParseHex("04678afdb0fe5548271967f1a67130b7105cd6a828e03909a67962e0ea1f61deb649f6bc3f4cef3804678afdb0fe5548271967f1a67130b7105cd6a828e03909a67962e0ea1f61deb649f6bc3f4cef38");
  BOOST_CHECK(t1.IsStandard());

  // 81-byte TX_NULL_DATA (non-standard)
  t1.vout[0].scriptPubKey = CScript() << OP_RETURN << ParseHex("04678afdb0fe5548271967f1a67130b7105cd6a828e03909a67962e0ea1f61deb649f6bc3f4cef3804678afdb0fe5548271967f1a67130b7105cd6a828e03909a67962e0ea1f61deb649f6bc3f4cef3800");
  BOOST_CHECK(!t1.IsStandard());

  // TX_NULL_DATA w/o PUSHDATA
  t1.vout.resize(1);
  t1.vout[0].scriptPubKey = CScript() << OP_RETURN;
  BOOST_CHECK(t1.IsStandard());

  // Only one TX_NULL_DATA permitted in all cases
  t1.vout.resize(2);
  t1.vout[0].scriptPubKey = CScript() << OP_RETURN << ParseHex("04678afdb0fe5548271967f1a67130b7105cd6a828e03909a67962e0ea1f61deb649f6bc3f4cef3804678afdb0fe5548271967f1a67130b7105cd6a828e03909a67962e0ea1f61deb649f6bc3f4cef38");
  t1.vout[1].scriptPubKey = CScript() << OP_RETURN << ParseHex("04678afdb0fe5548271967f1a67130b7105cd6a828e03909a67962e0ea1f61deb649f6bc3f4cef3804678afdb0fe5548271967f1a67130b7105cd6a828e03909a67962e0ea1f61deb649f6bc3f4cef38");
  BOOST_CHECK(!t1.IsStandard());
}

BOOST_AUTO_TEST_SUITE_END()
