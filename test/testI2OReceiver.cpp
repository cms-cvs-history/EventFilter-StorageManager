/*
   Author: Harry Cheung, FNAL

   Description:
     XDAQ application to test the I2O output module. It can receive
     I2O frames and write out a streamer data file. This streamer
     data file can be converted back to a standard root file by
     the test streamer file reader. This version can only handle
     collecting from one sender.
     See CMS EventFilter wiki page for further notes.

   Modification:
     version 1.1 2005/11/23
       Initial implementation, only handles one sender, need to
       handle multiple registry frames and HLT ids. Also the
       "other message" frame is hardwired to terminate the run
       and close the output file. Cannot handle if this terminate
       message comes before the last data frame!
*/

#include "EventFilter/StorageManager/test/testI2OReceiver.h"

#include "FWCore/Framework/interface/EventProcessor.h"
#include "FWCore/Utilities/interface/ProblemTracker.h" 
#include "FWCore/Utilities/interface/DebugMacros.h"
#include "FWCore/Utilities/interface/Exception.h"

#include "EventFilter/StorageManager/interface/i2oStorageManagerMsg.h"

#include "xcept/tools.h"

#include "i2o/Method.h"
#include "i2o/utils/include/i2o/utils/AddressMap.h"

#include <exception>
#include <iostream>
#include <string>
#include <vector>

using namespace edm;
using namespace std;

// -----------------------------------------------

struct SMI2ORecFrames   // used to store one event
{
  SMI2ORecFrames(unsigned int numFramesToAllocate, unsigned int eventID);

  unsigned int totFrames_;    // number of frames in this fragment
  unsigned int currFrames_;   // current frames received
  unsigned int eventIdent_;   // Event Identifier (need also HLT identifier)
  vector<toolbox::mem::Reference*> frameRefs_; // vector of frame reference pointers
};

SMI2ORecFrames::SMI2ORecFrames(unsigned int numFramesToAllocate,
                               unsigned int eventID):
  totFrames_(numFramesToAllocate), currFrames_(0), eventIdent_(eventID),
  frameRefs_(totFrames_, 0)
{
  FDEBUG(11) << "testI2OReceiver: Making a SMI2ORecFrames struct with " << numFramesToAllocate
             << " frames to store" << std::endl;
  FDEBUG(11) << "testI2OReceiver: checking = " << frameRefs_.size() << std::endl;
}

testI2OReceiver::testI2OReceiver(xdaq::ApplicationStub * s)
  throw (xdaq::exception::Exception): xdaq::Application(s)
{
    LOG4CPLUS_INFO(this->getApplicationLogger(),"Making testI2OReceiver");

  // use the default vector constructor for SMframeFragments_ with no elements
  filename_ = "smi2ostreamout.dat";
  ost_.open(filename_.c_str(),ios_base::binary | ios_base::out);
  if(!ost_)
  {
    // how do I throw exceptions in the online?
    //throw cms::Exception("Configuration","testI2OReceiver")
    //  << "cannot open file " << filename_;
    std::cerr << "testI2OReceiver: Cannot open file " << filename_ << std::endl;
  }

  // Bind specific messages to functions
  i2o::bind(this,
            &testI2OReceiver::receiveRegistryMessage,
            I2O_SM_PREAMBLE,
            XDAQ_ORGANIZATION_ID);
  i2o::bind(this,
            &testI2OReceiver::receiveDataMessage,
            I2O_SM_DATA,
            XDAQ_ORGANIZATION_ID);
  i2o::bind(this,
            &testI2OReceiver::receiveOtherMessage,
            I2O_SM_OTHER,
            XDAQ_ORGANIZATION_ID);
}

void testI2OReceiver::receiveRegistryMessage(toolbox::mem::Reference *ref)
{
  I2O_MESSAGE_FRAME         *stdMsg =
    (I2O_MESSAGE_FRAME*)ref->getDataLocation();
  I2O_SM_PREAMBLE_MESSAGE_FRAME *msg    =
    (I2O_SM_PREAMBLE_MESSAGE_FRAME*)stdMsg;
  FDEBUG(10) << "testI2OReceiver: Received registry message from HLT " << msg->hltID  << std::endl;
  FDEBUG(10) << "testI2OReceiver: registry size " << msg->dataSize << "\n";
  // can get rid of this if not dumping the data for checking
  std::string temp4print(msg->data,msg->dataSize);
  FDEBUG(10) << "testI2OReceiver: registry data = " << temp4print << std::endl;
  // write out the registry
  int sz = msg->dataSize;
  ost_.write((const char*)(&sz),sizeof(int));
  ost_.write((const char*)msg->data,sz);
  // release the frame buffer now that we are finished
  ref->release();

}

void testI2OReceiver::receiveDataMessage(toolbox::mem::Reference *ref)
{
  I2O_MESSAGE_FRAME         *stdMsg =
    (I2O_MESSAGE_FRAME*)ref->getDataLocation();
  I2O_SM_DATA_MESSAGE_FRAME *msg    =
    (I2O_SM_DATA_MESSAGE_FRAME*)stdMsg;
  FDEBUG(11) << "testI2OReceiver: Received data message from HLT " << msg->hltID 
             << " event " << msg->eventID
             << " total frames = " << msg->numFrames << std::endl;
  FDEBUG(11) << "testI2OReceiver: Frame " << msg->frameCount << " of " 
             << msg->numFrames-1 << std::endl;
  // can get rid of this if not dumping the data for checking
  //int minlen = 50;
  //if(minlen > (int)msg->dataSize) minlen = (int)msg->dataSize;
  //std::string temp4print(msg->data,minlen);
  //FDEBUG(11) << "testI2OReceiver: data = " << temp4print << std::endl;

  // save this frame fragment
  if(SMframeFragments_.size() > 0)
  {
    // see if this event already has some frames stored
    int eventFound = 0;
    vector<SMI2ORecFrames>::iterator foundPos;
    for(vector<SMI2ORecFrames>::iterator pos = SMframeFragments_.begin(); 
        pos != SMframeFragments_.end(); ++pos)
    {
      if(pos->eventIdent_ == (unsigned int)msg->eventID)
      { // should check there are no entries with duplicate event ids
        eventFound = 1;
        foundPos = pos;
      }
    }
    if(eventFound == 0)
    {
      SMI2ORecFrames *recframe = new SMI2ORecFrames((unsigned int)msg->numFrames,
                                                   (unsigned int)msg->eventID);
      SMframeFragments_.push_back(*recframe);
      SMframeFragments_.back().totFrames_ = msg->numFrames;
      SMframeFragments_.back().currFrames_ = 1;
      SMframeFragments_.back().eventIdent_ = msg->eventID;
      SMframeFragments_.back().frameRefs_[msg->frameCount] = ref;
      // now should check if frame is complete and deal with it
      vector<SMI2ORecFrames>::iterator testPos = --SMframeFragments_.end();
      testCompleteChain(testPos);
    } 
    else
    {
      FDEBUG(11) << "testI2OReceiver: found another frame " << msg->frameCount << " for event "
                 << foundPos->eventIdent_ << std::endl;
      // should really check this is not a duplicate frame
      // should test total frames is the same, and other tests are possible
      foundPos->currFrames_++;
      foundPos->frameRefs_[msg->frameCount] = ref;
      // now should check if frame is complete and deal with it
      testCompleteChain(foundPos);
    }
  }
  else
  {
    SMI2ORecFrames *recframe = new SMI2ORecFrames((unsigned int)msg->numFrames,
                                                 (unsigned int)msg->eventID);
    SMframeFragments_.push_back(*recframe);
    SMframeFragments_.back().totFrames_ = msg->numFrames;
    SMframeFragments_.back().currFrames_ = 1;
    SMframeFragments_.back().eventIdent_ = msg->eventID;
    SMframeFragments_.back().frameRefs_[msg->frameCount] = ref;
    // now should check if frame is complete and deal with it
    vector<SMI2ORecFrames>::iterator testPos = --SMframeFragments_.end();
    testCompleteChain(testPos);
  }

  FDEBUG(11) << "testI2OReceiver: SMframeFragments_ size is " << SMframeFragments_.size() << std::endl;
  // don't release buffers until the all frames from a chain are received
  // this is done in testCompleteChain

}

void testI2OReceiver::receiveOtherMessage(toolbox::mem::Reference *ref)
{
  I2O_MESSAGE_FRAME         *stdMsg =
    (I2O_MESSAGE_FRAME*)ref->getDataLocation();
  I2O_SM_OTHER_MESSAGE_FRAME *msg    =
    (I2O_SM_OTHER_MESSAGE_FRAME*)stdMsg;
  FDEBUG(10) << "testI2OReceiver: Received other message from HLT " << msg->hltID << std::endl;
  FDEBUG(10) << "testI2OReceiver: message content " << msg->otherData << "\n";
  // hardwire to mean received halt message so close the file
  // should really be checking if more event data from incomplete events are
  // still coming or new events that somehow came after the other message packet!
  std::cout << "testI2OReceiver: Received halt message. Closing file " << filename_ << std::endl;
  ost_.close();
  // release the frame buffer now that we are finished
  ref->release();
}

void testI2OReceiver::testCompleteChain(vector<SMI2ORecFrames>::iterator pos)
{
  // See if the SMI2ORecFrames pointed to by pos contains a complete chain
  // of I2O frames, if so reconstruct the chain, deal with the chain
  // then free the chain buffer and then destroy this SMI2ORecFrames
  if(pos->currFrames_ == pos->totFrames_)
  {
    FDEBUG(11) << "testI2OReceiver: Received fragment completes a chain that has " << pos->totFrames_
               << " frames " << std::endl;
    toolbox::mem::Reference *head = 0;
    toolbox::mem::Reference *tail = 0;
    if(pos->totFrames_ > 1)
    {
      FDEBUG(11) << "testI2OReceiver: Remaking the chain" << std::endl;
      for(int i=0; i < (int)(pos->totFrames_)-1 ; i++)
      {
        FDEBUG(11) << "testI2OReceiver: setting next reference for frame " << i << std::endl;
        head = pos->frameRefs_[i];
        tail = pos->frameRefs_[i+1];
        head->setNextReference(tail);
      }
    }
    head = pos->frameRefs_[0];
    FDEBUG(11) << "testI2OReceiver: Original chain remade" << std::endl;
    // Deal with the chain
    writeCompleteChain(head);
    // Destroy the SMI2ORecFrames that was storing these frame pointers
    SMframeFragments_.erase(pos);
    // free the complete chain buffer by freeing the head
    head->release();
  }
}

void testI2OReceiver::writeCompleteChain(toolbox::mem::Reference *head)
{
  // write out a complete chain to a file
  // we only use the head buffer reference so we can reuse this
  // function when integrating the Storage Manager with XDAQ
  //
  FDEBUG(11) << "testI2OReceiver: Write out data from chain" << std::endl;
  I2O_MESSAGE_FRAME         *stdMsg =
    (I2O_MESSAGE_FRAME*)head->getDataLocation();
  I2O_SM_DATA_MESSAGE_FRAME *msg    =
    (I2O_SM_DATA_MESSAGE_FRAME*)stdMsg;

  // Remake the data buffer from the chain
  // Use chain info to test exact method used in local transport version
  FDEBUG(11) << "testI2OReceiver: Number of frames in data chain = " << msg->numFrames << std::endl;
  if(msg->numFrames > 1)
  {
    FDEBUG(11) << "testI2OReceiver: populating event data buffer from chain for event "
               << msg->eventID << std::endl;
    // get total size and check with original size
    int origsize = msg->originalSize;
    // should check the size is correct before defining and filling array!!
    //char tempbuffer[origsize];   // ISO C++ does not support variable size arrays!
    // limit size of event data to 5MB (20 frames), test if this is okay
    char tempbuffer[MAX_I2O_SM_DATAFRAMES * MAX_I2O_SM_DATASIZE];
    int maxsize = MAX_I2O_SM_DATAFRAMES * MAX_I2O_SM_DATASIZE;
    if(origsize > maxsize)
      std::cerr << "testI2OReceiver: error event data size = " << origsize << " larger than "
                << " max size = " << maxsize << std::endl;
    FDEBUG(11) << "testI2OReceiver: getting data for frame 0" << std::endl;
    FDEBUG(11) << "testI2OReceiver: datasize = " << msg->dataSize << std::endl;
    int sz = msg->dataSize;
    for(int j=0; j < sz; j++)
      tempbuffer[j] = msg->data[j];
    int next_index = sz;
    toolbox::mem::Reference *curr = 0;
    toolbox::mem::Reference *prev = head;
    for(int i=0; i < (int)(msg->numFrames)-1 ; i++)
    {
      FDEBUG(11) << "testI2OReceiver: getting data for frame " << i+1 << std::endl;
      curr = prev->getNextReference();

      I2O_MESSAGE_FRAME         *stdMsgcurr =
        (I2O_MESSAGE_FRAME*)curr->getDataLocation();
      I2O_SM_DATA_MESSAGE_FRAME *msgcurr    =
        (I2O_SM_DATA_MESSAGE_FRAME*)stdMsgcurr;

      FDEBUG(11) << "testI2OReceiver: datasize = " << msgcurr->dataSize << std::endl;
      int sz = msgcurr->dataSize;
      for(int j=0; j < sz; j++)
        tempbuffer[next_index+j] = msgcurr->data[j];
      next_index = next_index + sz;
      prev = curr;
    }
    // tempbuffer is filled with whole chain data
    FDEBUG(11) << "testI2OReceiver: Belated checking data size = " << next_index << " original size = " 
               << msg->originalSize << std::endl;
    if(next_index == (int)origsize)
    {
      ost_.write((const char*)(&origsize),sizeof(int));
      ost_.write((const char*)tempbuffer,origsize);
    }
    else
    {
      std::cerr << "testI2OReceiver: data size " << next_index << " not equal to original size = " 
                << msg->originalSize << " for event "
                << msg->eventID << std::endl;
      std::cerr << "testI2OReceiver: Change this test to before filling array!!" << std::endl;
    }
  }
  else  // only one frame for this event data
  {
    int sz = msg->dataSize;
    // check size with original size
    if(sz == (int)msg->originalSize)
    {
      ost_.write((const char*)(&sz),sizeof(int));
      ost_.write((const char*)msg->data,sz);
    }
    else
    {
      std::cerr << "testI2OReceiver: one frame data size " << sz 
                << " not equal to original size = " 
                << msg->originalSize << " for event "
                << msg->eventID << std::endl;
    }
  }
}

/**
 * Provides factory method for the instantiation of SM applications
 */

extern "C" xdaq::Application * instantiate_testI2OReceiver(xdaq::ApplicationStub * stub )
{
        std::cout << "Going to construct a testI2OReceiver instance " << endl;
        return new testI2OReceiver(stub);
}

