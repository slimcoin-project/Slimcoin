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
    // uint256 p_15165 = uint256("0x0000017fba5ef709509c7380e3e128a69a5ab3c60b526c8345aff592dc8d8f81");
    // uint256 p_121300 = uint256("0x6f7f1b5cf35b3be443f747ffdacbc7faba860ec82535dcbe1d1486eed6eea131");
    uint256 p356299 = uint256("0x0000003a779a6620b17a0ee0c093b2ee8e5006572685246bd9e41ed95c99e22d");
    uint256 p747964 = uint256("0x000000410095e63c64bd3f25ae642464653ada088483f0bc7ed69a609dfc17a0");
    // should pass
  //should pass
    BOOST_CHECK(Checkpoints::CheckHardened(0, p0));
    BOOST_CHECK(Checkpoints::CheckHardened(356299, p356299));
    BOOST_CHECK(Checkpoints::CheckHardened(747964, p747964));
    
    
    // Wrong hashes at checkpoints should fail:
    BOOST_CHECK(!Checkpoints::CheckHardened(591298, p747964));
    BOOST_CHECK(!Checkpoints::CheckHardened(15936, p356299));

    // ... but any hash not at a checkpoint should succeed:
    BOOST_CHECK(Checkpoints::CheckHardened(591298+1, p747964));
    BOOST_CHECK(Checkpoints::CheckHardened(15936+1, p356299));

    BOOST_CHECK(Checkpoints::GetTotalBlocksEstimate() >= 904630); // as at 2017-03-28
}

BOOST_AUTO_TEST_SUITE_END()
