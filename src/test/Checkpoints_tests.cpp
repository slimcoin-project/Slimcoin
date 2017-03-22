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

  //should pass
  BOOST_CHECK(Checkpoints::CheckHardened(0, p0));
    
    
    // Wrong hashes at checkpoints should fail:
    BOOST_CHECK(!Checkpoints::CheckHardened(19080, p99999));
    BOOST_CHECK(!Checkpoints::CheckHardened(99999, p19080));

    // ... but any hash not at a checkpoint should succeed:
    BOOST_CHECK(Checkpoints::CheckHardened(19080+1, p99999));
    BOOST_CHECK(Checkpoints::CheckHardened(99999+1, p19080));

    BOOST_CHECK(Checkpoints::GetTotalBlocksEstimate() >= 99999);
}

BOOST_AUTO_TEST_SUITE_END()
