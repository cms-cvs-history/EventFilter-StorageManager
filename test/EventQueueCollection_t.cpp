#include "Utilities/Testing/interface/CppUnit_testdriver.icpp"
#include "cppunit/extensions/HelperMacros.h"

#include "EventFilter/StorageManager/interface/EventQueueCollection.h"
#include "EventFilter/StorageManager/interface/Exception.h"
#include "EventFilter/StorageManager/interface/I2OChain.h"
#include "EventFilter/StorageManager/interface/QueueID.h"
#include "EventFilter/StorageManager/interface/EnquingPolicyTag.h"

using stor::EventQueueCollection;
using stor::I2OChain;
using stor::QueueID;

class testEventQueueCollection : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(testEventQueueCollection);

  CPPUNIT_TEST(create_queues);
  CPPUNIT_TEST(pop_event_from_non_existing_queue);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void create_queues();
  void pop_event_from_non_existing_queue();

private:
  // No data members yet.
};

void
testEventQueueCollection::setUp()
{ 
}

void
testEventQueueCollection::tearDown()
{ 
}

void 
testEventQueueCollection::create_queues()
{
  // Default collection should have no queues.
  EventQueueCollection c;
  CPPUNIT_ASSERT(c.size() == 0);

  // Make sure that the different types of queue are both counted
  // correctly.
  QueueID id1 = c.createQueue(stor::enquing_policy::DiscardNew, 10);
  CPPUNIT_ASSERT(id1.policy() == stor::enquing_policy::DiscardNew);
  CPPUNIT_ASSERT(c.size() == 1);

  QueueID id2 = c.createQueue(stor::enquing_policy::DiscardOld, 20);
  CPPUNIT_ASSERT(id2.policy() == stor::enquing_policy::DiscardOld);
  CPPUNIT_ASSERT(c.size() == 2);
  QueueID id3 = c.createQueue(stor::enquing_policy::DiscardOld, 20);
  CPPUNIT_ASSERT(c.size() == 3);

  // Other policies should not allow creation
  QueueID id4 = c.createQueue(stor::enquing_policy::FailIfFull, 1);
  CPPUNIT_ASSERT(!id4.isValid());
  CPPUNIT_ASSERT(c.size() == 3);
}

void 
testEventQueueCollection::pop_event_from_non_existing_queue()
{
  // Attemping to pop and event from a non-existent queue should
  // result in an exception.
  EventQueueCollection c;
  CPPUNIT_ASSERT(c.size() == 0);
  QueueID invalid_id;
  CPPUNIT_ASSERT(!invalid_id.isValid());

  I2OChain chain;
  CPPUNIT_ASSERT(chain.empty());
  CPPUNIT_ASSERT_THROW(chain = c.popEvent(invalid_id), stor::exception::UnknownQueueId);
  CPPUNIT_ASSERT(chain.empty());
}

// This macro writes the 'main' for this test.
CPPUNIT_TEST_SUITE_REGISTRATION(testEventQueueCollection);

/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
