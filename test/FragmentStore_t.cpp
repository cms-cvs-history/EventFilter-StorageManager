#include <iostream>

#include "Utilities/Testing/interface/CppUnit_testdriver.icpp"
#include "cppunit/extensions/HelperMacros.h"

#include "EventFilter/StorageManager/interface/I2OChain.h"
#include "EventFilter/StorageManager/interface/FragmentStore.h"

#include "EventFilter/StorageManager/test/TestHelper.h"


/////////////////////////////////////////////////////////////
//
// This test exercises the FragmentStore class
//
/////////////////////////////////////////////////////////////

using namespace stor;

using stor::testhelper::outstanding_bytes;
using stor::testhelper::allocate_frame_with_basic_header;

class testFragmentStore : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(testFragmentStore);
  CPPUNIT_TEST(testSingleFragment);
  CPPUNIT_TEST(testMultipleFragments);
  CPPUNIT_TEST(testConcurrentFragments);

  CPPUNIT_TEST_SUITE_END();

public:
  void testSingleFragment();
  void testMultipleFragments();
  void testConcurrentFragments();

private:
  stor::I2OChain getFragment
  (
    const unsigned int eventCount,
    const unsigned int fragmentCount,
    const unsigned int totalFragments
  );

  FragmentStore fragmentStore;

};

stor::I2OChain testFragmentStore::getFragment
(
  const unsigned int eventCount,
  const unsigned int fragmentCount,
  const unsigned int totalFragments
)
{
  unsigned int eventNumber = 0xb4b4e1e1;
  eventNumber <<= eventCount;

  Reference* ref = 
    allocate_frame_with_basic_header(I2O_SM_DATA, fragmentCount, totalFragments);
  I2O_SM_DATA_MESSAGE_FRAME *i2oMsg =
    (I2O_SM_DATA_MESSAGE_FRAME*) ref->getDataLocation();
  i2oMsg->rbBufferID = 2;
  i2oMsg->runID = 0xa5a5d2d2;
  i2oMsg->eventID = eventNumber;
  i2oMsg->outModID = 0xc3c3f0f0;
  i2oMsg->fuProcID = 0x01234567;
  i2oMsg->fuGUID = 0x89abcdef;
  stor::I2OChain frag(ref);
  
  return frag;
}

void testFragmentStore::testSingleFragment()
{
  CPPUNIT_ASSERT(outstanding_bytes() == 0);
  {
    stor::I2OChain frag = getFragment(1,0,1);
    CPPUNIT_ASSERT(frag.complete());
    bool complete = fragmentStore.addFragment(frag);
    CPPUNIT_ASSERT(complete);
    CPPUNIT_ASSERT(frag.complete());
    CPPUNIT_ASSERT(frag.fragmentCount() == 1);
  }
  CPPUNIT_ASSERT(outstanding_bytes() == 0);
}

void testFragmentStore::testMultipleFragments()
{
  CPPUNIT_ASSERT(outstanding_bytes() == 0);

  const unsigned int totalFragments = 3;

  for (unsigned int i = 0; i < totalFragments; ++i)
  {
    stor::I2OChain frag = getFragment(1,i,totalFragments);
    CPPUNIT_ASSERT(!frag.complete());
    bool complete = fragmentStore.addFragment(frag);
    if ( i < totalFragments-1 )
    {
      CPPUNIT_ASSERT(!complete);
      CPPUNIT_ASSERT(!frag.complete());
    }
    else
    {
      CPPUNIT_ASSERT(complete);
      CPPUNIT_ASSERT(frag.complete());
      CPPUNIT_ASSERT(!frag.faulty());
      CPPUNIT_ASSERT(frag.fragmentCount() == totalFragments);
    }
  }
  CPPUNIT_ASSERT(outstanding_bytes() == 0);
}


void testFragmentStore::testConcurrentFragments()
{
  CPPUNIT_ASSERT(outstanding_bytes() == 0);

  const unsigned int totalFragments = 3;
  const unsigned int totalEvents = 5;

  for (unsigned int fragNum = 0; fragNum < totalFragments; ++fragNum)
  {
    for (unsigned int eventNum = 0; eventNum < totalEvents; ++eventNum)
    {
      stor::I2OChain frag = getFragment(eventNum,fragNum,totalFragments);
      CPPUNIT_ASSERT(!frag.complete());
      bool complete = fragmentStore.addFragment(frag);
      if ( fragNum < totalFragments-1 )
      {
        CPPUNIT_ASSERT(!complete);
        CPPUNIT_ASSERT(!frag.complete());
      }
      else
      {
        CPPUNIT_ASSERT(complete);
        CPPUNIT_ASSERT(frag.complete());
        CPPUNIT_ASSERT(!frag.faulty());
        CPPUNIT_ASSERT(frag.fragmentCount() == totalFragments);
      }
    }
  }
  CPPUNIT_ASSERT(outstanding_bytes() == 0);
}




// This macro writes the 'main' for this test.
CPPUNIT_TEST_SUITE_REGISTRATION(testFragmentStore);


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
