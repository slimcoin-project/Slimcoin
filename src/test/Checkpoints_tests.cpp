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
    uint256 p15165 = uint256("0x0000017fba5ef709509c7380e3e128a69a5ab3c60b526c8345aff592dc8d8f81");
    uint256 p121300 = uint256("0x6f7f1b5cf35b3be443f747ffdacbc7faba860ec82535dcbe1d1486eed6eea131");

    // should pass
  //should pass
    BOOST_CHECK(Checkpoints::CheckHardened(0, p0));
    BOOST_CHECK(Checkpoints::CheckHardened(15165, p15165));
    BOOST_CHECK(Checkpoints::CheckHardened(121300, p121300));
    
    
    // Wrong hashes at checkpoints should fail:
    BOOST_CHECK(!Checkpoints::CheckHardened(19080, p121300));
    BOOST_CHECK(!Checkpoints::CheckHardened(99999, p15165));

    // ... but any hash not at a checkpoint should succeed:
    BOOST_CHECK(Checkpoints::CheckHardened(19080+1, p121300));
    BOOST_CHECK(Checkpoints::CheckHardened(99999+1, p15165));

    BOOST_CHECK(Checkpoints::GetTotalBlocksEstimate() >= 121299);
}

BOOST_AUTO_TEST_SUITE_END()
