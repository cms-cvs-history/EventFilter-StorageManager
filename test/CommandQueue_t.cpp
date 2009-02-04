#include "Utilities/Testing/interface/CppUnit_testdriver.icpp"
#include "cppunit/extensions/HelperMacros.h"

#include "EventFilter/StorageManager/interface/CommandQueue.h"
#include "EventFilter/StorageManager/interface/StateMachine.h"


class testCommandQueue : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(testCommandQueue);
  CPPUNIT_TEST(default_q_is_empty);
  CPPUNIT_TEST(matched_pops_and_pushes);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void default_q_is_empty();
  void matched_pops_and_pushes();

private:
  // No data members yet.
};

void
testCommandQueue::setUp()
{ 
}

void
testCommandQueue::tearDown()
{ 
}


void 
testCommandQueue::default_q_is_empty()
{
  stor::CommandQueue q;
  CPPUNIT_ASSERT(q.empty());
  CPPUNIT_ASSERT(q.size() ==0);  
}

void
testCommandQueue::matched_pops_and_pushes()
{
  typedef boost::shared_ptr<boost::statechart::event_base> event_ptr;
  stor::CommandQueue q;
  q.push_front(event_ptr(new stor::Configure));
  CPPUNIT_ASSERT(q.size() == 1);
  q.push_front(event_ptr(new stor::Enable));
  q.push_front(event_ptr(new stor::EmergencyStop));
  CPPUNIT_ASSERT(q.size() == 3);
//   CPPUNIT_ASSERT(q.pop_back()->stateName() == "Configure");
//   CPPUNIT_ASSERT(q.pop_back()->stateName() == "Enable");
//   CPPUNIT_ASSERT(q.pop_back()->stateName() == "EmergencyStop");
  CPPUNIT_ASSERT(q.empty());
}


// This macro writes the 'main' for this test.
CPPUNIT_TEST_SUITE_REGISTRATION(testCommandQueue);


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
