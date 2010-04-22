// $Id: TestHelper.h,v 1.3.6.1 2010/04/21 09:39:55 mommsen Exp $

#ifndef StorageManager_TestHelper_h
#define StorageManager_TestHelper_h

///////////////////////////////////////////////////
// Collection of helper function for test suites //
///////////////////////////////////////////////////

#include <fstream>
#include <sstream>
#include <string>

#include "DataFormats/Common/interface/HLTenums.h"
#include "EventFilter/StorageManager/interface/I2OChain.h"
#include "EventFilter/Utilities/interface/i2oEvfMsgs.h"
#include "FWCore/Utilities/interface/Adler32Calculator.h"
#include "IOPool/Streamer/interface/MsgHeader.h"
#include "IOPool/Streamer/interface/InitMsgBuilder.h"
#include "IOPool/Streamer/interface/EventMsgBuilder.h"

#include "IOPool/Streamer/interface/DQMEventMsgBuilder.h"
#include "DataFormats/Provenance/interface/Timestamp.h"

#include "toolbox/mem/HeapAllocator.h"
#include "toolbox/mem/Reference.h"
#include "toolbox/mem/MemoryPoolFactory.h"
#include "toolbox/mem/exception/Exception.h"

#include "zlib.h"

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
    typedef std::vector<Reference*> References;

    void
    set_trigger_bit
    (
      std::vector<unsigned char>& hltBits,
      const uint32 bitIndex,
      const edm::hlt::HLTState pathStatus
    )
    {
      // ensure that bit vector is large enough
      uint32 minBitCount = bitIndex + 1;
      uint32 minSize = 1 + ((minBitCount - 1) / 4);
      if (hltBits.size() < minSize) hltBits.resize(minSize);

      uint32 vectorIndex = (uint32) (bitIndex / 4);
      uint32 shiftCount = 2 * (bitIndex % 4);

      uint32 clearMask = 0xff;
      clearMask ^= 0x3 << shiftCount;

      hltBits[vectorIndex] &= clearMask;
      hltBits[vectorIndex] |= pathStatus << shiftCount;
    }


    void
    clear_trigger_bits
    (
      std::vector<unsigned char>& hltBits
    )
    {
      for (unsigned int idx = 0; idx < hltBits.size(); ++idx)
        {
          hltBits[idx] = 0;
        }
    }


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
      const unsigned short code,
      const unsigned int frameIndex,
      const unsigned int totalFrameCount
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
      smMsg->dataSize = bufferSize - sizeof(I2O_SM_PREAMBLE_MESSAGE_FRAME);
      
      return temp;
    }
    
    Reference*
    allocate_frame_with_sample_header
    (
      const unsigned int frameIndex,
      const unsigned int totalFrameCount,
      const unsigned int rbBufferId
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
    

    Reference*
    allocate_frame_with_init_msg
    (
      const std::string requestedOMLabel
    )
    {
      char psetid[] = "1234567890123456";
      Strings hlt_names;
      Strings hlt_selections;
      Strings l1_names;

      hlt_names.push_back("a");  hlt_names.push_back("b");
      hlt_names.push_back("c");  hlt_names.push_back("d");
      hlt_names.push_back("e");  hlt_names.push_back("f");
      hlt_names.push_back("g");  hlt_names.push_back("h");
      hlt_names.push_back("i");

      hlt_selections.push_back("a");
      hlt_selections.push_back("c");
      hlt_selections.push_back("e");
      hlt_selections.push_back("g");
      hlt_selections.push_back("i");

      l1_names.push_back("t10");  l1_names.push_back("t11");

      char reltag[]="CMSSW_3_0_0_pre7";
      std::string processName = "HLT";
      std::string outputModuleLabel = requestedOMLabel;

      uLong crc = crc32(0L, Z_NULL, 0);
      Bytef* crcbuf = (Bytef*) outputModuleLabel.data();
      unsigned int outputModuleId =
        crc32(crc,crcbuf,outputModuleLabel.length());

      unsigned int value1 = 0xa5a5d2d2;
      unsigned int value2 = 0xb4b4e1e1;
      unsigned int value3 = 0xc3c3f0f0;

      Reference* ref = allocate_frame_with_basic_header(I2O_SM_PREAMBLE, 0, 1);
      I2O_SM_PREAMBLE_MESSAGE_FRAME *smMsg =
        (I2O_SM_PREAMBLE_MESSAGE_FRAME*) ref->getDataLocation();
      smMsg->hltTid = value1;
      smMsg->rbBufferID = 2;
      smMsg->outModID = outputModuleId;
      smMsg->fuProcID = value2;
      smMsg->fuGUID = value3;

      InitMsgBuilder
        initBuilder(smMsg->dataPtr(), smMsg->dataSize, 100,
                    Version(7,(const uint8*)psetid), (const char*) reltag,
                    processName.c_str(), outputModuleLabel.c_str(),
                    outputModuleId, hlt_names, hlt_selections, l1_names);

      return ref;
    }


    unsigned int
    get_output_module_id
    (
      const std::string& outputModuleLabel
    )
    {
      uLong crc = crc32(0L, Z_NULL, 0);
      Bytef* crcbuf = (Bytef*) outputModuleLabel.data();
      return crc32(crc,crcbuf,outputModuleLabel.length());
    }


    References
    allocate_multiple_frames_with_event_msg
    (
      const unsigned int runNumber,
      const unsigned int eventNumber,
      const unsigned int lumiNumber,
      const std::string outputModuleLabel,
      std::vector<bool>& l1Bits,
      std::vector<unsigned char>& hltBits,
      const unsigned int hltBitCount,
      const unsigned int rbBufferID,
      const unsigned int hltTid,
      const unsigned int fuProcID,
      const unsigned int fuGUID,
      const unsigned int totalFragments
    )
    {
      int bufferSize = 2000;
      std::vector<unsigned char> tmpBuffer;
      tmpBuffer.resize(bufferSize);
      const uint32_t adler32_chksum = 0;
      char host_name[255];
      gethostname(host_name, 255);
      const unsigned int outputModuleId = get_output_module_id(outputModuleLabel);

      EventMsgBuilder
        eventBuilder(&tmpBuffer[0], bufferSize, runNumber,
                     eventNumber, lumiNumber, outputModuleId,
                     l1Bits, &hltBits[0], hltBitCount,
                     (uint32)adler32_chksum, host_name);
      const uint32 msgSize = eventBuilder.size();
      const uint32 fragmentSize = msgSize/totalFragments + 1;

      References refs;
      for (uint32 idx = 0; idx < totalFragments; ++idx)
        {
          Reference* frame = allocate_frame_with_basic_header(I2O_SM_DATA, idx, totalFragments);
          I2O_SM_DATA_MESSAGE_FRAME *smEventMsg =
            (I2O_SM_DATA_MESSAGE_FRAME*) frame->getDataLocation();
          smEventMsg->hltTid = hltTid;
          smEventMsg->rbBufferID = rbBufferID;
          smEventMsg->runID = runNumber;
          smEventMsg->eventID = eventNumber;
          smEventMsg->outModID = outputModuleId;
          smEventMsg->fuProcID = fuProcID;
          smEventMsg->fuGUID = fuGUID;
          
          const unsigned char* sourceLoc = &tmpBuffer[idx*fragmentSize];
          unsigned long sourceSize = fragmentSize;
          if ((msgSize - idx*fragmentSize) < fragmentSize)
            {
              sourceSize = msgSize - idx*fragmentSize;
            }
          unsigned char* targetLoc = (unsigned char*) smEventMsg->dataPtr();;
          std::copy(sourceLoc, sourceLoc+sourceSize, targetLoc);
          smEventMsg->dataSize = sourceSize;
          smEventMsg->PvtMessageFrame.StdMessageFrame.MessageSize =
            (sourceSize + 3 + sizeof(I2O_SM_DATA_MESSAGE_FRAME)) / 4;

          refs.push_back(frame);
        }

      return refs;
    }


    References
    allocate_multiple_frames_with_event_msg
    (
      const unsigned int eventNumber,
      const unsigned int totalFragments
    )
    {
      const unsigned int runNumber = 100;
      const unsigned int lumiNumber = 777;
      const unsigned int rbBufferID = 3;
      const unsigned int hltTid = 0xa5a5d2d2;
      const unsigned int fuProcID = 0xb4b4e1e1;
      const unsigned int fuGUID = 0xc3c3f0f0;
      const std::string outputModuleLabel = "HLTOutput";

      std::vector<bool> l1Bits;
      l1Bits.push_back(true);
      l1Bits.push_back(false);

      std::vector<unsigned char> hltBits;
      uint32 hltBitCount = 9;
      clear_trigger_bits(hltBits);
      set_trigger_bit(hltBits, 0, edm::hlt::Pass);
      set_trigger_bit(hltBits, 2, edm::hlt::Pass);
      
      References frames =
        allocate_multiple_frames_with_event_msg(
          runNumber, eventNumber, lumiNumber,
          outputModuleLabel, l1Bits, hltBits, hltBitCount,
          rbBufferID, hltTid, fuProcID, fuGUID, totalFragments);

      return frames;
    }


    Reference*
    allocate_frame_with_event_msg
    (
      unsigned int eventNumber,
      std::string outputModuleLabel,
      std::vector<unsigned char>& hltBits,
      unsigned int hltBitCount
    )
    {
      const unsigned int runNumber = 100;
      const unsigned int lumiNumber = 777;
      const unsigned int rbBufferID = 3;
      const unsigned int hltTid = 0xa5a5d2d2;
      const unsigned int fuProcID = 0xb4b4e1e1;
      const unsigned int fuGUID = 0xc3c3f0f0;
  
      std::vector<bool> l1Bits;
      l1Bits.push_back(true);
      l1Bits.push_back(false);
      
      References refs =
        allocate_multiple_frames_with_event_msg(
          runNumber, eventNumber, lumiNumber,
          outputModuleLabel, l1Bits, hltBits, hltBitCount,
          rbBufferID, hltTid, fuProcID, fuGUID, 1);
      
      return refs.front();
    }

    Reference*
    allocate_frame_with_event_msg
    (
      unsigned int eventNumber = 1
    )
    {
      std::vector<unsigned char> hltBits;
      set_trigger_bit(hltBits, 3, edm::hlt::Fail);
      set_trigger_bit(hltBits, 5, edm::hlt::Pass);
      set_trigger_bit(hltBits, 8, edm::hlt::Exception);

      return allocate_frame_with_event_msg(eventNumber, "HLT", hltBits, hltBits.size());
    }

    Reference*
    allocate_frame_with_error_msg
    (
      const unsigned int eventNumber
    )
    {
      unsigned int runNumber = 100;

      unsigned int value1 = 0xa5a5d2d2;
      unsigned int value2 = 0xb4b4e1e1;
      unsigned int value3 = 0xc3c3f0f0;

      Reference* ref = allocate_frame_with_basic_header(I2O_SM_ERROR, 0, 1);
      I2O_SM_DATA_MESSAGE_FRAME *smEventMsg =
        (I2O_SM_DATA_MESSAGE_FRAME*) ref->getDataLocation();
      smEventMsg->hltTid = value1;
      smEventMsg->rbBufferID = 3;
      smEventMsg->runID = runNumber;
      smEventMsg->eventID = eventNumber;
      smEventMsg->outModID = 0xffffffff;
      smEventMsg->fuProcID = value2;
      smEventMsg->fuGUID = value3;

      return ref;
    }


    Reference*
    allocate_frame_with_dqm_msg
    (
      const unsigned int eventNumber,
      const std::string& topFolder = "HLT",
      const unsigned int runNumber = 1111,
      const unsigned int lumiNumber = 1,
      const unsigned int rbBufferID = 1,
      const unsigned int folderID = 0xc3c3f0f0,
      const unsigned int fuProcID = 0x01234567,
      const unsigned int fuGUID = 0x89abcdef,
      const std::string releaseTag = "v00"
    )
    {
      edm::Timestamp ts;
      DQMEvent::TObjectTable mon_elts;

      Reference* ref = allocate_frame_with_basic_header( I2O_SM_DQM, 0, 1 );

      I2O_SM_DQM_MESSAGE_FRAME* smMsg = (I2O_SM_DQM_MESSAGE_FRAME*)ref->getDataLocation();
      smMsg->rbBufferID = rbBufferID;
      smMsg->runID = runNumber;
      smMsg->eventAtUpdateID = eventNumber;
      smMsg->folderID = folderID;
      smMsg->fuProcID = fuProcID;
      smMsg->fuGUID = fuGUID;

      // no data yet to get a checksum (not needed for test)
      uint32_t adler32_chksum = 0;
      char host_name[255];
      gethostname(host_name, 255);

      DQMEventMsgBuilder b( (void*)(smMsg->dataPtr()), smMsg->dataSize,
                            runNumber, eventNumber,
                            ts,
                            lumiNumber, eventNumber,
                            (uint32)adler32_chksum,
                            host_name,
                            releaseTag,
                            topFolder,
                            mon_elts );

      return ref;

    }


    // Return the number of bytes currently allocated out of the
    // (global) Pool.
    size_t
    outstanding_bytes()
    {
      return g_pool->getMemoryUsage().getUsed();
    }


    // Fills the string with content of file.
    // Returns false if an error occurred
    bool
    read_file(const std::string& filename, std::string& content)
    {
      std::ifstream in(filename.c_str());
      if (!in.is_open())
        return false;
      
      std::string line;
      while (std::getline(in, line))
        content.append(line);

      if (!in.eof())
        return false;

      return true;
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
