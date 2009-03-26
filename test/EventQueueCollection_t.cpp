#include "Utilities/Testing/interface/CppUnit_testdriver.icpp"
#include "cppunit/extensions/HelperMacros.h"

#include "EventFilter/StorageManager/interface/EnquingPolicyTag.h"
#include "EventFilter/StorageManager/interface/EventQueueCollection.h"
#include "EventFilter/StorageManager/interface/Exception.h"
#include "EventFilter/StorageManager/interface/I2OChain.h"
#include "EventFilter/StorageManager/interface/QueueID.h"
#include "EventFilter/StorageManager/interface/Utils.h"

#include "EventFilter/StorageManager/test/TestHelper.h"

#include "boost/thread.hpp"
#include "boost/shared_ptr.hpp"
#include "boost/bind.hpp"

#include <algorithm>

using stor::EventQueueCollection;
using stor::I2OChain;
using stor::QueueID;
using stor::testhelper::allocate_frame_with_sample_header;
using stor::testhelper::outstanding_bytes;

using stor::enquing_policy::DiscardOld;
using stor::enquing_policy::DiscardNew;

using std::binary_search;
using std::sort;

class testEventQueueCollection : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(testEventQueueCollection);

  CPPUNIT_TEST(create_queues);
  CPPUNIT_TEST(pop_event_from_non_existing_queue);
  CPPUNIT_TEST(add_and_pop);
  CPPUNIT_TEST(invalid_queueid);
  CPPUNIT_TEST(clear_all_queues);
  CPPUNIT_TEST(remove_all_queues);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void create_queues();
  void pop_event_from_non_existing_queue();
  void add_and_pop();
  void invalid_queueid();
  void clear_all_queues();
  void remove_all_queues();

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
  QueueID id1 = c.createQueue(DiscardNew, 10);
  CPPUNIT_ASSERT(c.size() == 1);
  CPPUNIT_ASSERT(id1.policy() == DiscardNew);
  CPPUNIT_ASSERT(id1.index() == 0);


  QueueID id2 = c.createQueue(DiscardOld, 20);
  CPPUNIT_ASSERT(c.size() == 2);
  CPPUNIT_ASSERT(id2.policy() == DiscardOld);
  CPPUNIT_ASSERT(id2.index() == 0);

  QueueID id3 = c.createQueue(DiscardOld, 20);
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

void add_and_pop_helper(boost::shared_ptr<EventQueueCollection> pcoll);
void create_queues_helper(boost::shared_ptr<EventQueueCollection> pcoll);

void
testEventQueueCollection::add_and_pop()
{
  using namespace boost;
  shared_ptr<EventQueueCollection> pcoll(new EventQueueCollection);

  thread t1(bind(add_and_pop_helper, pcoll));
  thread t2(bind(create_queues_helper, pcoll));
  thread t3(bind(create_queues_helper, pcoll));
  thread t4(bind(create_queues_helper, pcoll));
  thread t5(bind(create_queues_helper, pcoll));

  t1.join();
  t2.join();
  t3.join();
  t4.join();
  t5.join();
}

void
create_queues_helper(boost::shared_ptr<EventQueueCollection> pcoll)
{
  for (int i = 0; i < 1000; ++i)
    {
      pcoll->createQueue(DiscardNew);
      pcoll->createQueue(DiscardOld);
      ::usleep(2000); // 1000 microseconds
    }
}

void
add_and_pop_helper(boost::shared_ptr<EventQueueCollection> pcoll)
{
  CPPUNIT_ASSERT(outstanding_bytes() == 0);
  EventQueueCollection& coll = *pcoll;
  // We want events to go bad very rapidly.
  double expiration_interval = 1.0;

  // Make some queues of each flavor, with very little capacity.  We want
  // them to fill rapidly.
  stor::utils::time_point_t now = stor::utils::getCurrentTime();

  QueueID q1 = coll.createQueue(DiscardOld, 2, expiration_interval, now);
  CPPUNIT_ASSERT(q1.isValid());
  QueueID q2 = coll.createQueue(DiscardNew, 2, expiration_interval, now);
  CPPUNIT_ASSERT(q2.isValid());
  QueueID q3 = coll.createQueue(DiscardNew, 1, expiration_interval, now);
  CPPUNIT_ASSERT(q3.isValid());

  // Make some chains, tagging them, and inserting them into the
  // collection. We use many more chains than we have slots in the
  // queues, to make sure that we don't block when the queues are full.
  const int num_chains = 100;

  for (int i = 0; i != num_chains; ++i)
    {
      I2OChain event(allocate_frame_with_sample_header(0,1,1));
      CPPUNIT_ASSERT(event.totalDataSize() != 0);
      unsigned char* payload = event.dataLocation(0);
      CPPUNIT_ASSERT(payload);
      payload[0] = i;      // event now carries an index as data.
      event.tagForEventConsumer(q1);
      if (i % 2 == 0) event.tagForEventConsumer(q2);
      if (i % 3 == 0) event.tagForEventConsumer(q3);
      coll.addEvent(event);
      CPPUNIT_ASSERT(outstanding_bytes() != 0);
    }
  // None of our queues should be empty; all should be full.
  CPPUNIT_ASSERT(!coll.empty(q1));
  CPPUNIT_ASSERT(coll.full(q1));

  CPPUNIT_ASSERT(!coll.empty(q2));
  CPPUNIT_ASSERT(coll.full(q2));

  CPPUNIT_ASSERT(!coll.empty(q3));
  CPPUNIT_ASSERT(coll.full(q3));

  // Queue with id q1 should contain "new" events; q2 and q3 should
  // contain "old" events.
  CPPUNIT_ASSERT(coll.popEvent(q1).dataLocation(0)[0] > num_chains/2);
  CPPUNIT_ASSERT(coll.popEvent(q2).dataLocation(0)[0] < num_chains/2);
  CPPUNIT_ASSERT(coll.popEvent(q3).dataLocation(0)[0] < num_chains/2);

  // Queues 1 and 2 should not be empty (because each contains one
  // event), but q3 should be empty (it has a capacity of one, and we
  // popped that one off).
  CPPUNIT_ASSERT(!coll.empty(q1));
  CPPUNIT_ASSERT(!coll.empty(q2));
  CPPUNIT_ASSERT(coll.empty(q3));
  
  // Now sleep for 1 second. Our queues should have all become stale;
  // they should also all be empty.
  ::sleep(1);
  std::vector<QueueID> stale_queues;
  coll.clearStaleQueues(stale_queues);
  //CPPUNIT_ASSERT(stale_queues.size() == coll.size());
  sort(stale_queues.begin(), stale_queues.end());
  CPPUNIT_ASSERT(binary_search(stale_queues.begin(), stale_queues.end(), q1));
  CPPUNIT_ASSERT(binary_search(stale_queues.begin(), stale_queues.end(), q2));
  CPPUNIT_ASSERT(binary_search(stale_queues.begin(), stale_queues.end(), q3));
  CPPUNIT_ASSERT(coll.empty(q1));
  CPPUNIT_ASSERT(coll.empty(q2));
  CPPUNIT_ASSERT(coll.empty(q3));

  CPPUNIT_ASSERT(outstanding_bytes() == 0);
}

void
testEventQueueCollection::invalid_queueid()
{
  CPPUNIT_ASSERT(outstanding_bytes() == 0);
  EventQueueCollection coll;
  // Make sure none of the interface functions cause a failure. Many
  // do not return any status we can test; we just run the function
  // and observe that we do *not* crash or throw any exception.
  QueueID id1(stor::enquing_policy::DiscardNew, 0);
  QueueID id2(stor::enquing_policy::DiscardOld, 0);

  coll.setExpirationInterval(id1, 2.0);
  coll.setExpirationInterval(id2, 2.0);

  CPPUNIT_ASSERT(coll.getExpirationInterval(id1) == 0.0);
  CPPUNIT_ASSERT(coll.getExpirationInterval(id2) == 0.0);

  {
    I2OChain event(allocate_frame_with_sample_header(0,1,1));
    event.tagForEventConsumer(id1);
    event.tagForEventConsumer(id2);
    CPPUNIT_ASSERT(!event.empty());
    coll.addEvent(event);
    CPPUNIT_ASSERT(outstanding_bytes() != 0);
  }
  // Trying to pop an event off an nonexistent queue should give an
  // empty event.
  I2OChain event;
  CPPUNIT_ASSERT(event.empty());
  event = coll.popEvent(id1);
  CPPUNIT_ASSERT(event.empty());
  event = coll.popEvent(id2);
  CPPUNIT_ASSERT(event.empty());

  coll.clearQueue(id1); // should have no effect
  coll.clearQueue(id2); // should have no effect
  CPPUNIT_ASSERT(coll.empty(id1)); // nonexistent queue is empty.
  CPPUNIT_ASSERT(coll.full(id1));  // nonexistent queue is also full.
  CPPUNIT_ASSERT(coll.empty(id2)); // nonexistent queue is empty.
  CPPUNIT_ASSERT(coll.full(id2));  // nonexistent queue is also full.

  
  std::vector<QueueID> stale_queues;
  coll.clearStaleQueues(stale_queues);
  CPPUNIT_ASSERT(stale_queues.empty());
  CPPUNIT_ASSERT(outstanding_bytes() == 0);
}

void
testEventQueueCollection::clear_all_queues()
{
    CPPUNIT_ASSERT(outstanding_bytes() == 0);
  EventQueueCollection coll;
  QueueID q1 = coll.createQueue(DiscardNew);
  QueueID q2 = coll.createQueue(DiscardOld);
  QueueID q3 = coll.createQueue(DiscardOld);
  QueueID q4 = coll.createQueue(DiscardNew);
  CPPUNIT_ASSERT(coll.size() == 4);
  
  for (int i = 0; i < 100; ++i)
    {
      I2OChain event(allocate_frame_with_sample_header(0,1,1));
      event.tagForEventConsumer(q1);
      if (i%2 == 0) event.tagForEventConsumer(q2);
      if (i%3 == 0) event.tagForEventConsumer(q3);
      if (i%4 == 0) event.tagForEventConsumer(q4);
      coll.addEvent(event);
      CPPUNIT_ASSERT(outstanding_bytes() != 0);
    }
  CPPUNIT_ASSERT(!coll.empty(q1));
  CPPUNIT_ASSERT(!coll.empty(q2));
  CPPUNIT_ASSERT(!coll.empty(q3));
  CPPUNIT_ASSERT(!coll.empty(q4));
  
  coll.clearQueues();
  CPPUNIT_ASSERT(coll.size() == 4);
  CPPUNIT_ASSERT(coll.empty(q1));
  CPPUNIT_ASSERT(coll.empty(q2));
  CPPUNIT_ASSERT(coll.empty(q3));
  CPPUNIT_ASSERT(coll.empty(q4));
    CPPUNIT_ASSERT(outstanding_bytes() == 0);
}

void
testEventQueueCollection::remove_all_queues()
{
  EventQueueCollection coll;
  QueueID q1 = coll.createQueue(DiscardNew);
  QueueID q2 = coll.createQueue(DiscardOld);
  QueueID q3 = coll.createQueue(DiscardOld);
  QueueID q4 = coll.createQueue(DiscardNew);

  CPPUNIT_ASSERT(coll.size() == 4);
  coll.removeQueues();
  CPPUNIT_ASSERT(coll.size() == 0);  
}


// This macro writes the 'main' for this test.
CPPUNIT_TEST_SUITE_REGISTRATION(testEventQueueCollection);

/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
