#include "Utilities/Testing/interface/CppUnit_testdriver.icpp"
#include "cppunit/extensions/HelperMacros.h"

#include "EventFilter/StorageManager/interface/EnquingPolicyTag.h"
#include "EventFilter/StorageManager/interface/EventQueueCollection.h"
#include "EventFilter/StorageManager/interface/Exception.h"
#include "EventFilter/StorageManager/interface/I2OChain.h"
#include "EventFilter/StorageManager/interface/QueueID.h"

#include "EventFilter/StorageManager/test/TestHelper.h"

using stor::EventQueueCollection;
using stor::I2OChain;
using stor::QueueID;
using stor::testhelper::allocate_frame_with_sample_header;

class testEventQueueCollection : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(testEventQueueCollection);

  CPPUNIT_TEST(create_queues);
  CPPUNIT_TEST(pop_event_from_non_existing_queue);
  CPPUNIT_TEST(add_and_pop);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void create_queues();
  void pop_event_from_non_existing_queue();
  void add_and_pop();

private:
  // No data members yet.
};

void
testEventQueueCollection::setUp()
{ 
  CPPUNIT_ASSERT(g_factory);
  CPPUNIT_ASSERT(g_alloc);
  CPPUNIT_ASSERT(g_pool);
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
  CPPUNIT_ASSERT(c.size() == 1);
  CPPUNIT_ASSERT(id1.policy() == stor::enquing_policy::DiscardNew);
  CPPUNIT_ASSERT(id1.index() == 0);


  QueueID id2 = c.createQueue(stor::enquing_policy::DiscardOld, 20);
  CPPUNIT_ASSERT(c.size() == 2);
  CPPUNIT_ASSERT(id2.policy() == stor::enquing_policy::DiscardOld);
  CPPUNIT_ASSERT(id2.index() == 0);

  QueueID id3 = c.createQueue(stor::enquing_policy::DiscardOld, 20);
  CPPUNIT_ASSERT(c.size() == 3);
  CPPUNIT_ASSERT(id3.index() == 1);

  // Other policies should not allow creation
  QueueID id4 = c.createQueue(stor::enquing_policy::FailIfFull, 1);
  CPPUNIT_ASSERT(c.size() == 3);
  CPPUNIT_ASSERT(!id4.isValid());
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
  CPPUNIT_ASSERT_THROW(chain = c.popEvent(invalid_id), 
                       stor::exception::UnknownQueueId);
  CPPUNIT_ASSERT(chain.empty());
}

void
testEventQueueCollection::add_and_pop()
{
  EventQueueCollection coll;
  coll.setExpirationInterval(1.0);
  CPPUNIT_ASSERT(coll.getExpirationInterval() == 1.0);

  // Make some queues of each flavor, with very little capacity.  We want
  // them to fill rapidly.
  QueueID q1 = coll.createQueue(stor::enquing_policy::DiscardOld,  2);
  CPPUNIT_ASSERT(q1.isValid());
  QueueID q2 = coll.createQueue(stor::enquing_policy::DiscardNew, 2);
  CPPUNIT_ASSERT(q2.isValid());
  QueueID q3 = coll.createQueue(stor::enquing_policy::DiscardNew, 1);
  CPPUNIT_ASSERT(q3.isValid());

  // Make some chains, tagging them, and inserting them into the
  // collection.
  const int num_chains = 100;

  for (int i = 0; i != num_chains; ++i)
    {
      I2OChain event(allocate_frame_with_sample_header(0,1,1));
      event.tagForEventConsumer(q1);
      if (i % 2 == 0) event.tagForEventConsumer(q2);
      if (i % 3 == 0) event.tagForEventConsumer(q3);
      coll.addEvent(event);
    }
}

// This macro writes the 'main' for this test.
CPPUNIT_TEST_SUITE_REGISTRATION(testEventQueueCollection);

/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
