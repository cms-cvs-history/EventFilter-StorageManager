#include "Utilities/Testing/interface/CppUnit_testdriver.icpp"
#include "cppunit/extensions/HelperMacros.h"

#include <sstream>
#include <string>

#include "EventFilter/StorageManager/interface/SyncQueue.h"

/////////////////////////////////////////////////////////////
//
// This test exercises the for_each member template of SyncQueue.  It
// also makes use of the clear() member function.  Both of these are
// proposed additions to SynQueue, needed for use in StorageManager.
//
// Note that these tests do not test the reliability of the locking
// mechanism used by SyncQueue. We use the same locking mechanism used
// elsewhere in SyncQueue, additionally taking care to employ the
// ScopedLock pattern for exception safety.
//
/////////////////////////////////////////////////////////////

typedef stor::SyncQueue<int> queue_t;

// Demonstrate a function that modifies its input.
void add_one(int& i) { ++i; }

// Demonstrate a function that takes input by value.
void print_to_cout(int i) { std::cout << i << '\n'; }

// Demonstrate a functor class with a non-const operator().
class  Accumulate
{
public:
  Accumulate() : _sum(0) { }
  void operator()(int i) { _sum += i; }
  int sum() const { return _sum; }
private:
  int _sum;
};

// Demonstrate a functor class with non-void return value. Note that
// it must be safe to ignore the return value, because
// SyncQueue::for_each ignores the return value.  This functor also
// demonstrates taking its argument by reference-to-const. Finally, it
// illustrates a functor for which operator() is a const member
// function.
class PrintToStream
{
public:
  explicit PrintToStream(std::ostream& os = std::cout) : _stream(os) { }
  std::ostream& operator()(int const& i) const { _stream << i << '\n'; return _stream;}  
  std::ostream& stream() { return _stream; }
private:
  std::ostream& _stream;
};

class testSyncQueue : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(testSyncQueue);
  CPPUNIT_TEST(iterate_on_empty);
  CPPUNIT_TEST(iterate_on_nonempty);
  CPPUNIT_TEST(write_to_sstream);
  CPPUNIT_TEST(clear);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void iterate_on_empty();
  void iterate_on_nonempty();
  void write_to_sstream();
  void clear();

private:
  queue_t  _queue;
};

void
testSyncQueue::setUp()
{ 
  _queue.clear();
}


void
testSyncQueue::tearDown()
{
}

void 
testSyncQueue::iterate_on_empty()
{
  Accumulate acc = _queue.for_each(Accumulate());
  CPPUNIT_ASSERT(acc.sum() == 0);
}

void
testSyncQueue::iterate_on_nonempty()
{
  _queue.push(1);
  _queue.push(2);
  _queue.push(3);
  Accumulate acc = _queue.for_each(Accumulate());
  CPPUNIT_ASSERT(acc.sum() == 6);

  _queue.for_each(&add_one);
  Accumulate acc2 = _queue.for_each(Accumulate());
  CPPUNIT_ASSERT(acc2.sum() == 9);

  std::cout << '\n';
  _queue.for_each(&print_to_cout);
}

void
testSyncQueue::write_to_sstream()
{
  _queue.push(1);
  _queue.push(2);
  _queue.push(3);

  std::ostringstream out;
  PrintToStream printer(out);
  CPPUNIT_ASSERT(_queue.for_each(printer).stream()); // make sure the stream
					    // state is good.
  CPPUNIT_ASSERT(out.str() == std::string("1\n2\n3\n"));
}

void
testSyncQueue::clear()
{
  _queue.push(1);
  _queue.push(2);
  _queue.push(3);
  CPPUNIT_ASSERT(_queue.size() == 3);
  _queue.clear();
  CPPUNIT_ASSERT(_queue.size() == 0);  
}

// This macro writes the 'main' for this test.
CPPUNIT_TEST_SUITE_REGISTRATION(testSyncQueue);


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
