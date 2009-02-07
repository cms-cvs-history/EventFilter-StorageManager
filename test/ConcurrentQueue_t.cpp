#include "Utilities/Testing/interface/CppUnit_testdriver.icpp"
#include "cppunit/extensions/HelperMacros.h"

#include "EventFilter/StorageManager/interface/ConcurrentQueue.h"
#include "boost/thread.hpp"
#include "boost/shared_ptr.hpp"

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
      _sharedQueue->push_front(_counter);
    }
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
      if (_sharedQueue->pop_back(val)) ++_counter;
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
  CPPUNIT_TEST(push_and_pop);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void default_q_is_empty();
  void push_and_pop();

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
testConcurrentQueue::push_and_pop()
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


// This macro writes the 'main' for this test.
CPPUNIT_TEST_SUITE_REGISTRATION(testConcurrentQueue);


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
