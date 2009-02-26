// $Id: TestHelper.h,v 1.1.2.2 2009/02/23 19:17:48 biery Exp $

#ifndef StorageManager_TestHelper_h
#define StorageManager_TestHelper_h

///////////////////////////////////////////////////
// Collection of helper function for test suites //
///////////////////////////////////////////////////

#include "EventFilter/StorageManager/interface/I2OChain.h"
#include "EventFilter/Utilities/interface/i2oEvfMsgs.h"
#include "IOPool/Streamer/interface/MsgHeader.h"

#include "toolbox/mem/HeapAllocator.h"
#include "toolbox/mem/Reference.h"
#include "toolbox/mem/MemoryPoolFactory.h"
#include "toolbox/mem/exception/Exception.h"

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


namespace stor
{
  namespace testhelper
  {

    // Allocate a new frame from the (global) Pool.
    Reference*
    allocate_frame()
    {
      const int bufferSize = 1024;
      Reference* temp = g_factory->getFrame(g_pool, bufferSize);
      assert(temp);
      
      unsigned char* tmpPtr = static_cast<unsigned char*>(temp->getDataLocation());
      for (int idx = 0; idx < bufferSize; ++idx)
      {
        tmpPtr[idx] = 0;
      }
      
      return temp;
    }
    
    
    Reference*
    allocate_frame_with_basic_header
    (
      unsigned short code,
      unsigned int frameIndex,
      unsigned int totalFrameCount
    )
    {
      const int bufferSize = 1024;
      Reference* temp = g_factory->getFrame(g_pool, bufferSize);
      assert(temp);
      
      unsigned char* tmpPtr = static_cast<unsigned char*>(temp->getDataLocation());
      for (int idx = 0; idx < bufferSize; ++idx)
      {
        tmpPtr[idx] = 0;
      }
      
      I2O_PRIVATE_MESSAGE_FRAME *pvtMsg =
        (I2O_PRIVATE_MESSAGE_FRAME*) temp->getDataLocation();
      I2O_SM_MULTIPART_MESSAGE_FRAME *smMsg =
        (I2O_SM_MULTIPART_MESSAGE_FRAME*) pvtMsg;
      pvtMsg->StdMessageFrame.MessageSize = bufferSize / 4;
      pvtMsg->XFunctionCode = code;
      smMsg->numFrames = totalFrameCount;
      smMsg->frameCount = frameIndex;
      
      return temp;
    }
    
    Reference*
    allocate_frame_with_sample_header
    (
      unsigned int frameIndex,
      unsigned int totalFrameCount,
      unsigned int rbBufferId
    )
    {
      unsigned int value1 = 0xa5a5d2d2;
      unsigned int value2 = 0xb4b4e1e1;
      unsigned int value3 = 0xc3c3f0f0;
      unsigned int value4 = 0x12345678;
      
      const int bufferSize = 1024;
      Reference* temp = g_factory->getFrame(g_pool, bufferSize);
      assert(temp);
      
      unsigned char* tmpPtr = static_cast<unsigned char*>(temp->getDataLocation());
      for (int idx = 0; idx < bufferSize; ++idx)
      {
        tmpPtr[idx] = 0;
      }
      
      I2O_PRIVATE_MESSAGE_FRAME *pvtMsg =
        (I2O_PRIVATE_MESSAGE_FRAME*) temp->getDataLocation();
      I2O_SM_PREAMBLE_MESSAGE_FRAME *smMsg =
        (I2O_SM_PREAMBLE_MESSAGE_FRAME*) pvtMsg;
      pvtMsg->StdMessageFrame.MessageSize = bufferSize / 4;
      pvtMsg->XFunctionCode = I2O_SM_PREAMBLE;
      smMsg->numFrames = totalFrameCount;
      smMsg->frameCount = frameIndex;
      smMsg->hltTid = value1;
      smMsg->rbBufferID = rbBufferId;
      smMsg->outModID = value2;
      smMsg->fuProcID = value3;
      smMsg->fuGUID = value4;
      smMsg->dataSize = bufferSize - sizeof(I2O_SM_PREAMBLE_MESSAGE_FRAME);
      
      return temp;
    }
    

    // Return the number of bytes currently allocated out of the
    // (global) Pool.
    size_t
    outstanding_bytes()
    {
      return g_pool->getMemoryUsage().getUsed();
    }

  } // namespace testhelper
} // namespace stor

#endif //StorageManager_TestHelper_h


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
