#include "Utilities/Testing/interface/CppUnit_testdriver.icpp"
#include "cppunit/extensions/HelperMacros.h"

#include "EventFilter/StorageManager/interface/ExpirableEventQueue.h"
#include "EventFilter/StorageManager/interface/I2OChain.h"
#include "EventFilter/StorageManager/interface/Utils.h"

#include "EventFilter/StorageManager/test/TestHelper.h"

#include <cstdlib> // for size_t

typedef stor::ExpirableEventQueueDiscardNew DN_q_t;
typedef stor::ExpirableEventQueueDiscardOld DO_q_t;

using stor::utils::duration_t;
using stor::testhelper::allocate_frame_with_sample_header;

using namespace stor;

//  -------------------------------------------------------------------
//  The following function templates are used in the tests below, to
//  assure uniformity in the testing of both instantiations of
//  ExpirableEventQueue.
//  -------------------------------------------------------------------

template <class Q>
void
test_default()
{
  // Default constructed queues are empty.
  Q q;
  CPPUNIT_ASSERT(q.empty());
  CPPUNIT_ASSERT(!q.full());

  // Clearing the queue should have no effect.
  q.clear();
  CPPUNIT_ASSERT(q.empty());
  CPPUNIT_ASSERT(!q.full());
}

template <class Q>
void
test_fill_and_go_stale()
{
  // Make a queue with a short time-to-staleness and small capacity,
  // so we can see it fill up.
  size_t capacity(10);
  duration_t seconds_to_stale(2.0);
  Q q(capacity, seconds_to_stale);
  CPPUNIT_ASSERT(!q.full());

  // Pump in enough events to fill the queue.
  for (size_t i = 0; i < capacity; ++i)
    {
      I2OChain event(allocate_frame_with_sample_header(0,1,1));
      q.enq_nowait(event);
    }
  CPPUNIT_ASSERT(q.full());

  // An immediate call to clearIfStale() should do nothing.
  CPPUNIT_ASSERT(!q.clearIfStale());
  CPPUNIT_ASSERT(q.full());

  // After waiting for our staleness interval, the queue should still
  // be full. But then a call to clearIfStale should clear the queue.
  ::sleep(static_cast<int>(seconds_to_stale));

  CPPUNIT_ASSERT(q.full());
  CPPUNIT_ASSERT(!q.empty());

  CPPUNIT_ASSERT(q.clearIfStale());

  CPPUNIT_ASSERT(!q.full());
  CPPUNIT_ASSERT(q.empty());
}

template <class Q>
void
test_pop_freshens_queue()
{
  size_t capacity(2);
  duration_t seconds_to_stale(1.0);
  Q q(capacity, seconds_to_stale);

  // Push in an event, then let the queue go stale.
  q.enq_nowait(I2OChain(allocate_frame_with_sample_header(0,1,1)));
  CPPUNIT_ASSERT(!q.empty());
  ::sleep(static_cast<int>(seconds_to_stale));

  // Verify the queue has gone stale.
  CPPUNIT_ASSERT(q.clearIfStale());
  CPPUNIT_ASSERT(q.empty());

  // A queue that has gone stale should remain stale if we add a new
  // event to it.
  q.enq_nowait(I2OChain(allocate_frame_with_sample_header(0,1,1)));
  CPPUNIT_ASSERT(!q.empty());
  CPPUNIT_ASSERT(q.clearIfStale());
  CPPUNIT_ASSERT(q.empty());

  // Popping from the queue should make it non-stale. This must be
  // true *even if the queue is empty*, so that popping does not get
  // an event.
   I2OChain popped;
   CPPUNIT_ASSERT(!q.deq_nowait(popped));
   CPPUNIT_ASSERT(q.empty());
   CPPUNIT_ASSERT(!q.clearIfStale());
}

//  -------------------------------------------------------------------

class testExpirableEventQueue : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(testExpirableEventQueue);

  CPPUNIT_TEST(default_queue);
  CPPUNIT_TEST(fill_and_go_stale);
  CPPUNIT_TEST(pop_freshens_queue);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void default_queue();
  void fill_and_go_stale();
  void pop_freshens_queue();

private:

};

void
testExpirableEventQueue::setUp()
{
  CPPUNIT_ASSERT(g_factory);
  CPPUNIT_ASSERT(g_alloc);
  CPPUNIT_ASSERT(g_pool);
}

void
testExpirableEventQueue::tearDown()
{ 
}

void 
testExpirableEventQueue::default_queue()
{
  test_default<DN_q_t>();
  test_default<DO_q_t>();
}

void
testExpirableEventQueue::fill_and_go_stale()
{
  test_fill_and_go_stale<DN_q_t>();
  test_fill_and_go_stale<DO_q_t>();
}

void
testExpirableEventQueue::pop_freshens_queue()
{
  test_pop_freshens_queue<DN_q_t>();
  test_pop_freshens_queue<DO_q_t>();
}

// This macro writes the 'main' for this test.
CPPUNIT_TEST_SUITE_REGISTRATION(testExpirableEventQueue);


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -