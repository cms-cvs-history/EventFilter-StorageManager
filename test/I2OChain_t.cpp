#include "Utilities/Testing/interface/CppUnit_testdriver.icpp"
#include "cppunit/extensions/HelperMacros.h"

#include "EventFilter/StorageManager/interface/I2OChain.h"

class testI2OChain : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(testI2OChain);
  CPPUNIT_TEST(default_chain);
  CPPUNIT_TEST(copy_chain);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void default_chain();
  void copy_chain();

private:
  // No data members yet.
};

void
testI2OChain::setUp()
{ 
//   toolbox::net::URN urn("toolbox-mem-pool", "myPool");
//   toolbox::mem::HeapAllocator* a = new toolbox::mem::HeapAllocator();
//   toolbox::mem::Pool * p = toolbox::mem::getMemoryPoolFactory()->createPool(urn, a);

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
testI2OChain::copy_chain()
{
}

// This macro writes the 'main' for this test.
CPPUNIT_TEST_SUITE_REGISTRATION(testI2OChain);


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
