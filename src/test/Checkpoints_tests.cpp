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
    uint256 p0 = uint256("0x00000766be5a4bb74c040b85a98d2ba2b433c5f4c673912b3331ea6f18d61bea");
    uint256 p11111 = uint256("0x00000018d3973f42cefe4d9222e7c9721851b491d5db658f0dee9e0a018d2123");
    uint256 p134444 = uint256("0x000000035fed0f108336d1ff15ea23c4d2708369376750825a0b59667eb99bd2");

    // should pass
  //should pass
    BOOST_CHECK(Checkpoints::CheckHardened(0, p0));
    BOOST_CHECK(Checkpoints::CheckHardened(11111, p11111));
    BOOST_CHECK(Checkpoints::CheckHardened(134444, p134444));
    
    
    // Wrong hashes at checkpoints should fail:
    BOOST_CHECK(!Checkpoints::CheckHardened(19080, p134444));
    BOOST_CHECK(!Checkpoints::CheckHardened(99999, p11111));

    // ... but any hash not at a checkpoint should succeed:
    BOOST_CHECK(Checkpoints::CheckHardened(19080+1, p134444));
    BOOST_CHECK(Checkpoints::CheckHardened(99999+1, p11111));

    BOOST_CHECK(Checkpoints::GetTotalBlocksEstimate() >= 134444);
}

BOOST_AUTO_TEST_SUITE_END()
