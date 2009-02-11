#include "Utilities/Testing/interface/CppUnit_testdriver.icpp"
#include "cppunit/extensions/HelperMacros.h"

#include <assert.h>
#include <vector>

#include "toolbox/mem/HeapAllocator.h"
#include "toolbox/mem/Reference.h"
#include "toolbox/mem/MemoryPoolFactory.h"
#include "toolbox/mem/exception/Exception.h"

#include "EventFilter/StorageManager/interface/Exception.h"
#include "EventFilter/StorageManager/interface/I2OChain.h"
#include "EventFilter/Utilities/interface/i2oEvfMsgs.h"
#include "IOPool/Streamer/interface/MsgHeader.h"

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
  CPPUNIT_TEST(assign_chain);
  CPPUNIT_TEST(swap_chain);
  CPPUNIT_TEST(copying_does_not_exhaust_buffer);
  CPPUNIT_TEST(release_chain);
  CPPUNIT_TEST(release_default_chain);
  CPPUNIT_TEST(invalid_fragment);
  CPPUNIT_TEST(populate_i2o_header);
  CPPUNIT_TEST(copy_i2o_header);
  CPPUNIT_TEST(assign_i2o_header);
  CPPUNIT_TEST(swap_i2o_header);
  CPPUNIT_TEST(release_i2o_header);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void default_chain();
  void nonempty_chain_cleans_up_nice();
  void copy_chain();
  void assign_chain();
  void swap_chain();
  void copying_does_not_exhaust_buffer();
  void release_chain();
  void release_default_chain();
  void invalid_fragment();
  void populate_i2o_header();
  void copy_i2o_header();
  void assign_i2o_header();
  void swap_i2o_header();
  void release_i2o_header();

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
  CPPUNIT_ASSERT(!frag.faulty());
  size_t memory_consumed_by_zero_frames = outstanding_bytes();
  CPPUNIT_ASSERT(memory_consumed_by_zero_frames == 0);  
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
    CPPUNIT_ASSERT(!frag.faulty());
    frag.markFaulty();
    CPPUNIT_ASSERT(frag.faulty());
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
testI2OChain::assign_chain()
{
  stor::I2OChain frag1(allocate_frame());
  size_t memory_consumed_by_one_frame = outstanding_bytes();
  CPPUNIT_ASSERT(memory_consumed_by_one_frame != 0);
  
  stor::I2OChain frag2(allocate_frame());
  size_t memory_consumed_by_two_frames = outstanding_bytes();
  CPPUNIT_ASSERT(memory_consumed_by_two_frames > memory_consumed_by_one_frame);

  stor::I2OChain no_frags;
  CPPUNIT_ASSERT(no_frags.empty());

  // Assigning to frag2 should release the resources associated with frag2.
  frag2 = no_frags;
  CPPUNIT_ASSERT(outstanding_bytes() == memory_consumed_by_one_frame);  
  CPPUNIT_ASSERT(frag2.empty());

  // Assigning frag1 to frag2 should consume no new resources
  frag2 = frag1;
  CPPUNIT_ASSERT(outstanding_bytes() == memory_consumed_by_one_frame);  
  CPPUNIT_ASSERT(!frag2.empty());
  CPPUNIT_ASSERT(frag2.getBufferData() == frag1.getBufferData());  

  // Assigning no_frags to frag1 and frag2 should release all resources.
  CPPUNIT_ASSERT(no_frags.empty());
  frag1 = no_frags;
  frag2 = no_frags;
  CPPUNIT_ASSERT(outstanding_bytes() == 0);
}

void
testI2OChain::swap_chain()
{
  CPPUNIT_ASSERT(outstanding_bytes() == 0);
  {
    stor::I2OChain frag(allocate_frame());
    size_t memory_consumed_by_one_frame = outstanding_bytes();
    CPPUNIT_ASSERT(memory_consumed_by_one_frame != 0);
    CPPUNIT_ASSERT(!frag.empty());
    unsigned long* data_location = frag.getBufferData();
    
    stor::I2OChain no_frags;
    CPPUNIT_ASSERT(no_frags.empty());
    CPPUNIT_ASSERT(outstanding_bytes() == memory_consumed_by_one_frame);
    
    // Swapping should not change the amount of allocated memory, but
    // it should reverse the roles: no_frags should be non-empty, and
    // frags should be empty.
    std::swap(no_frags, frag);
    CPPUNIT_ASSERT(outstanding_bytes() == memory_consumed_by_one_frame);
    CPPUNIT_ASSERT(frag.empty());
    CPPUNIT_ASSERT(!no_frags.empty());
    CPPUNIT_ASSERT(no_frags.getBufferData() == data_location);
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

void
testI2OChain::release_chain()
{
  CPPUNIT_ASSERT(outstanding_bytes() ==0);
  {
    stor::I2OChain frag(allocate_frame());
    CPPUNIT_ASSERT(outstanding_bytes() != 0);
    frag.release();
    CPPUNIT_ASSERT(frag.empty());
    CPPUNIT_ASSERT(frag.getBufferData() == 0);
    CPPUNIT_ASSERT(outstanding_bytes() == 0);
    
  }
  CPPUNIT_ASSERT(outstanding_bytes() ==0);
}

void
testI2OChain::release_default_chain()
{
  CPPUNIT_ASSERT(outstanding_bytes() ==0);
  {
    stor::I2OChain empty;
    CPPUNIT_ASSERT(outstanding_bytes() == 0);
    empty.release();
    CPPUNIT_ASSERT(empty.empty());
    CPPUNIT_ASSERT(empty.getBufferData() == 0);
    CPPUNIT_ASSERT(outstanding_bytes() == 0);    
  }
  CPPUNIT_ASSERT(outstanding_bytes() ==0);
}

void 
testI2OChain::invalid_fragment()
{
  CPPUNIT_ASSERT(outstanding_bytes() == 0);
  {
    Reference* ref = allocate_frame();
    I2O_PRIVATE_MESSAGE_FRAME *pvtMsg =
      (I2O_PRIVATE_MESSAGE_FRAME*) ref->getDataLocation();
    pvtMsg->XFunctionCode = 0xff;
    try {
      stor::I2OChain frag(ref);

      // we should  not get here because the I2OChain
      // constructor should throw an exception for
      // an invalid fragment
      CPPUNIT_ASSERT(false);
    }
    catch (stor::exception::Exception& excpt) {
      CPPUNIT_ASSERT(outstanding_bytes() != 0);
      ref->release();
    }
  }
  CPPUNIT_ASSERT(outstanding_bytes() == 0);
}

void
testI2OChain::populate_i2o_header()
{
  CPPUNIT_ASSERT(outstanding_bytes() == 0);
  {
    unsigned int value1 = 0xa5a5d2d2;
    unsigned int value2 = 0xb4b4e1e1;
    unsigned int value3 = 0xc3c3f0f0;
    unsigned int value4 = 0x12345678;

    Reference* ref = allocate_frame();
    I2O_PRIVATE_MESSAGE_FRAME *pvtMsg =
      (I2O_PRIVATE_MESSAGE_FRAME*) ref->getDataLocation();
    pvtMsg->XFunctionCode = I2O_SM_PREAMBLE;
    I2O_SM_PREAMBLE_MESSAGE_FRAME *i2oMsg =
      (I2O_SM_PREAMBLE_MESSAGE_FRAME*) pvtMsg;
    i2oMsg->hltTid = value1;
    i2oMsg->rbBufferID = 2;
    i2oMsg->outModID = value2;
    i2oMsg->fuProcID = value3;
    i2oMsg->fuGUID = value4;

    stor::I2OChain initMsgFrag(ref);
    CPPUNIT_ASSERT(initMsgFrag.getMessageCode() == Header::INIT);
    stor::FragKey const& fragmentKey = initMsgFrag.getFragmentKey();
    CPPUNIT_ASSERT(fragmentKey.code_ == Header::INIT);
    CPPUNIT_ASSERT(fragmentKey.run_ == 0);
    CPPUNIT_ASSERT(fragmentKey.event_ == value1);
    CPPUNIT_ASSERT(fragmentKey.secondaryId_ == value2);
    CPPUNIT_ASSERT(fragmentKey.originatorPid_ == value3);
    CPPUNIT_ASSERT(fragmentKey.originatorGuid_ == value4);
  }
  CPPUNIT_ASSERT(outstanding_bytes() == 0);
}

void
testI2OChain::copy_i2o_header()
{
  CPPUNIT_ASSERT(outstanding_bytes() == 0);
  {
    unsigned int value1 = 0xa5a5d2d2;
    unsigned int value2 = 0xb4b4e1e1;
    unsigned int value3 = 0xc3c3f0f0;
    unsigned int value4 = 0x01234567;
    unsigned int value5 = 0x89abcdef;

    Reference* ref = allocate_frame();
    I2O_PRIVATE_MESSAGE_FRAME *pvtMsg =
      (I2O_PRIVATE_MESSAGE_FRAME*) ref->getDataLocation();
    pvtMsg->XFunctionCode = I2O_SM_DATA;
    I2O_SM_DATA_MESSAGE_FRAME *i2oMsg =
      (I2O_SM_DATA_MESSAGE_FRAME*) pvtMsg;
    i2oMsg->rbBufferID = 2;
    i2oMsg->runID = value1;
    i2oMsg->eventID = value2;
    i2oMsg->outModID = value3;
    i2oMsg->fuProcID = value4;
    i2oMsg->fuGUID = value5;

    stor::I2OChain eventMsgFrag(ref);
    stor::I2OChain copy(eventMsgFrag);

    {
      CPPUNIT_ASSERT(eventMsgFrag.getMessageCode() == Header::EVENT);
      stor::FragKey const& fragmentKey = eventMsgFrag.getFragmentKey();
      CPPUNIT_ASSERT(fragmentKey.code_ == Header::EVENT);
      CPPUNIT_ASSERT(fragmentKey.run_ == value1);
      CPPUNIT_ASSERT(fragmentKey.event_ == value2);
      CPPUNIT_ASSERT(fragmentKey.secondaryId_ == value3);
      CPPUNIT_ASSERT(fragmentKey.originatorPid_ == value4);
      CPPUNIT_ASSERT(fragmentKey.originatorGuid_ == value5);
    }

    {
      CPPUNIT_ASSERT(copy.getMessageCode() == Header::EVENT);
      stor::FragKey const& fragmentKey = copy.getFragmentKey();
      CPPUNIT_ASSERT(fragmentKey.code_ == Header::EVENT);
      CPPUNIT_ASSERT(fragmentKey.run_ == value1);
      CPPUNIT_ASSERT(fragmentKey.event_ == value2);
      CPPUNIT_ASSERT(fragmentKey.secondaryId_ == value3);
      CPPUNIT_ASSERT(fragmentKey.originatorPid_ == value4);
      CPPUNIT_ASSERT(fragmentKey.originatorGuid_ == value5);
    }
  }
  CPPUNIT_ASSERT(outstanding_bytes() == 0);
}

void
testI2OChain::assign_i2o_header()
{
  CPPUNIT_ASSERT(outstanding_bytes() == 0);
  {
    unsigned int value1 = 0xa5a5d2d2;
    unsigned int value2 = 0xb4b4e1e1;
    unsigned int value3 = 0xc3c3f0f0;
    unsigned int value4 = 0x01234567;
    unsigned int value5 = 0x89abcdef;

    Reference* ref = allocate_frame();
    I2O_PRIVATE_MESSAGE_FRAME *pvtMsg =
      (I2O_PRIVATE_MESSAGE_FRAME*) ref->getDataLocation();
    pvtMsg->XFunctionCode = I2O_SM_ERROR;
    I2O_SM_DATA_MESSAGE_FRAME *i2oMsg =
      (I2O_SM_DATA_MESSAGE_FRAME*) pvtMsg;
    i2oMsg->rbBufferID = 2;
    i2oMsg->runID = value1;
    i2oMsg->eventID = value2;
    i2oMsg->outModID = value3;
    i2oMsg->fuProcID = value4;
    i2oMsg->fuGUID = value5;

    stor::I2OChain eventMsgFrag(ref);
    stor::I2OChain copy = eventMsgFrag;

    {
      CPPUNIT_ASSERT(eventMsgFrag.getMessageCode() == Header::ERROR_EVENT);
      stor::FragKey const& fragmentKey = eventMsgFrag.getFragmentKey();
      CPPUNIT_ASSERT(fragmentKey.code_ == Header::ERROR_EVENT);
      CPPUNIT_ASSERT(fragmentKey.run_ == value1);
      CPPUNIT_ASSERT(fragmentKey.event_ == value2);
      CPPUNIT_ASSERT(fragmentKey.secondaryId_ == value3);
      CPPUNIT_ASSERT(fragmentKey.originatorPid_ == value4);
      CPPUNIT_ASSERT(fragmentKey.originatorGuid_ == value5);
    }

    {
      CPPUNIT_ASSERT(copy.getMessageCode() == Header::ERROR_EVENT);
      stor::FragKey const& fragmentKey = copy.getFragmentKey();
      CPPUNIT_ASSERT(fragmentKey.code_ == Header::ERROR_EVENT);
      CPPUNIT_ASSERT(fragmentKey.run_ == value1);
      CPPUNIT_ASSERT(fragmentKey.event_ == value2);
      CPPUNIT_ASSERT(fragmentKey.secondaryId_ == value3);
      CPPUNIT_ASSERT(fragmentKey.originatorPid_ == value4);
      CPPUNIT_ASSERT(fragmentKey.originatorGuid_ == value5);
    }
  }
  CPPUNIT_ASSERT(outstanding_bytes() == 0);
}

void
testI2OChain::swap_i2o_header()
{
  CPPUNIT_ASSERT(outstanding_bytes() == 0);
  {
    unsigned int value1 = 0xa5a5d2d2;
    unsigned int value2 = 0xb4b4e1e1;
    unsigned int value3 = 0xc3c3f0f0;
    unsigned int value4 = 0x01234567;
    unsigned int value5 = 0x89abcdef;

    Reference* ref = allocate_frame();
    I2O_PRIVATE_MESSAGE_FRAME *pvtMsg =
      (I2O_PRIVATE_MESSAGE_FRAME*) ref->getDataLocation();
    pvtMsg->XFunctionCode = I2O_SM_DQM;
    I2O_SM_DQM_MESSAGE_FRAME *i2oMsg =
      (I2O_SM_DQM_MESSAGE_FRAME*) pvtMsg;
    i2oMsg->rbBufferID = 2;
    i2oMsg->runID = value1;
    i2oMsg->eventAtUpdateID = value2;
    i2oMsg->folderID = value3;
    i2oMsg->fuProcID = value4;
    i2oMsg->fuGUID = value5;
    stor::I2OChain frag1(ref);

    ref = allocate_frame();
    pvtMsg = (I2O_PRIVATE_MESSAGE_FRAME*) ref->getDataLocation();
    pvtMsg->XFunctionCode = I2O_SM_DQM;
    i2oMsg = (I2O_SM_DQM_MESSAGE_FRAME*) pvtMsg;
    i2oMsg->rbBufferID = 3;
    i2oMsg->runID = value5;
    i2oMsg->eventAtUpdateID = value4;
    i2oMsg->folderID = value3;
    i2oMsg->fuProcID = value2;
    i2oMsg->fuGUID = value1;
    stor::I2OChain frag2(ref);

    {
      CPPUNIT_ASSERT(frag1.getMessageCode() == Header::DQM_EVENT);
      stor::FragKey const& fragmentKey = frag1.getFragmentKey();
      CPPUNIT_ASSERT(fragmentKey.code_ == Header::DQM_EVENT);
      CPPUNIT_ASSERT(fragmentKey.run_ == value1);
      CPPUNIT_ASSERT(fragmentKey.event_ == value2);
      CPPUNIT_ASSERT(fragmentKey.secondaryId_ == value3);
      CPPUNIT_ASSERT(fragmentKey.originatorPid_ == value4);
      CPPUNIT_ASSERT(fragmentKey.originatorGuid_ == value5);
    }

    {
      CPPUNIT_ASSERT(frag2.getMessageCode() == Header::DQM_EVENT);
      stor::FragKey const& fragmentKey = frag2.getFragmentKey();
      CPPUNIT_ASSERT(fragmentKey.code_ == Header::DQM_EVENT);
      CPPUNIT_ASSERT(fragmentKey.run_ == value5);
      CPPUNIT_ASSERT(fragmentKey.event_ == value4);
      CPPUNIT_ASSERT(fragmentKey.secondaryId_ == value3);
      CPPUNIT_ASSERT(fragmentKey.originatorPid_ == value2);
      CPPUNIT_ASSERT(fragmentKey.originatorGuid_ == value1);
    }

    std::swap(frag1, frag2);

    {
      CPPUNIT_ASSERT(frag1.getMessageCode() == Header::DQM_EVENT);
      stor::FragKey const& fragmentKey = frag1.getFragmentKey();
      CPPUNIT_ASSERT(fragmentKey.code_ == Header::DQM_EVENT);
      CPPUNIT_ASSERT(fragmentKey.run_ == value5);
      CPPUNIT_ASSERT(fragmentKey.event_ == value4);
      CPPUNIT_ASSERT(fragmentKey.secondaryId_ == value3);
      CPPUNIT_ASSERT(fragmentKey.originatorPid_ == value2);
      CPPUNIT_ASSERT(fragmentKey.originatorGuid_ == value1);
    }

    {
      CPPUNIT_ASSERT(frag2.getMessageCode() == Header::DQM_EVENT);
      stor::FragKey const& fragmentKey = frag2.getFragmentKey();
      CPPUNIT_ASSERT(fragmentKey.code_ == Header::DQM_EVENT);
      CPPUNIT_ASSERT(fragmentKey.run_ == value1);
      CPPUNIT_ASSERT(fragmentKey.event_ == value2);
      CPPUNIT_ASSERT(fragmentKey.secondaryId_ == value3);
      CPPUNIT_ASSERT(fragmentKey.originatorPid_ == value4);
      CPPUNIT_ASSERT(fragmentKey.originatorGuid_ == value5);
    }
  }
  CPPUNIT_ASSERT(outstanding_bytes() == 0);
}

void
testI2OChain::release_i2o_header()
{
  CPPUNIT_ASSERT(outstanding_bytes() == 0);
  {
    unsigned int value1 = 0xa5a5d2d2;
    unsigned int value2 = 0xb4b4e1e1;
    unsigned int value3 = 0xc3c3f0f0;
    unsigned int value4 = 0x12345678;

    Reference* ref = allocate_frame();
    I2O_PRIVATE_MESSAGE_FRAME *pvtMsg =
      (I2O_PRIVATE_MESSAGE_FRAME*) ref->getDataLocation();
    pvtMsg->XFunctionCode = I2O_SM_PREAMBLE;
    I2O_SM_PREAMBLE_MESSAGE_FRAME *i2oMsg =
      (I2O_SM_PREAMBLE_MESSAGE_FRAME*) pvtMsg;
    i2oMsg->hltTid = value1;
    i2oMsg->rbBufferID = 2;
    i2oMsg->outModID = value2;
    i2oMsg->fuProcID = value3;
    i2oMsg->fuGUID = value4;

    stor::I2OChain initMsgFrag(ref);
    initMsgFrag.release();
    CPPUNIT_ASSERT(initMsgFrag.getMessageCode() == 0);
    stor::FragKey const& fragmentKey = initMsgFrag.getFragmentKey();
    CPPUNIT_ASSERT(fragmentKey.code_ == 0);
    CPPUNIT_ASSERT(fragmentKey.run_ == 0);
    CPPUNIT_ASSERT(fragmentKey.event_ == 0);
    CPPUNIT_ASSERT(fragmentKey.secondaryId_ == 0);
    CPPUNIT_ASSERT(fragmentKey.originatorPid_ == 0);
    CPPUNIT_ASSERT(fragmentKey.originatorGuid_ == 0);
  }
  CPPUNIT_ASSERT(outstanding_bytes() == 0);
}


Reference*
testI2OChain::allocate_frame()
{
  Reference* temp = g_factory->getFrame(g_pool, 1024);
  assert(temp);

  // 11-Feb-2009, KAB - provide a default value for the I2O message type
  // so that the I2OChain constructor doesn't throw an exception when
  // it tries to determine what type of message is in the buffer
  I2O_PRIVATE_MESSAGE_FRAME *pvtMsg =
    (I2O_PRIVATE_MESSAGE_FRAME*) temp->getDataLocation();
  pvtMsg->XFunctionCode = I2O_SM_ERROR;

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
