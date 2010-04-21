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
using stor::testhelper::allocate_multiple_frames_with_event_msg;
using toolbox::mem::Reference;

class testFragmentStore : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(testFragmentStore);
  CPPUNIT_TEST(testSingleFragment);
  CPPUNIT_TEST(testMultipleFragments);
  CPPUNIT_TEST(testConcurrentFragments);
  CPPUNIT_TEST(testStaleEvent);
  CPPUNIT_TEST(testSingleIncompleteEvent);
  CPPUNIT_TEST(testOneOfManyIncompleteEvent);
  CPPUNIT_TEST(testClear);

  CPPUNIT_TEST_SUITE_END();

public:
  void testSingleFragment();
  void testMultipleFragments();
  void testConcurrentFragments();
  void testStaleEvent();
  void testSingleIncompleteEvent();
  void testOneOfManyIncompleteEvent();
  void testClear();

private:
  unsigned int getEventNumber
  (
    const unsigned int eventCount
  ) const;

  typedef std::vector<stor::I2OChain> Fragments;
  Fragments getFragments(
    const unsigned int eventNumber,
    const unsigned int totalFragments
  ) const;

  FragmentStore fragmentStore;
};

unsigned int testFragmentStore::getEventNumber
(
  const unsigned int eventCount
) const
{
  unsigned int eventNumber = 0xb4b4e1e1;
  eventNumber <<= eventCount;
  return eventNumber;
}


testFragmentStore::Fragments testFragmentStore::getFragments(
  const unsigned int eventNumber,
  const unsigned int totalFragments
) const
{
  Fragments fragments;
  testhelper::References refs =
    allocate_multiple_frames_with_event_msg(getEventNumber(eventNumber),totalFragments);
  for (testhelper::References::const_iterator it = refs.begin(), itEnd = refs.end();
       it != itEnd; ++it)
  {
    fragments.push_back(stor::I2OChain(*it));
  }
  return fragments;
}


void testFragmentStore::testSingleFragment()
{
  CPPUNIT_ASSERT(outstanding_bytes() == 0);
  {
    stor::I2OChain frag = getFragments(1,1).front();
    CPPUNIT_ASSERT(frag.complete());
    CPPUNIT_ASSERT(!frag.faulty());
    bool complete = fragmentStore.addFragment(frag);
    CPPUNIT_ASSERT(complete);
    CPPUNIT_ASSERT(frag.complete());
    CPPUNIT_ASSERT(!frag.faulty());
    CPPUNIT_ASSERT(frag.fragmentCount() == 1);
  }
  CPPUNIT_ASSERT(outstanding_bytes() == 0);
}

void testFragmentStore::testMultipleFragments()
{
  CPPUNIT_ASSERT(outstanding_bytes() == 0);

  {
    const unsigned int totalFragments = 3;
    Fragments fragments = getFragments(1,totalFragments);
    
    for (unsigned int i = 0; i < totalFragments; ++i)
    {
      stor::I2OChain frag = fragments[i];
      CPPUNIT_ASSERT(!frag.complete());
      bool complete = fragmentStore.addFragment(frag);
      if ( i < totalFragments-1 )
      {
        CPPUNIT_ASSERT(!complete);
        CPPUNIT_ASSERT(frag.empty());
      }
      else
      {
        CPPUNIT_ASSERT(complete);
        CPPUNIT_ASSERT(!frag.empty());
        CPPUNIT_ASSERT(frag.complete());
        CPPUNIT_ASSERT(!frag.faulty());
        CPPUNIT_ASSERT(frag.fragmentCount() == totalFragments);
      }
    }
  }
  
  CPPUNIT_ASSERT(outstanding_bytes() == 0);
}


void testFragmentStore::testConcurrentFragments()
{
  CPPUNIT_ASSERT(outstanding_bytes() == 0);

  {
    const unsigned int totalFragments = 5;
    const unsigned int totalEvents = 15;
    std::vector<Fragments> allFragments;
    
    for (unsigned int eventNum = 0; eventNum < totalEvents; ++eventNum)
    {
      allFragments.push_back( getFragments(eventNum,totalFragments) );
    }
    
    for (unsigned int fragNum = 0; fragNum < totalFragments; ++fragNum)
    {
      for (unsigned int eventNum = 0; eventNum < totalEvents; ++eventNum)
      {
        stor::I2OChain frag = allFragments[eventNum][fragNum];
        CPPUNIT_ASSERT(!frag.complete());
        bool complete = fragmentStore.addFragment(frag);
        if ( fragNum < totalFragments-1 )
        {
          CPPUNIT_ASSERT(!complete);
          CPPUNIT_ASSERT(frag.empty());
        }
        else
        {
          CPPUNIT_ASSERT(complete);
          CPPUNIT_ASSERT(frag.complete());
          CPPUNIT_ASSERT(!frag.faulty());
          CPPUNIT_ASSERT(frag.fragmentCount() == totalFragments);
          
          stor::FragKey fragmentKey = frag.fragmentKey();
          CPPUNIT_ASSERT(fragmentKey.event_ == getEventNumber(eventNum));
        }
      }
    }
  }
  CPPUNIT_ASSERT(outstanding_bytes() == 0);
}

void testFragmentStore::testStaleEvent()
{
  CPPUNIT_ASSERT(outstanding_bytes() == 0);
  {
    stor::I2OChain frag = getFragments(1,1).front();
    CPPUNIT_ASSERT(frag.complete());
    bool hasStaleEvent = fragmentStore.getStaleEvent(frag, 0);
    CPPUNIT_ASSERT(hasStaleEvent == false);
    CPPUNIT_ASSERT(frag.empty());
  }
  CPPUNIT_ASSERT(outstanding_bytes() == 0);
}

void testFragmentStore::testSingleIncompleteEvent()
{
  CPPUNIT_ASSERT(outstanding_bytes() == 0);
  {
    const unsigned int totalFragments = 5;
    stor::I2OChain frag;
    bool complete;
    Fragments fragments = getFragments(1, totalFragments);

    for (unsigned int i = 0; i < totalFragments-1; ++i)
    {
      frag = fragments[i];
      CPPUNIT_ASSERT(!frag.complete());
      complete = fragmentStore.addFragment(frag);
    }
    CPPUNIT_ASSERT(frag.empty());
    
    CPPUNIT_ASSERT(fragmentStore.getStaleEvent(frag, 0) == true);
    CPPUNIT_ASSERT(!frag.complete());
    CPPUNIT_ASSERT(frag.faulty());
    CPPUNIT_ASSERT(frag.fragmentCount() == totalFragments-1);
    
    stor::FragKey fragmentKey = frag.fragmentKey();
    CPPUNIT_ASSERT(fragmentKey.event_ == getEventNumber(1));
  }
  CPPUNIT_ASSERT(outstanding_bytes() == 0);
}


void testFragmentStore::testOneOfManyIncompleteEvent()
{
  CPPUNIT_ASSERT(outstanding_bytes() == 0);
  {
    const unsigned int totalFragments = 5;
    const unsigned int totalEvents = 15;
    stor::I2OChain frag;
    bool complete;
    std::vector<Fragments> allFragments;
    allFragments.push_back(getFragments(0,totalFragments));

    // Fill one event with a missing fragment
    for (unsigned int fragNum = 0; fragNum < totalFragments-1; ++fragNum)
    {
      frag = allFragments[0][fragNum];
      CPPUNIT_ASSERT(!frag.complete());
      complete = fragmentStore.addFragment(frag);
    }
    CPPUNIT_ASSERT(!complete);
    CPPUNIT_ASSERT(frag.empty());
    
    // Sleep for a second
    sleep(1);
    
    // Fill more events with missing fragments
    for (unsigned int eventNum = 1; eventNum < totalEvents; ++eventNum)
    {
      allFragments.push_back(getFragments(eventNum,totalFragments));

      for (unsigned int fragNum = 1; fragNum < totalFragments; ++fragNum)
      {
        frag = allFragments[eventNum][fragNum];
        CPPUNIT_ASSERT(!frag.complete());
        complete = fragmentStore.addFragment(frag);
      }
      CPPUNIT_ASSERT(!complete);
      CPPUNIT_ASSERT(frag.empty());
    }

    // Get the first event which should be stale by now
    CPPUNIT_ASSERT(fragmentStore.getStaleEvent(frag, 1) == true);
    CPPUNIT_ASSERT(!frag.complete());
    CPPUNIT_ASSERT(frag.faulty());
    CPPUNIT_ASSERT(frag.fragmentCount() == totalFragments-1);
    
    stor::FragKey fragmentKey = frag.fragmentKey();
    CPPUNIT_ASSERT(fragmentKey.event_ == getEventNumber(0));
    
    // Finish the other events
    for (unsigned int eventNum = 1; eventNum < totalEvents; ++eventNum)
    {
      frag = allFragments[eventNum][0];
      CPPUNIT_ASSERT(!frag.complete());
      complete = fragmentStore.addFragment(frag);
      CPPUNIT_ASSERT(complete);
      CPPUNIT_ASSERT(frag.complete());
      CPPUNIT_ASSERT(!frag.faulty());
      CPPUNIT_ASSERT(frag.fragmentCount() == totalFragments);
    
      stor::FragKey fragmentKey = frag.fragmentKey();
      CPPUNIT_ASSERT(fragmentKey.event_ == getEventNumber(eventNum));
    }
    CPPUNIT_ASSERT(fragmentStore.getStaleEvent(frag, 0) == false);
    CPPUNIT_ASSERT(frag.empty());
  }
  CPPUNIT_ASSERT(outstanding_bytes() == 0);
}


void testFragmentStore::testClear()
{
  CPPUNIT_ASSERT(outstanding_bytes() == 0);
  {
    const unsigned int totalFragments = 5;
    const unsigned int totalEvents = 15;
    stor::I2OChain frag;
    bool complete;
    std::vector<Fragments> allFragments;
    
    for (unsigned int eventNum = 0; eventNum < totalEvents; ++eventNum)
    {
      Fragments fragments = getFragments(eventNum,totalFragments);

      for (unsigned int fragNum = 0; fragNum < totalFragments-1; ++fragNum)
      {
        frag = fragments[fragNum];
        CPPUNIT_ASSERT(!frag.complete());
        complete = fragmentStore.addFragment(frag);
      }
      CPPUNIT_ASSERT(!complete);
      CPPUNIT_ASSERT(frag.empty());
    }
    CPPUNIT_ASSERT(outstanding_bytes() != 0);
    fragmentStore.clear();
    CPPUNIT_ASSERT(outstanding_bytes() == 0);
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
