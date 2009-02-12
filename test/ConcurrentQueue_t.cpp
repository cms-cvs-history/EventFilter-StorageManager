#include "Utilities/Testing/interface/CppUnit_testdriver.icpp"
#include "cppunit/extensions/HelperMacros.h"

#include "EventFilter/StorageManager/interface/ConcurrentQueue.h"
#include "FWCore/Utilities/interface/CPUTimer.h"
#include "boost/thread.hpp"
#include "boost/shared_ptr.hpp"
#include "boost/bind.hpp"

#include <math.h>

typedef stor::ConcurrentQueue<int> queue_t;

class FillQueue
{
public:
  FillQueue(boost::shared_ptr<queue_t>& p, 
	    unsigned int delay,
	    unsigned int nEntries):
    _sharedQueue(p),
    _delay(delay),
    _counter(nEntries+1)
  { }  

  void operator()();

  void waiting_fill();
  
private:
  boost::shared_ptr<queue_t> _sharedQueue;
  unsigned int               _delay;
  unsigned int               _counter;
};


void FillQueue::operator()()
{
  while(--_counter)
    {
      sleep(_delay);
      _sharedQueue->enq_nowait(_counter);
    }
}

void FillQueue::waiting_fill()
{
  while(--_counter) _sharedQueue->enq_wait(_counter);
}

class DrainQueue
{
public:
  DrainQueue(boost::shared_ptr<queue_t>& p, unsigned int delay) :
    _sharedQueue(p),
    _delay(delay),
    _counter(0)
  { }  

  void operator()();
  unsigned int count() const;

private:
  boost::shared_ptr<queue_t> _sharedQueue;
  unsigned int               _delay;
  unsigned int               _counter;
};

void DrainQueue::operator()()
{
  queue_t::value_type val;
  while(true)
    {
      sleep(_delay);
      if (_sharedQueue->deq_nowait(val)) ++_counter;
      else return;
    }
}

unsigned int DrainQueue::count() const
{
  return _counter;
}

class testConcurrentQueue : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(testConcurrentQueue);
  CPPUNIT_TEST(default_q_is_empty);
  CPPUNIT_TEST(queue_is_fifo);
  CPPUNIT_TEST(enq_and_deq);
  CPPUNIT_TEST(many_fillers);
  CPPUNIT_TEST(enq_timing);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void default_q_is_empty();
  void queue_is_fifo();
  void enq_and_deq();
  void many_fillers();

  void enq_timing();

private:
  // No data members yet.
};

void
testConcurrentQueue::setUp()
{ 
}

void
testConcurrentQueue::tearDown()
{ 
}

void 
testConcurrentQueue::default_q_is_empty()
{
  stor::ConcurrentQueue<int> q;
  CPPUNIT_ASSERT(q.empty());
}

void
testConcurrentQueue::queue_is_fifo()
{
  stor::ConcurrentQueue<int> q;
  q.enq_nowait(1);
  q.enq_nowait(2);
  q.enq_nowait(3);
  int value(0);
  CPPUNIT_ASSERT(q.deq_nowait(value));
  CPPUNIT_ASSERT(value == 1);
  CPPUNIT_ASSERT(q.deq_nowait(value));
  CPPUNIT_ASSERT(value == 2);
  CPPUNIT_ASSERT(q.deq_nowait(value));
  CPPUNIT_ASSERT(value == 3);
  CPPUNIT_ASSERT(q.empty());
}

void
testConcurrentQueue::enq_and_deq()
{
  boost::shared_ptr<queue_t> q(new queue_t);
  unsigned int delay = 0;
  unsigned int num_items = 10000;
  boost::thread producer(FillQueue(q, delay, num_items));
  boost::thread consumer(DrainQueue(q, delay));
  producer.join();
  sleep(1); // gross hack: give the consumer a chance to finish
  CPPUNIT_ASSERT(q->size() == 0);
}

void
testConcurrentQueue::many_fillers()
{
  boost::shared_ptr<queue_t> q(new queue_t(1000000));
  //size_t num_fillers(100);
  size_t num_fillers(3);
  unsigned int num_items(1000);
  
  boost::thread_group producers;
  for (unsigned int i = 0; i < num_fillers; ++i)
    {
      using boost::bind;
      using boost::thread;
      FillQueue last(q, 0, num_items);
      producers.add_thread(new thread(bind(&FillQueue::waiting_fill,
                                           &last)));
    }
  producers.join_all();
  CPPUNIT_ASSERT(q->size() == num_items * num_fillers);
}

void
testConcurrentQueue::enq_timing()
{
  queue_t q(1);

  // Queue is initially empty, so the first call should succeed.
  CPPUNIT_ASSERT(q.enq_nowait(1));
  CPPUNIT_ASSERT(q.size() == 1);
  CPPUNIT_ASSERT(q.capacity() == 1);

  // The queue is now full. The next enq should fail.
  edm::CPUTimer t;
  queue_t::value_type value;
  t.start();
  CPPUNIT_ASSERT(!q.enq_nowait(1));
  t.stop();
  // We somewhat arbitrarily choose 10 milliseconds as "immediately
  // enough".
  CPPUNIT_ASSERT(t.realTime() < 0.01);  

  // Now test the timeout version, with a range of timeouts.
  for (unsigned long wait_time = 0; wait_time < 3; ++wait_time)
    {
      t.reset();
      CPPUNIT_ASSERT(q.size() == 1);
      t.start();
      CPPUNIT_ASSERT(!q.enq_timed_wait(1, wait_time));
      t.stop();
      // We somewhat arbitrarily choose 10 milliseconds as "good enough
      // resolution".
      CPPUNIT_ASSERT(fabs(t.realTime()-wait_time) < 0.01);
    }

  // Now test the version that waits indefinitiely. We fill the queue,
  // start a draining thread that delays before eac deq, and then make
  // sure we do eventually return from the call to enq_wait.
  boost::shared_ptr<queue_t> qptr(new queue_t(1));
  CPPUNIT_ASSERT(qptr->capacity() == 1);
  CPPUNIT_ASSERT(qptr->enq_nowait(1));
  CPPUNIT_ASSERT(qptr->size() == 1);

  unsigned long delay = 2;
  boost::thread consumer(DrainQueue(qptr,2));

  qptr->enq_wait(2);
  consumer.join();
  CPPUNIT_ASSERT(qptr->empty());  
}

// This macro writes the 'main' for this test.
CPPUNIT_TEST_SUITE_REGISTRATION(testConcurrentQueue);


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
