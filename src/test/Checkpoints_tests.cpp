//
// Unit tests for block-chain checkpoints
//
#include <boost/assign/list_of.hpp> // for 'map_list_of()'
#include <boost/test/unit_test.hpp>
#include <boost/foreach.hpp>

#include "../checkpoints.h"
#include "../util.h"

using namespace std;

BOOST_AUTO_TEST_SUITE(Checkpoints_tests)

BOOST_AUTO_TEST_CASE(sanity)
{
  
  uint256 p0 = uint256("0x0000070028a8a92117913b5ec2a4c6a457870f7aed0171af427ce50f42a6ae07");
  uint256 p11111 = uint256("0x0000000069e244f73d78e8fd29ba2fd2ed618bd6fa2ee92559f542fdb26e7c1d");
  uint256 p134444 = uint256("0x00000000000005b12ffd4cd315cd34ffd4a594f430ac814c91184a0d42d2b0fe");

  //should pass
  BOOST_CHECK(Checkpoints::CheckHardened(0, p0));
    
  // Wrong hashes at checkpoints should fail:
  BOOST_CHECK(!Checkpoints::CheckHardened(0, p134444));

  // ... but any hash not at a checkpoint should succeed:
  BOOST_CHECK(Checkpoints::CheckHardened(11111 + 1, p134444));
  BOOST_CHECK(Checkpoints::CheckHardened(134444 + 1, p11111));

  BOOST_CHECK(Checkpoints::GetTotalBlocksEstimate() >= 0);
}    

BOOST_AUTO_TEST_SUITE_END()
