#include "Utilities/Testing/interface/CppUnit_testdriver.icpp"
#include "cppunit/extensions/HelperMacros.h"

#include <assert.h>
#include <vector>

#include "EventFilter/StorageManager/interface/Exception.h"
#include "EventFilter/StorageManager/interface/I2OChain.h"

#include "EventFilter/StorageManager/test/TestHelper.h"

using stor::testhelper::outstanding_bytes;
using stor::testhelper::allocate_frame;
using stor::testhelper::allocate_frame_with_basic_header;
using stor::testhelper::allocate_frame_with_sample_header;


class testI2OChain : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(testI2OChain);
  CPPUNIT_TEST(default_chain);
  CPPUNIT_TEST(null_reference);
  CPPUNIT_TEST(nonempty_chain_cleans_up_nice);
  CPPUNIT_TEST(copy_chain);
  CPPUNIT_TEST(assign_chain);
  CPPUNIT_TEST(swap_chain);
  CPPUNIT_TEST(copying_does_not_exhaust_buffer);
  CPPUNIT_TEST(release_chain);
  CPPUNIT_TEST(release_default_chain);
  CPPUNIT_TEST(invalid_fragment);
  CPPUNIT_TEST(populate_i2o_header);
  CPPUNIT_TEST(copy_with_valid_header);
  CPPUNIT_TEST(assign_with_valid_header);
  CPPUNIT_TEST(swap_with_valid_header);
  CPPUNIT_TEST(release_with_valid_header);
  CPPUNIT_TEST(add_fragment);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void default_chain();
  void null_reference();
  void nonempty_chain_cleans_up_nice();
  void copy_chain();
  void assign_chain();
  void swap_chain();
  void copying_does_not_exhaust_buffer();
  void release_chain();
  void release_default_chain();
  void invalid_fragment();
  void populate_i2o_header();
  void copy_with_valid_header();
  void assign_with_valid_header();
  void swap_with_valid_header();
  void release_with_valid_header();
  void add_fragment();

private:

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
  CPPUNIT_ASSERT(frag.messageCode() == Header::INVALID);
  CPPUNIT_ASSERT(frag.fragmentCount() == 0);
  CPPUNIT_ASSERT(frag.creationTime() == -1);
  CPPUNIT_ASSERT(frag.lastFragmentTime() == -1);
  //CPPUNIT_ASSERT(!frag.getTotalDataSize() == 0);
  size_t memory_consumed_by_zero_frames = outstanding_bytes();
  CPPUNIT_ASSERT(memory_consumed_by_zero_frames == 0);  
}

void 
testI2OChain::null_reference()
{
  stor::I2OChain frag(0);
  CPPUNIT_ASSERT(frag.empty());
  CPPUNIT_ASSERT(!frag.complete());
  CPPUNIT_ASSERT(!frag.faulty());
  CPPUNIT_ASSERT(frag.messageCode() == Header::INVALID);
  CPPUNIT_ASSERT(frag.fragmentCount() == 0);
  CPPUNIT_ASSERT(frag.creationTime() == -1);
  CPPUNIT_ASSERT(frag.lastFragmentTime() == -1);
  //CPPUNIT_ASSERT(!frag.getTotalDataSize() == 0);
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
    //frag.markComplete();
    //CPPUNIT_ASSERT(frag.complete());
    CPPUNIT_ASSERT(frag.faulty());  // because the buffer is empty
    frag.markFaulty();
    CPPUNIT_ASSERT(frag.faulty());
    CPPUNIT_ASSERT(frag.messageCode() == Header::INVALID);
    CPPUNIT_ASSERT(frag.fragmentCount() == 1);
  }
  CPPUNIT_ASSERT(outstanding_bytes() == 0);
}


void
testI2OChain::copy_chain()
{
  CPPUNIT_ASSERT(outstanding_bytes() == 0);
  {
    stor::I2OChain frag(allocate_frame());
    double creationTime = frag.creationTime();
    double lastFragmentTime = frag.lastFragmentTime();
    size_t memory_consumed_by_one_frame = outstanding_bytes();
    CPPUNIT_ASSERT(memory_consumed_by_one_frame != 0);
    {
      stor::I2OChain copy(frag);
      size_t memory_consumed_after_copy = outstanding_bytes();
      CPPUNIT_ASSERT(memory_consumed_after_copy != 0);
      CPPUNIT_ASSERT(memory_consumed_after_copy ==
                     memory_consumed_by_one_frame);
      CPPUNIT_ASSERT(copy.getBufferData() == frag.getBufferData());
      CPPUNIT_ASSERT(copy.creationTime() == creationTime); 
      CPPUNIT_ASSERT(copy.lastFragmentTime() == lastFragmentTime); 
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
  CPPUNIT_ASSERT(frag2.creationTime() == frag1.creationTime());  
  CPPUNIT_ASSERT(frag2.lastFragmentTime() == frag1.lastFragmentTime());  

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
    double creationTime = frag.creationTime();
    double lastFragmentTime = frag.lastFragmentTime();

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
    CPPUNIT_ASSERT(no_frags.creationTime() == creationTime);
    CPPUNIT_ASSERT(no_frags.lastFragmentTime() == lastFragmentTime);
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
    CPPUNIT_ASSERT(frag.creationTime() == -1);
    CPPUNIT_ASSERT(frag.lastFragmentTime() == -1);
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
    stor::I2OChain frag(ref);
    CPPUNIT_ASSERT(!frag.empty());
    CPPUNIT_ASSERT(!frag.complete());
    CPPUNIT_ASSERT(frag.faulty());
    CPPUNIT_ASSERT(frag.messageCode() == Header::INVALID);
    CPPUNIT_ASSERT(frag.fragmentCount() == 1);
    CPPUNIT_ASSERT(outstanding_bytes() != 0);
  }
  CPPUNIT_ASSERT(outstanding_bytes() == 0);
  {
    Reference* ref = allocate_frame_with_basic_header(0xff, 0, 0);
    stor::I2OChain frag(ref);
    CPPUNIT_ASSERT(!frag.empty());
    CPPUNIT_ASSERT(!frag.complete());
    CPPUNIT_ASSERT(frag.faulty());
    CPPUNIT_ASSERT(frag.messageCode() == Header::INVALID);
    CPPUNIT_ASSERT(frag.fragmentCount() == 1);
    CPPUNIT_ASSERT(outstanding_bytes() != 0);
  }
  CPPUNIT_ASSERT(outstanding_bytes() == 0);
  {
    Reference* ref = allocate_frame_with_basic_header(I2O_SM_ERROR, 0, 0);
    stor::I2OChain frag(ref);
    CPPUNIT_ASSERT(!frag.empty());
    CPPUNIT_ASSERT(!frag.complete());
    CPPUNIT_ASSERT(frag.faulty());
    CPPUNIT_ASSERT(frag.messageCode() == Header::INVALID);
    CPPUNIT_ASSERT(frag.fragmentCount() == 1);
    CPPUNIT_ASSERT(outstanding_bytes() != 0);
  }
  {
    Reference* ref = allocate_frame_with_basic_header(I2O_SM_ERROR, 3, 3);
    stor::I2OChain frag(ref);
    CPPUNIT_ASSERT(!frag.empty());
    CPPUNIT_ASSERT(!frag.complete());
    CPPUNIT_ASSERT(frag.faulty());
    CPPUNIT_ASSERT(frag.messageCode() == Header::INVALID);
    CPPUNIT_ASSERT(frag.fragmentCount() == 1);
    CPPUNIT_ASSERT(outstanding_bytes() != 0);
  }
  CPPUNIT_ASSERT(outstanding_bytes() == 0);
  {
    Reference* ref = allocate_frame_with_basic_header(I2O_SM_ERROR, 100, 2);
    stor::I2OChain frag(ref);
    CPPUNIT_ASSERT(!frag.empty());
    CPPUNIT_ASSERT(!frag.complete());
    CPPUNIT_ASSERT(frag.faulty());
    CPPUNIT_ASSERT(frag.messageCode() == Header::INVALID);
    CPPUNIT_ASSERT(frag.fragmentCount() == 1);
    CPPUNIT_ASSERT(outstanding_bytes() != 0);
  }
  CPPUNIT_ASSERT(outstanding_bytes() == 0);
  {
    Reference* ref = allocate_frame_with_basic_header(I2O_SM_ERROR, 0, 1);
    stor::I2OChain frag(ref);
    CPPUNIT_ASSERT(!frag.empty());
    CPPUNIT_ASSERT(frag.complete());
    CPPUNIT_ASSERT(!frag.faulty());
    CPPUNIT_ASSERT(frag.messageCode() == Header::ERROR_EVENT);
    CPPUNIT_ASSERT(frag.fragmentCount() == 1);
    CPPUNIT_ASSERT(outstanding_bytes() != 0);
  }
  CPPUNIT_ASSERT(outstanding_bytes() == 0);
  {
    Reference* ref = allocate_frame_with_basic_header(I2O_SM_ERROR, 1, 2);
    stor::I2OChain frag(ref);
    CPPUNIT_ASSERT(!frag.empty());
    CPPUNIT_ASSERT(!frag.complete());
    CPPUNIT_ASSERT(!frag.faulty());
    CPPUNIT_ASSERT(frag.messageCode() == Header::ERROR_EVENT);
    CPPUNIT_ASSERT(frag.fragmentCount() == 1);
    CPPUNIT_ASSERT(outstanding_bytes() != 0);
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

    Reference* ref = allocate_frame_with_basic_header(I2O_SM_PREAMBLE, 0, 1);
    I2O_SM_PREAMBLE_MESSAGE_FRAME *i2oMsg =
      (I2O_SM_PREAMBLE_MESSAGE_FRAME*) ref->getDataLocation();
    i2oMsg->hltTid = value1;
    i2oMsg->rbBufferID = 2;
    i2oMsg->outModID = value2;
    i2oMsg->fuProcID = value3;
    i2oMsg->fuGUID = value4;

    stor::I2OChain initMsgFrag(ref);
    CPPUNIT_ASSERT(initMsgFrag.messageCode() == Header::INIT);
    stor::FragKey fragmentKey = initMsgFrag.fragmentKey();
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
testI2OChain::copy_with_valid_header()
{
  CPPUNIT_ASSERT(outstanding_bytes() == 0);
  {
    unsigned int value1 = 0xa5a5d2d2;
    unsigned int value2 = 0xb4b4e1e1;
    unsigned int value3 = 0xc3c3f0f0;
    unsigned int value4 = 0x01234567;
    unsigned int value5 = 0x89abcdef;

    Reference* ref = allocate_frame_with_basic_header(I2O_SM_DATA, 0, 1);
    I2O_SM_DATA_MESSAGE_FRAME *i2oMsg =
      (I2O_SM_DATA_MESSAGE_FRAME*) ref->getDataLocation();
    i2oMsg->rbBufferID = 2;
    i2oMsg->runID = value1;
    i2oMsg->eventID = value2;
    i2oMsg->outModID = value3;
    i2oMsg->fuProcID = value4;
    i2oMsg->fuGUID = value5;

    stor::I2OChain eventMsgFrag(ref);
    stor::I2OChain copy(eventMsgFrag);

    {
      CPPUNIT_ASSERT(eventMsgFrag.messageCode() == Header::EVENT);
      stor::FragKey fragmentKey = eventMsgFrag.fragmentKey();
      CPPUNIT_ASSERT(fragmentKey.code_ == Header::EVENT);
      CPPUNIT_ASSERT(fragmentKey.run_ == value1);
      CPPUNIT_ASSERT(fragmentKey.event_ == value2);
      CPPUNIT_ASSERT(fragmentKey.secondaryId_ == value3);
      CPPUNIT_ASSERT(fragmentKey.originatorPid_ == value4);
      CPPUNIT_ASSERT(fragmentKey.originatorGuid_ == value5);
    }

    {
      CPPUNIT_ASSERT(copy.messageCode() == Header::EVENT);
      stor::FragKey fragmentKey = copy.fragmentKey();
      CPPUNIT_ASSERT(fragmentKey.code_ == Header::EVENT);
      CPPUNIT_ASSERT(fragmentKey.run_ == value1);
      CPPUNIT_ASSERT(fragmentKey.event_ == value2);
      CPPUNIT_ASSERT(fragmentKey.secondaryId_ == value3);
      CPPUNIT_ASSERT(fragmentKey.originatorPid_ == value4);
      CPPUNIT_ASSERT(fragmentKey.originatorGuid_ == value5);
    }

    {
      CPPUNIT_ASSERT(eventMsgFrag.creationTime() == copy.creationTime());
      CPPUNIT_ASSERT(eventMsgFrag.lastFragmentTime() == copy.lastFragmentTime());
    }
  }
  CPPUNIT_ASSERT(outstanding_bytes() == 0);
}

void
testI2OChain::assign_with_valid_header()
{
  CPPUNIT_ASSERT(outstanding_bytes() == 0);
  {
    unsigned int value1 = 0xa5a5d2d2;
    unsigned int value2 = 0xb4b4e1e1;
    unsigned int value3 = 0xc3c3f0f0;
    unsigned int value4 = 0x01234567;
    unsigned int value5 = 0x89abcdef;

    Reference* ref = allocate_frame_with_basic_header(I2O_SM_ERROR, 0, 1);
    I2O_SM_DATA_MESSAGE_FRAME *i2oMsg =
      (I2O_SM_DATA_MESSAGE_FRAME*) ref->getDataLocation();
    i2oMsg->rbBufferID = 2;
    i2oMsg->runID = value1;
    i2oMsg->eventID = value2;
    i2oMsg->outModID = value3;
    i2oMsg->fuProcID = value4;
    i2oMsg->fuGUID = value5;

    stor::I2OChain eventMsgFrag(ref);
    stor::I2OChain copy = eventMsgFrag;

    {
      CPPUNIT_ASSERT(eventMsgFrag.messageCode() == Header::ERROR_EVENT);
      stor::FragKey fragmentKey = eventMsgFrag.fragmentKey();
      CPPUNIT_ASSERT(fragmentKey.code_ == Header::ERROR_EVENT);
      CPPUNIT_ASSERT(fragmentKey.run_ == value1);
      CPPUNIT_ASSERT(fragmentKey.event_ == value2);
      CPPUNIT_ASSERT(fragmentKey.secondaryId_ == value3);
      CPPUNIT_ASSERT(fragmentKey.originatorPid_ == value4);
      CPPUNIT_ASSERT(fragmentKey.originatorGuid_ == value5);
    }

    {
      CPPUNIT_ASSERT(copy.messageCode() == Header::ERROR_EVENT);
      stor::FragKey fragmentKey = copy.fragmentKey();
      CPPUNIT_ASSERT(fragmentKey.code_ == Header::ERROR_EVENT);
      CPPUNIT_ASSERT(fragmentKey.run_ == value1);
      CPPUNIT_ASSERT(fragmentKey.event_ == value2);
      CPPUNIT_ASSERT(fragmentKey.secondaryId_ == value3);
      CPPUNIT_ASSERT(fragmentKey.originatorPid_ == value4);
      CPPUNIT_ASSERT(fragmentKey.originatorGuid_ == value5);
    }

    {
      CPPUNIT_ASSERT(eventMsgFrag.creationTime() == copy.creationTime());
      CPPUNIT_ASSERT(eventMsgFrag.lastFragmentTime() == copy.lastFragmentTime());
    }
  }
  CPPUNIT_ASSERT(outstanding_bytes() == 0);
}

void
testI2OChain::swap_with_valid_header()
{
  CPPUNIT_ASSERT(outstanding_bytes() == 0);
  {
    unsigned int value1 = 0xa5a5d2d2;
    unsigned int value2 = 0xb4b4e1e1;
    unsigned int value3 = 0xc3c3f0f0;
    unsigned int value4 = 0x01234567;
    unsigned int value5 = 0x89abcdef;

    Reference* ref = allocate_frame_with_basic_header(I2O_SM_DQM, 0, 1);
    I2O_SM_DQM_MESSAGE_FRAME *i2oMsg =
      (I2O_SM_DQM_MESSAGE_FRAME*) ref->getDataLocation();
    i2oMsg->rbBufferID = 2;
    i2oMsg->runID = value1;
    i2oMsg->eventAtUpdateID = value2;
    i2oMsg->folderID = value3;
    i2oMsg->fuProcID = value4;
    i2oMsg->fuGUID = value5;
    stor::I2OChain frag1(ref);
    double creationTime1 = frag1.creationTime();
    double lastFragmentTime1 = frag1.lastFragmentTime();
 
    ref = allocate_frame_with_basic_header(I2O_SM_DQM, 0, 1);
    i2oMsg = (I2O_SM_DQM_MESSAGE_FRAME*) ref->getDataLocation();
    i2oMsg->rbBufferID = 3;
    i2oMsg->runID = value5;
    i2oMsg->eventAtUpdateID = value4;
    i2oMsg->folderID = value3;
    i2oMsg->fuProcID = value2;
    i2oMsg->fuGUID = value1;
    stor::I2OChain frag2(ref);
    double creationTime2 = frag2.creationTime();
    double lastFragmentTime2 = frag2.lastFragmentTime();

    {
      CPPUNIT_ASSERT(frag1.messageCode() == Header::DQM_EVENT);
      stor::FragKey fragmentKey = frag1.fragmentKey();
      CPPUNIT_ASSERT(fragmentKey.code_ == Header::DQM_EVENT);
      CPPUNIT_ASSERT(fragmentKey.run_ == value1);
      CPPUNIT_ASSERT(fragmentKey.event_ == value2);
      CPPUNIT_ASSERT(fragmentKey.secondaryId_ == value3);
      CPPUNIT_ASSERT(fragmentKey.originatorPid_ == value4);
      CPPUNIT_ASSERT(fragmentKey.originatorGuid_ == value5);
    }

    {
      CPPUNIT_ASSERT(frag2.messageCode() == Header::DQM_EVENT);
      stor::FragKey fragmentKey = frag2.fragmentKey();
      CPPUNIT_ASSERT(fragmentKey.code_ == Header::DQM_EVENT);
      CPPUNIT_ASSERT(fragmentKey.run_ == value5);
      CPPUNIT_ASSERT(fragmentKey.event_ == value4);
      CPPUNIT_ASSERT(fragmentKey.secondaryId_ == value3);
      CPPUNIT_ASSERT(fragmentKey.originatorPid_ == value2);
      CPPUNIT_ASSERT(fragmentKey.originatorGuid_ == value1);
    }

    std::swap(frag1, frag2);

    {
      CPPUNIT_ASSERT(frag1.messageCode() == Header::DQM_EVENT);
      stor::FragKey fragmentKey = frag1.fragmentKey();
      CPPUNIT_ASSERT(fragmentKey.code_ == Header::DQM_EVENT);
      CPPUNIT_ASSERT(fragmentKey.run_ == value5);
      CPPUNIT_ASSERT(fragmentKey.event_ == value4);
      CPPUNIT_ASSERT(fragmentKey.secondaryId_ == value3);
      CPPUNIT_ASSERT(fragmentKey.originatorPid_ == value2);
      CPPUNIT_ASSERT(fragmentKey.originatorGuid_ == value1);
    }

    {
      CPPUNIT_ASSERT(frag2.messageCode() == Header::DQM_EVENT);
      stor::FragKey fragmentKey = frag2.fragmentKey();
      CPPUNIT_ASSERT(fragmentKey.code_ == Header::DQM_EVENT);
      CPPUNIT_ASSERT(fragmentKey.run_ == value1);
      CPPUNIT_ASSERT(fragmentKey.event_ == value2);
      CPPUNIT_ASSERT(fragmentKey.secondaryId_ == value3);
      CPPUNIT_ASSERT(fragmentKey.originatorPid_ == value4);
      CPPUNIT_ASSERT(fragmentKey.originatorGuid_ == value5);
    }

    {
      CPPUNIT_ASSERT(frag1.creationTime() == creationTime2);
      CPPUNIT_ASSERT(frag2.creationTime() == creationTime1);
      CPPUNIT_ASSERT(frag1.lastFragmentTime() == lastFragmentTime2);
      CPPUNIT_ASSERT(frag2.lastFragmentTime() == lastFragmentTime1);
    }
  }
  CPPUNIT_ASSERT(outstanding_bytes() == 0);
}

void
testI2OChain::release_with_valid_header()
{
  CPPUNIT_ASSERT(outstanding_bytes() == 0);
  {
    unsigned int value1 = 0xa5a5d2d2;
    unsigned int value2 = 0xb4b4e1e1;
    unsigned int value3 = 0xc3c3f0f0;
    unsigned int value4 = 0x12345678;

    Reference* ref = allocate_frame_with_basic_header(I2O_SM_PREAMBLE, 0, 1);
    I2O_SM_PREAMBLE_MESSAGE_FRAME *i2oMsg =
      (I2O_SM_PREAMBLE_MESSAGE_FRAME*) ref->getDataLocation();
    i2oMsg->hltTid = value1;
    i2oMsg->rbBufferID = 2;
    i2oMsg->outModID = value2;
    i2oMsg->fuProcID = value3;
    i2oMsg->fuGUID = value4;

    stor::I2OChain initMsgFrag(ref);
    CPPUNIT_ASSERT(initMsgFrag.messageCode() == Header::INIT);
    stor::FragKey fragmentKey = initMsgFrag.fragmentKey();
    CPPUNIT_ASSERT(fragmentKey.code_ == Header::INIT);
    CPPUNIT_ASSERT(fragmentKey.run_ == 0);
    CPPUNIT_ASSERT(fragmentKey.event_ == value1);
    CPPUNIT_ASSERT(fragmentKey.secondaryId_ == value2);
    CPPUNIT_ASSERT(fragmentKey.originatorPid_ == value3);
    CPPUNIT_ASSERT(fragmentKey.originatorGuid_ == value4);

    initMsgFrag.release();
    CPPUNIT_ASSERT(initMsgFrag.messageCode() == 0);
    CPPUNIT_ASSERT(initMsgFrag.creationTime() == -1);
    CPPUNIT_ASSERT(initMsgFrag.lastFragmentTime() == -1);
    fragmentKey = initMsgFrag.fragmentKey();
    CPPUNIT_ASSERT(fragmentKey.code_ == 0);
    CPPUNIT_ASSERT(fragmentKey.run_ == 0);
    CPPUNIT_ASSERT(fragmentKey.event_ == 0);
    CPPUNIT_ASSERT(fragmentKey.secondaryId_ == 0);
    CPPUNIT_ASSERT(fragmentKey.originatorPid_ == 0);
    CPPUNIT_ASSERT(fragmentKey.originatorGuid_ == 0);
  }
  CPPUNIT_ASSERT(outstanding_bytes() == 0);
}

void
testI2OChain::add_fragment()
{

  CPPUNIT_ASSERT(outstanding_bytes() == 0);
  {
    // add two fragments together (normal order)

    Reference* ref = allocate_frame_with_sample_header(0, 2);
    stor::I2OChain frag1(ref);
    double creationTime1 = frag1.creationTime();

    ref = allocate_frame_with_sample_header(1, 2);
    stor::I2OChain frag2(ref);
    double lastFragmentTime2 = frag2.lastFragmentTime();

    frag1.addToChain(frag2);
    CPPUNIT_ASSERT(!frag1.empty());
    CPPUNIT_ASSERT(frag2.empty());
    CPPUNIT_ASSERT(frag1.complete());
    CPPUNIT_ASSERT(!frag2.complete());
    CPPUNIT_ASSERT(!frag1.faulty());
    CPPUNIT_ASSERT(!frag2.faulty());
    CPPUNIT_ASSERT(frag1.getFragmentID(0) == 0);
    CPPUNIT_ASSERT(frag1.getFragmentID(1) == 1);

    CPPUNIT_ASSERT(frag1.creationTime() == creationTime1);
    CPPUNIT_ASSERT(frag1.lastFragmentTime() > lastFragmentTime2);
  }
  CPPUNIT_ASSERT(outstanding_bytes() == 0);
  {
    // add two fragments together (reverse order)

    Reference* ref = allocate_frame_with_sample_header(0, 2);
    stor::I2OChain frag2(ref);
    double creationTime2 = frag2.creationTime();

    ref = allocate_frame_with_sample_header(1, 2);
    stor::I2OChain frag1(ref);
    double lastFragmentTime1 = frag1.lastFragmentTime();

    frag1.addToChain(frag2);
    CPPUNIT_ASSERT(!frag1.empty());
    CPPUNIT_ASSERT(frag2.empty());
    CPPUNIT_ASSERT(frag1.complete());
    CPPUNIT_ASSERT(!frag2.complete());
    CPPUNIT_ASSERT(!frag1.faulty());
    CPPUNIT_ASSERT(!frag2.faulty());
    CPPUNIT_ASSERT(frag1.getFragmentID(0) == 0);
    CPPUNIT_ASSERT(frag1.getFragmentID(1) == 1);

    CPPUNIT_ASSERT(frag1.creationTime() == creationTime2);
    CPPUNIT_ASSERT(frag1.lastFragmentTime() > lastFragmentTime1);

    // verify that adding a fragment to a complete chain throws an exception

    ref = allocate_frame_with_sample_header(0, 2);
    stor::I2OChain frag3(ref);
    double creationTime3 = frag3.creationTime();
    double lastFragmentTime3 = frag3.lastFragmentTime();

    try
      {
        frag1.addToChain(frag3);
        CPPUNIT_ASSERT(false);
      }
    catch (stor::exception::I2OChain& excpt)
      {
      }
    CPPUNIT_ASSERT(!frag1.empty());
    CPPUNIT_ASSERT(!frag3.empty());
    CPPUNIT_ASSERT(frag1.complete());
    CPPUNIT_ASSERT(!frag3.complete());
    CPPUNIT_ASSERT(!frag1.faulty());
    CPPUNIT_ASSERT(!frag3.faulty());

    CPPUNIT_ASSERT(frag1.creationTime() == creationTime2);
    CPPUNIT_ASSERT(frag3.creationTime() == creationTime3);
    CPPUNIT_ASSERT(frag1.lastFragmentTime() > lastFragmentTime1);
    CPPUNIT_ASSERT(frag3.lastFragmentTime() == lastFragmentTime3);
  }
  CPPUNIT_ASSERT(outstanding_bytes() == 0);
  {
    // add three fragments together (normal order)

    Reference* ref = allocate_frame_with_sample_header(0, 3);
    stor::I2OChain frag1(ref);
    double creationTime1 = frag1.creationTime();

    ref = allocate_frame_with_sample_header(1, 3);
    stor::I2OChain frag2(ref);
    double lastFragmentTime2 = frag2.lastFragmentTime();

    ref = allocate_frame_with_sample_header(2, 3);
    stor::I2OChain frag3(ref);
    double lastFragmentTime3 = frag3.lastFragmentTime();

    frag1.addToChain(frag2);
    CPPUNIT_ASSERT(!frag1.empty());
    CPPUNIT_ASSERT(frag2.empty());
    CPPUNIT_ASSERT(!frag1.complete());
    CPPUNIT_ASSERT(!frag2.complete());
    CPPUNIT_ASSERT(!frag1.faulty());
    CPPUNIT_ASSERT(!frag2.faulty());

    CPPUNIT_ASSERT(frag1.creationTime() == creationTime1);
    CPPUNIT_ASSERT(frag1.lastFragmentTime() > lastFragmentTime2);

    frag1.addToChain(frag3);

    CPPUNIT_ASSERT(!frag1.empty());
    CPPUNIT_ASSERT(frag3.empty());
    CPPUNIT_ASSERT(frag1.complete());
    CPPUNIT_ASSERT(!frag3.complete());
    CPPUNIT_ASSERT(!frag1.faulty());
    CPPUNIT_ASSERT(!frag3.faulty());

    CPPUNIT_ASSERT(frag1.getFragmentID(0) == 0);
    CPPUNIT_ASSERT(frag1.getFragmentID(1) == 1);
    CPPUNIT_ASSERT(frag1.getFragmentID(2) == 2);

    CPPUNIT_ASSERT(frag1.creationTime() == creationTime1);
    CPPUNIT_ASSERT(frag1.lastFragmentTime() > lastFragmentTime3);
  }
  CPPUNIT_ASSERT(outstanding_bytes() == 0);
  {
    // add three fragments together (reverse order)

    Reference* ref = allocate_frame_with_sample_header(0, 3);
    stor::I2OChain frag3(ref);
    double creationTime3 = frag3.creationTime();
    double lastFragmentTime3 = frag3.lastFragmentTime();

    ref = allocate_frame_with_sample_header(1, 3);
    stor::I2OChain frag2(ref);
    double creationTime2 = frag2.creationTime();

    ref = allocate_frame_with_sample_header(2, 3);
    stor::I2OChain frag1(ref);
    double lastFragmentTime1 = frag1.lastFragmentTime();

    frag1.addToChain(frag2);
    CPPUNIT_ASSERT(!frag1.empty());
    CPPUNIT_ASSERT(frag2.empty());
    CPPUNIT_ASSERT(!frag1.complete());
    CPPUNIT_ASSERT(!frag2.complete());
    CPPUNIT_ASSERT(!frag1.faulty());
    CPPUNIT_ASSERT(!frag2.faulty());

    CPPUNIT_ASSERT(frag1.creationTime() == creationTime2);
    CPPUNIT_ASSERT(frag1.lastFragmentTime() > lastFragmentTime1);

    frag1.addToChain(frag3);

    CPPUNIT_ASSERT(!frag1.empty());
    CPPUNIT_ASSERT(frag3.empty());
    CPPUNIT_ASSERT(frag1.complete());
    CPPUNIT_ASSERT(!frag3.complete());
    CPPUNIT_ASSERT(!frag1.faulty());
    CPPUNIT_ASSERT(!frag3.faulty());

    CPPUNIT_ASSERT(frag1.getFragmentID(0) == 0);
    CPPUNIT_ASSERT(frag1.getFragmentID(1) == 1);
    CPPUNIT_ASSERT(frag1.getFragmentID(2) == 2);

    CPPUNIT_ASSERT(frag1.creationTime() == creationTime3);
    CPPUNIT_ASSERT(frag1.lastFragmentTime() > lastFragmentTime3);
  }
  CPPUNIT_ASSERT(outstanding_bytes() == 0);
  {
    // add three fragments together (mixed order)

    Reference* ref = allocate_frame_with_sample_header(2, 3);
    stor::I2OChain frag1(ref);
    double creationTime1 = frag1.creationTime();

    ref = allocate_frame_with_sample_header(0, 3);
    stor::I2OChain frag2(ref);
    double lastFragmentTime2 = frag2.lastFragmentTime();

    ref = allocate_frame_with_sample_header(1, 3);
    stor::I2OChain frag3(ref);
    double lastFragmentTime3 = frag3.lastFragmentTime();

    frag1.addToChain(frag2);
    CPPUNIT_ASSERT(!frag1.empty());
    CPPUNIT_ASSERT(frag2.empty());
    CPPUNIT_ASSERT(!frag1.complete());
    CPPUNIT_ASSERT(!frag2.complete());
    CPPUNIT_ASSERT(!frag1.faulty());
    CPPUNIT_ASSERT(!frag2.faulty());

    CPPUNIT_ASSERT(frag1.creationTime() == creationTime1);
    CPPUNIT_ASSERT(frag1.lastFragmentTime() > lastFragmentTime2);

    frag1.addToChain(frag3);

    CPPUNIT_ASSERT(!frag1.empty());
    CPPUNIT_ASSERT(frag3.empty());
    CPPUNIT_ASSERT(frag1.complete());
    CPPUNIT_ASSERT(!frag3.complete());
    CPPUNIT_ASSERT(!frag1.faulty());
    CPPUNIT_ASSERT(!frag3.faulty());

    CPPUNIT_ASSERT(frag1.getFragmentID(0) == 0);
    CPPUNIT_ASSERT(frag1.getFragmentID(1) == 1);
    CPPUNIT_ASSERT(frag1.getFragmentID(2) == 2);

    CPPUNIT_ASSERT(frag1.creationTime() == creationTime1);
    CPPUNIT_ASSERT(frag1.lastFragmentTime() > lastFragmentTime3);
  }
  CPPUNIT_ASSERT(outstanding_bytes() == 0);
  {
    // verify that adding duplicate frames makes a chain faulty

    Reference* ref = allocate_frame_with_sample_header(1, 3);
    stor::I2OChain frag1(ref);
    double creationTime1 = frag1.creationTime();

    ref = allocate_frame_with_sample_header(1, 3);
    stor::I2OChain frag2(ref);
    double lastFragmentTime2 = frag2.lastFragmentTime();

    frag1.addToChain(frag2);
    CPPUNIT_ASSERT(!frag1.empty());
    CPPUNIT_ASSERT(frag2.empty());
    CPPUNIT_ASSERT(!frag1.complete());
    CPPUNIT_ASSERT(!frag2.complete());
    CPPUNIT_ASSERT(frag1.faulty());
    CPPUNIT_ASSERT(!frag2.faulty());

    CPPUNIT_ASSERT(frag1.creationTime() == creationTime1);
    CPPUNIT_ASSERT(frag1.lastFragmentTime() > lastFragmentTime2);

    // verify that adding a fragment to a faulty chain works

    ref = allocate_frame_with_sample_header(0, 3);
    stor::I2OChain frag3(ref);
    double lastFragmentTime3 = frag3.lastFragmentTime();

    frag1.addToChain(frag3);
    CPPUNIT_ASSERT(!frag1.empty());
    CPPUNIT_ASSERT(frag3.empty());
    CPPUNIT_ASSERT(!frag1.complete());
    CPPUNIT_ASSERT(!frag3.complete());
    CPPUNIT_ASSERT(frag1.faulty());
    CPPUNIT_ASSERT(!frag3.faulty());

    CPPUNIT_ASSERT(frag1.fragmentCount() == 3);
    CPPUNIT_ASSERT(frag1.getFragmentID(0) == 1);
    CPPUNIT_ASSERT(frag1.getFragmentID(1) == 1);
    CPPUNIT_ASSERT(frag1.getFragmentID(2) == 0);

    CPPUNIT_ASSERT(frag1.creationTime() == creationTime1);
    CPPUNIT_ASSERT(frag1.lastFragmentTime() > lastFragmentTime3);
  }
  CPPUNIT_ASSERT(outstanding_bytes() == 0);
  {
    // verify that adding a fragment to an empty chain throws an exception

    Reference* ref = allocate_frame_with_sample_header(0, 3);
    stor::I2OChain frag1(ref);
    stor::I2OChain frag2;

    try
      {
        frag2.addToChain(frag1);
        CPPUNIT_ASSERT(false);
      }
    catch (stor::exception::I2OChain& excpt)
      {
      }
    CPPUNIT_ASSERT(frag2.empty());
    CPPUNIT_ASSERT(!frag1.empty());
    CPPUNIT_ASSERT(!frag1.complete());
    CPPUNIT_ASSERT(!frag2.complete());
    CPPUNIT_ASSERT(!frag1.faulty());
    CPPUNIT_ASSERT(!frag2.faulty());
  }
  CPPUNIT_ASSERT(outstanding_bytes() == 0);
  {
    // verify that adding a faulty chain to a chain works

    Reference* ref = allocate_frame_with_sample_header(0, 2);
    stor::I2OChain frag1(ref);
    double creationTime1 = frag1.creationTime();

    ref = allocate_frame_with_sample_header(1, 2);
    stor::I2OChain frag2(ref);
    double lastFragmentTime2 = frag2.lastFragmentTime();
    frag2.markFaulty();
    CPPUNIT_ASSERT(!frag1.faulty());
    CPPUNIT_ASSERT(frag2.faulty());
    frag1.addToChain(frag2);
    CPPUNIT_ASSERT(frag2.empty());
    CPPUNIT_ASSERT(!frag1.empty());
    CPPUNIT_ASSERT(!frag1.complete());
    CPPUNIT_ASSERT(!frag2.complete());
    CPPUNIT_ASSERT(frag1.faulty());
    CPPUNIT_ASSERT(!frag2.faulty());

    CPPUNIT_ASSERT(frag1.creationTime() == creationTime1);
    CPPUNIT_ASSERT(frag1.lastFragmentTime() > lastFragmentTime2);
  }
  CPPUNIT_ASSERT(outstanding_bytes() == 0);

}


// This macro writes the 'main' for this test.
CPPUNIT_TEST_SUITE_REGISTRATION(testI2OChain);


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
