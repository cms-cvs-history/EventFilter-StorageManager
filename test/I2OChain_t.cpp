#include "Utilities/Testing/interface/CppUnit_testdriver.icpp"
#include "cppunit/extensions/HelperMacros.h"

#include <assert.h>
#include <vector>

#include "toolbox/mem/HeapAllocator.h"
#include "toolbox/mem/Reference.h"
#include "toolbox/mem/MemoryPoolFactory.h"
#include "toolbox/mem/exception/Exception.h"

#include "EventFilter/StorageManager/interface/I2OChain.h"

using toolbox::mem::Pool;
using toolbox::mem::MemoryPoolFactory;
using toolbox::mem::getMemoryPoolFactory;
using toolbox::mem::HeapAllocator;
using toolbox::mem::Reference;


// There seems to be no sane way to create and destroy some of these
// toolbox entities, so we use globals. valgrind complains about
// leaked resources, but attempts to clean up resources (and to not
// use global resources) does not remove the leaks.
namespace
{
  MemoryPoolFactory*  g_factory(getMemoryPoolFactory());
  toolbox::net::URN   g_urn("toolbox-mem-pool","myPool");
  HeapAllocator* g_alloc(new HeapAllocator);
  Pool*          g_pool(g_factory->createPool(g_urn, g_alloc));
}

class testI2OChain : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(testI2OChain);
  CPPUNIT_TEST(default_chain);
  CPPUNIT_TEST(nonempty_chain_cleans_up_nice);
  CPPUNIT_TEST(copy_chain);
  CPPUNIT_TEST(copying_does_not_exhaust_buffer);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void default_chain();
  void nonempty_chain_cleans_up_nice();
  void copy_chain();
  void copying_does_not_exhaust_buffer();

private:
  // Allocate a new frame from the (global) Pool.
  Reference* allocate_frame();

  // Return the number of bytes currently allocated out of the
  // (global) Pool.
  size_t outstanding_bytes();
};

void
testI2OChain::setUp()
{ 
  CPPUNIT_ASSERT(g_factory);
  CPPUNIT_ASSERT(g_alloc);
  CPPUNIT_ASSERT(g_pool);
}

void
testI2OChain::tearDown()
{ 
}

void 
testI2OChain::default_chain()
{
  stor::I2OChain frag;
  CPPUNIT_ASSERT(frag.empty());
  CPPUNIT_ASSERT(!frag.complete());
}

void
testI2OChain::nonempty_chain_cleans_up_nice()
{
  CPPUNIT_ASSERT(outstanding_bytes() == 0);
  {
    stor::I2OChain frag(allocate_frame());
    CPPUNIT_ASSERT(outstanding_bytes() != 0);
    CPPUNIT_ASSERT(!frag.empty());
    CPPUNIT_ASSERT(!frag.complete());
    frag.markComplete();
    CPPUNIT_ASSERT(frag.complete());
  }
  CPPUNIT_ASSERT(outstanding_bytes() == 0);
}


void
testI2OChain::copy_chain()
{
  CPPUNIT_ASSERT(outstanding_bytes() == 0);
  {
    stor::I2OChain frag(allocate_frame());
    size_t memory_consumed_by_one_frame = outstanding_bytes();
    CPPUNIT_ASSERT(memory_consumed_by_one_frame != 0);
    {
      stor::I2OChain copy(frag);
      size_t memory_consumed_after_copy = outstanding_bytes();
      CPPUNIT_ASSERT(memory_consumed_after_copy != 0);
      CPPUNIT_ASSERT(memory_consumed_after_copy ==
                     memory_consumed_by_one_frame);
      CPPUNIT_ASSERT(copy.getBufferData() == frag.getBufferData());
    }
    // Here the copy is gone, but the original remains; we should not
    // have released the resources.
    CPPUNIT_ASSERT(outstanding_bytes() != 0);
  }
  CPPUNIT_ASSERT(outstanding_bytes() == 0);
}

void
testI2OChain::copying_does_not_exhaust_buffer()
{
  stor::I2OChain frag(allocate_frame());
  size_t memory_consumed_by_one_frame = outstanding_bytes();
  CPPUNIT_ASSERT(memory_consumed_by_one_frame != 0);

  // Now make many copies.
  size_t copies_to_make = 1000*1000UL;
  std::vector<stor::I2OChain> copies(copies_to_make, frag);
  CPPUNIT_ASSERT(copies.size() == copies_to_make);

  // Make sure we haven't consumed any more buffer space...
  CPPUNIT_ASSERT(outstanding_bytes() ==
                 memory_consumed_by_one_frame);

  // Make sure they all manage the identical buffer...  Using
  // std::find_if would be briefer, but more obscure to many
  // maintainers. If you disagree, please replace this look with the
  // appropriate predicate (using lambda) and use of find_if.
  unsigned long* expected_data_location = frag.getBufferData();
  for (std::vector<stor::I2OChain>::iterator 
         i = copies.begin(),
         e = copies.end();
       i != e;
       ++i)
    {
      CPPUNIT_ASSERT( i->getBufferData() == expected_data_location);
    }
         
  
  // Now destroy the copies.
  copies.clear();
  CPPUNIT_ASSERT(outstanding_bytes() ==
                 memory_consumed_by_one_frame);                 
}


Reference*
testI2OChain::allocate_frame()
{
  Reference* temp = g_factory->getFrame(g_pool, 1024);
  assert(temp);
  return temp;
}

size_t
testI2OChain::outstanding_bytes()
{
  return g_pool->getMemoryUsage().getUsed();
}

// This macro writes the 'main' for this test.
CPPUNIT_TEST_SUITE_REGISTRATION(testI2OChain);


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
