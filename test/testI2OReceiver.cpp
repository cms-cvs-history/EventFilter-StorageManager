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
     version 1.2 2005/12/15
       Added a default page to give statistics. The statistics for
         the memory pool maximum size is not filled.
       Added code to decode the run and event number and use these
         to decide if a fragment from a run/event combination has
         already been received.
     version 1.3 2006/01/24
       Remake the header obtained from first frame, and take off
       the header from all subsequent frames. (Needed by the
       Storage Manager but not for writing out streamer file.)

*/

#include "EventFilter/StorageManager/test/testI2OReceiver.h"

#include "FWCore/Framework/interface/EventProcessor.h"
#include "FWCore/Utilities/interface/ProblemTracker.h" 
#include "FWCore/Utilities/interface/DebugMacros.h"
#include "FWCore/Utilities/interface/Exception.h"
#include "IOPool/Streamer/interface/Messages.h"

#include "EventFilter/StorageManager/interface/i2oStorageManagerMsg.h"

#include "xcept/tools.h"

#include "i2o/Method.h"
#include "i2o/utils/include/i2o/utils/AddressMap.h"

#include "toolbox/mem/Pool.h"
#include "xcept/tools.h"
#include "xgi/Method.h"

#include <exception>
#include <iostream>
#include <string>
#include <vector>

using namespace edm;
using namespace std;

// -----------------------------------------------

struct SMI2ORecFrames   // used to store one event
{
  SMI2ORecFrames(unsigned int numFramesToAllocate, 
                 unsigned long runID, unsigned long eventID);

  unsigned int totFrames_;    // number of frames in this fragment
  unsigned int currFrames_;   // current frames received
  unsigned long runIdent_;    // Run Identifier (need also HLT identifier)
  unsigned long eventIdent_;  // Event Identifier
  vector<toolbox::mem::Reference*> frameRefs_; // vector of frame reference pointers
};

SMI2ORecFrames::SMI2ORecFrames(unsigned int numFramesToAllocate,
                               unsigned long runID, unsigned long eventID):
  totFrames_(numFramesToAllocate), currFrames_(0), runIdent_(runID),
  eventIdent_(eventID), frameRefs_(totFrames_, 0)
{
  FDEBUG(10) << "testI2OReceiver: Making a SMI2ORecFrames struct with " << numFramesToAllocate
             << " frames to store" << std::endl;
  FDEBUG(10) << "testI2OReceiver: checking = " << frameRefs_.size() << std::endl;
}

testI2OReceiver::testI2OReceiver(xdaq::ApplicationStub * s)
  throw (xdaq::exception::Exception): xdaq::Application(s)
{
    LOG4CPLUS_INFO(this->getApplicationLogger(),"Making testI2OReceiver");

  // use the default vector constructor for SMframeFragments_ with no elements
  filename_ = "smi2ostreamout.dat";
  // useful for tests
  //filename_ = "/dev/null";
  ost_.open(filename_.c_str(),ios_base::binary | ios_base::out);
  if(!ost_)
  {
    // how do I throw exceptions in the online?
    //throw cms::Exception("Configuration","testI2OReceiver")
    //  << "cannot open file " << filename_;
    std::cerr << "testI2OReceiver: Cannot open file " << filename_ << std::endl;
  }
  eventcounter_ = 0;
  framecounter_ = 0;
  pool_is_set_ = 0;
  pool_ = 0;

  // for performance measurements
  samples_ = 100; // measurements every 25MB (about)
  databw_ = 0.;
  datarate_ = 0.;
  datalatency_ = 0.;
  totalsamples_ = 0;
  duration_ = 0.;
  meandatabw_ = 0.;
  meandatarate_ = 0.;
  meandatalatency_ = 0.;
  pmeter_ = new stor::SMPerformanceMeter();
  pmeter_->init(samples_);
  maxdatabw_ = 0.;
  mindatabw_ = 999999.;

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

  xgi::bind(this,&testI2OReceiver::defaultWebPage, "Default");
  xgi::bind(this,&testI2OReceiver::css, "styles.css");
}

void testI2OReceiver::receiveRegistryMessage(toolbox::mem::Reference *ref)
{
  // get the pool pointer for statistics if not already set
  if(pool_is_set_ == 0)
  {
    pool_ = ref->getBuffer()->getPool();
   pool_is_set_ = 1;
  }

  I2O_MESSAGE_FRAME         *stdMsg =
    (I2O_MESSAGE_FRAME*)ref->getDataLocation();
  I2O_SM_PREAMBLE_MESSAGE_FRAME *msg    =
    (I2O_SM_PREAMBLE_MESSAGE_FRAME*)stdMsg;
  FDEBUG(10) << "testI2OReceiver: Received registry message from HLT " << msg->hltURL
             << " application " << msg->hltClassName << " id " << msg->hltLocalId
             << " instance " << msg->hltInstance << " tid " << msg->hltTid << std::endl;
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
  
  framecounter_++;

  // for bandwidth performance measurements
  unsigned long actualFrameSize = (unsigned long)sizeof(I2O_SM_PREAMBLE_MESSAGE_FRAME);
  addMeasurement(actualFrameSize);

}

void testI2OReceiver::receiveDataMessage(toolbox::mem::Reference *ref)
{
  // get the pool pointer for statistics
  if(pool_is_set_ == 0)
  {
    pool_ = ref->getBuffer()->getPool();
   pool_is_set_ = 1;
  }

  I2O_MESSAGE_FRAME         *stdMsg =
    (I2O_MESSAGE_FRAME*)ref->getDataLocation();
  I2O_SM_DATA_MESSAGE_FRAME *msg    =
    (I2O_SM_DATA_MESSAGE_FRAME*)stdMsg;
  FDEBUG(10) << "testI2OReceiver: Received data message from HLT at " << msg->hltURL 
             << " application " << msg->hltClassName << " id " << msg->hltLocalId
             << " instance " << msg->hltInstance << " tid " << msg->hltTid << std::endl;
  FDEBUG(10) << "                 for run " << msg->runID << " event " << msg->eventID
             << " total frames = " << msg->numFrames << std::endl;
  FDEBUG(10) << "testI2OReceiver: Frame " << msg->frameCount << " of " 
             << msg->numFrames-1 << std::endl;
  // can get rid of this if not dumping the data for checking
  //int minlen = 50;
  //if(minlen > (int)msg->dataSize) minlen = (int)msg->dataSize;
  //std::string temp4print(msg->data,minlen);
  //FDEBUG(10) << "testI2OReceiver: data = " << temp4print << std::endl;
  // if we want to check InitiatorContext and TransactionContext later
  // currently not used in xdaq 3.3
  //I2O_INITIATOR_CONTEXT sourceContext = stdMsg->InitiatorContext;
  //std::cout << " InitiatorContext = " << sourceContext.LowPart << " " 
  //          << sourceContext.HighPart << std::endl;
  //I2O_TRANSACTION_CONTEXT transContext = msg->PvtMessageFrame.TransactionContext;
  //std::cout << " TransactionContext = " << transContext.LowPart << " " 
  //          << transContext.HighPart << std::endl;

  // save this frame fragment
  if(SMframeFragments_.size() > 0)
  {
    // see if this event already has some frames stored
    int eventFound = 0;
    vector<SMI2ORecFrames>::iterator foundPos;
    for(vector<SMI2ORecFrames>::iterator pos = SMframeFragments_.begin(); 
        pos != SMframeFragments_.end(); ++pos)
    {
      if(pos->eventIdent_ == msg->eventID && pos->runIdent_ == msg->runID)
      { // should check there are no entries with duplicate event ids
        eventFound = 1;
        foundPos = pos;
      }
    }
    if(eventFound == 0)
    {
      SMI2ORecFrames *recframe = new SMI2ORecFrames((unsigned int)msg->numFrames,
                                                   msg->runID, msg->eventID);
      SMframeFragments_.push_back(*recframe);
      SMframeFragments_.back().totFrames_  = msg->numFrames;
      SMframeFragments_.back().currFrames_ = 1;
      SMframeFragments_.back().runIdent_   = msg->runID;
      SMframeFragments_.back().eventIdent_ = msg->eventID;
      SMframeFragments_.back().frameRefs_[msg->frameCount] = ref;
      // now should check if frame is complete and deal with it
      vector<SMI2ORecFrames>::iterator testPos = --SMframeFragments_.end();
      testCompleteChain(testPos);
    } 
    else
    {
      FDEBUG(10) << "testI2OReceiver: found another frame " << msg->frameCount << " for run "
                 << foundPos->runIdent_ << " and event " << foundPos->eventIdent_ << std::endl;
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
                                                 msg->runID, msg->eventID);
    SMframeFragments_.push_back(*recframe);
    SMframeFragments_.back().totFrames_  = msg->numFrames;
    SMframeFragments_.back().currFrames_ = 1;
    SMframeFragments_.back().runIdent_   = msg->runID;
    SMframeFragments_.back().eventIdent_ = msg->eventID;
    SMframeFragments_.back().frameRefs_[msg->frameCount] = ref;
    // now should check if frame is complete and deal with it
    vector<SMI2ORecFrames>::iterator testPos = --SMframeFragments_.end();
    testCompleteChain(testPos);
  }

  FDEBUG(10) << "testI2OReceiver: SMframeFragments_ size is " << SMframeFragments_.size() << std::endl;
  // don't release buffers until the all frames from a chain are received
  // this is done in testCompleteChain

  framecounter_++;

  // for bandwidth performance measurements
  // does not work for chains transferred locally!
  unsigned long actualFrameSize = (unsigned long)sizeof(I2O_SM_DATA_MESSAGE_FRAME);
  addMeasurement(actualFrameSize);
}

void testI2OReceiver::receiveOtherMessage(toolbox::mem::Reference *ref)
{
  // get the pool pointer for statistics
  if(pool_is_set_ == 0)
  {
    pool_ = ref->getBuffer()->getPool();
   pool_is_set_ = 1;
  }

  I2O_MESSAGE_FRAME         *stdMsg =
    (I2O_MESSAGE_FRAME*)ref->getDataLocation();
  I2O_SM_OTHER_MESSAGE_FRAME *msg    =
    (I2O_SM_OTHER_MESSAGE_FRAME*)stdMsg;
  FDEBUG(10) << "testI2OReceiver: Received other message from HLT " << msg->hltURL
             << " application " << msg->hltClassName << " id " << msg->hltLocalId
             << " instance " << msg->hltInstance << " tid " << msg->hltTid << std::endl;
  FDEBUG(10) << "testI2OReceiver: message content " << msg->otherData << "\n";
  // hardwire to mean received halt message so close the file
  // should really be checking if more event data from incomplete events are
  // still coming or new events that somehow came after the other message packet!
  std::cout << "testI2OReceiver: Received halt message. Closing file " << filename_ << std::endl;
  ost_.close();
  // release the frame buffer now that we are finished
  ref->release();

  framecounter_++;

  // for bandwidth performance measurements
  unsigned long actualFrameSize = (unsigned long)sizeof(I2O_SM_OTHER_MESSAGE_FRAME);
  addMeasurement(actualFrameSize);
}

void testI2OReceiver::addMeasurement(unsigned long size)
{
  // for bandwidth performance measurements
  if ( pmeter_->addSample(size) )
  {
    LOG4CPLUS_INFO(getApplicationLogger(),
      toolbox::toString("measured latency: %f for size %d",pmeter_->latency(), size));
    LOG4CPLUS_INFO(getApplicationLogger(),
      toolbox::toString("latency:  %f, rate: %f,bandwidth %f, size: %d\n",
      pmeter_->latency(),pmeter_->rate(),pmeter_->bandwidth(),size));
    // new measurement; so update
    databw_ = pmeter_->bandwidth();
    datarate_ = pmeter_->rate();
    datalatency_ = pmeter_->latency();
    totalsamples_ = pmeter_->totalsamples();
    duration_ = pmeter_->duration();
    meandatabw_ = pmeter_->meanbandwidth();
    meandatarate_ = pmeter_->meanrate();
    meandatalatency_ = pmeter_->meanlatency();
    if(databw_ > maxdatabw_) maxdatabw_ = databw_;
    if(databw_ < mindatabw_) mindatabw_ = databw_;
  }
}

void testI2OReceiver::testCompleteChain(vector<SMI2ORecFrames>::iterator pos)
{
  // See if the SMI2ORecFrames pointed to by pos contains a complete chain
  // of I2O frames, if so reconstruct the chain, deal with the chain
  // then free the chain buffer and then destroy this SMI2ORecFrames
  if(pos->totFrames_ == 1)
  {
    // chain is complete as there is only one frame
    toolbox::mem::Reference *head = 0;
    head = pos->frameRefs_[0];
    FDEBUG(10) << "testI2OReceiver: No chain as only one frame" << std::endl;
    // Write the data out
    writeCompleteChain(head);
    // Destroy the SMI2ORecFrames that was storing these frame pointers
    SMframeFragments_.erase(pos);
    // free the complete chain buffer by freeing the head
    head->release();
    eventcounter_++;
  }
  else
  {
    if(pos->currFrames_ == pos->totFrames_)
    {
      FDEBUG(10) << "testI2OReceiver: Received fragment completes a chain that has " << pos->totFrames_
                 << " frames " << std::endl;
      toolbox::mem::Reference *head = 0;
      toolbox::mem::Reference *tail = 0;
      if(pos->totFrames_ > 1)
      {
        FDEBUG(10) << "testI2OReceiver: Remaking the chain" << std::endl;
        for(int i=0; i < (int)(pos->totFrames_)-1 ; i++)
        {
          FDEBUG(10) << "testI2OReceiver: setting next reference for frame " << i << std::endl;
          head = pos->frameRefs_[i];
          tail = pos->frameRefs_[i+1];
          head->setNextReference(tail);
        }
      }
      head = pos->frameRefs_[0];
      FDEBUG(10) << "testI2OReceiver: Original chain remade" << std::endl;
      // Deal with the chain
      writeCompleteChain(head);
      // Destroy the SMI2ORecFrames that was storing these frame pointers
      SMframeFragments_.erase(pos);
      // free the complete chain buffer by freeing the head
      head->release();
      eventcounter_++;
    }
    else
    {
    // If running with local transfers, a chain of I2O frames when posted only has the
    // head frame sent. So a single frame can complete a chain for local transfers.
    // We need to test for this. Must be head frame and next pointer must exist.
      if(pos->currFrames_ == 1)
      {
        toolbox::mem::Reference *head = 0;
        toolbox::mem::Reference *next = 0;
        head = pos->frameRefs_[0];
        // best to check the complete chain just in case!
        unsigned int tested_frames = 1;
        next = head;
        while((next=next->getNextReference())!=0) tested_frames++;
        FDEBUG(10) << "testI2OReceiver: Head frame has " << tested_frames-1
                   << " linked frames out of " << pos->totFrames_-1 << std::endl;
        if(pos->totFrames_ == tested_frames)
        {
          // found a complete linked chain from the leading frame
          FDEBUG(10) << "testI2OReceiver: Leading frame contains a complete linked chain"
                     << " - must be local transfer" << std::endl;
          // Deal with the chain
          writeCompleteChain(head);
          // Destroy the SMI2ORecFrames that was storing these frame pointers
          SMframeFragments_.erase(pos);
          // free the complete chain buffer by freeing the head
          head->release();
          eventcounter_++;
        }
      }
    }
  }
}

void testI2OReceiver::writeCompleteChain(toolbox::mem::Reference *head)
{
  // write out a complete chain to a file
  // we only use the head buffer reference so we can reuse this
  // function when integrating the Storage Manager with XDAQ
  //
  FDEBUG(10) << "testI2OReceiver: Write out data from chain" << std::endl;
  I2O_MESSAGE_FRAME         *stdMsg =
    (I2O_MESSAGE_FRAME*)head->getDataLocation();
  I2O_SM_DATA_MESSAGE_FRAME *msg    =
    (I2O_SM_DATA_MESSAGE_FRAME*)stdMsg;

  // Remake the data buffer from the chain
  // Use chain info to test exact method used in local transport version
  FDEBUG(10) << "testI2OReceiver: Number of frames in data chain = " << msg->numFrames << std::endl;
  if(msg->numFrames > 1)
  {
    FDEBUG(10) << "testI2OReceiver: populating event data buffer from chain for run "
               << msg->runID << " and event " << msg->eventID << std::endl;
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
    FDEBUG(10) << "testI2OReceiver: getting data for frame 0" << std::endl;
    FDEBUG(10) << "testI2OReceiver: datasize = " << msg->dataSize << std::endl;
    int sz = msg->dataSize;
    for(int j=0; j < sz; j++)
      tempbuffer[j] = msg->data[j];
    // remake the Header for the leading frame/fragment
    EventMsg msgBuffer(tempbuffer,sz);
    edm::RunNumber_t runid     = msgBuffer.getRunNumber();
    edm::EventNumber_t eventid = msgBuffer.getEventNumber();
    char dummyBuffer[MAX_I2O_SM_DATAFRAMES * MAX_I2O_SM_DATASIZE];
    EventMsg msgFrag(dummyBuffer, origsize, eventid, runid, 1, 1);
    unsigned int headerNeededSize = sizeof(MsgCode::Codes)+sizeof(EventMsg::Header);
    for(int j=0; j < (int)headerNeededSize; j++) 
      tempbuffer[j] = dummyBuffer[j];

    int next_index = sz;
    toolbox::mem::Reference *curr = 0;
    toolbox::mem::Reference *prev = head;
    for(int i=0; i < (int)(msg->numFrames)-1 ; i++)
    {
      FDEBUG(10) << "testI2OReceiver: getting data for frame " << i+1 << std::endl;
      curr = prev->getNextReference(); // should test if this exists!

      I2O_MESSAGE_FRAME         *stdMsgcurr =
        (I2O_MESSAGE_FRAME*)curr->getDataLocation();
      I2O_SM_DATA_MESSAGE_FRAME *msgcurr    =
        (I2O_SM_DATA_MESSAGE_FRAME*)stdMsgcurr;

      FDEBUG(10) << "testI2OReceiver: datasize = " << msgcurr->dataSize << std::endl;
      int sz = msgcurr->dataSize;
      //for(int j=0; j < sz; j++)
      //  tempbuffer[next_index+j] = msgcurr->data[j];
      // for now - just take out header for all other frames/fragments
      for(int j=0; j < sz-(int)headerNeededSize; j++)
        tempbuffer[next_index+j] = msgcurr->data[headerNeededSize+j];
      //next_index = next_index + sz;
      next_index = next_index + sz - headerNeededSize;
      prev = curr;
    }
    // tempbuffer is filled with whole chain data
    FDEBUG(10) << "testI2OReceiver: Belated checking data size = " << next_index << " original size = " 
               << msg->originalSize << std::endl;
    if(next_index == (int)origsize)
    {
      ost_.write((const char*)(&origsize),sizeof(int));
      ost_.write((const char*)tempbuffer,origsize);
    }
    else
    {
      std::cerr << "testI2OReceiver: data size " << next_index << " not equal to original size = " 
                << msg->originalSize << " for run "
                << msg->runID << " and event " << msg->eventID << std::endl;
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
                << msg->originalSize << " for run "
                << msg->runID << " and event " << msg->eventID << std::endl;
    }
  }
}

#include <iomanip>

void testI2OReceiver::defaultWebPage(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception)
{
  //std::cout << "default web page called" << std::endl;
  *out << "<html>"                                                   << endl;
  *out << "<head>"                                                   << endl;
  *out << "<link type=\"text/css\" rel=\"stylesheet\"";
  *out << " href=\"/" <<  getApplicationDescriptor()->getURN()
       << "/styles.css\"/>"                   << endl;
  *out << "<title>" << getApplicationDescriptor()->getClassName() << " instance "
       << getApplicationDescriptor()->getInstance()
       << "</title>"     << endl;
    *out << "<table border=\"0\" width=\"100%\">"                      << endl;
    *out << "<tr>"                                                     << endl;
    *out << "  <td align=\"left\">"                                    << endl;
    *out << "    <img"                                                 << endl;
    *out << "     align=\"middle\""                                    << endl;
    *out << "     src=\"/daq/evb/examples/fu/images/fu64x64.gif\""     << endl;
    *out << "     alt=\"main\""                                        << endl;
    *out << "     width=\"64\""                                        << endl;
    *out << "     height=\"64\""                                       << endl;
    *out << "     border=\"\"/>"                                       << endl;
    *out << "    <b>"                                                  << endl;
    *out << getApplicationDescriptor()->getClassName() << " instance "
         << getApplicationDescriptor()->getInstance()                  << endl;
    *out << "    </b>"                                                 << endl;
    *out << "  </td>"                                                  << endl;
    *out << "  <td width=\"32\">"                                      << endl;
    *out << "    <a href=\"/urn:xdaq-application:lid=3\">"             << endl;
    *out << "      <img"                                               << endl;
    *out << "       align=\"middle\""                                  << endl;
    *out << "       src=\"/daq/xdaq/hyperdaq/images/HyperDAQ.jpg\""    << endl;
    *out << "       alt=\"HyperDAQ\""                                  << endl;
    *out << "       width=\"32\""                                      << endl;
    *out << "       height=\"32\""                                      << endl;
    *out << "       border=\"\"/>"                                     << endl;
    *out << "    </a>"                                                 << endl;
    *out << "  </td>"                                                  << endl;
    *out << "  <td width=\"32\">"                                      << endl;
    *out << "  </td>"                                                  << endl;
    *out << "  <td width=\"32\">"                                      << endl;
    *out << "    <a href=\"/" << getApplicationDescriptor()->getURN()
         << "/debug\">"                   << endl;
    *out << "      <img"                                               << endl;
    *out << "       align=\"middle\""                                  << endl;
    *out << "       src=\"/daq/evb/bu/images/debug32x32.gif\""         << endl;
    *out << "       alt=\"debug\""                                     << endl;
    *out << "       width=\"32\""                                      << endl;
    *out << "       height=\"32\""                                     << endl;
    *out << "       border=\"\"/>"                                     << endl;
    *out << "    </a>"                                                 << endl;
    *out << "  </td>"                                                  << endl;
    *out << "</tr>"                                                    << endl;
    *out << "</table>"                                                 << endl;

  *out << "<hr/>"                                                    << endl;
  *out << "<table>"                                                  << endl;
  *out << "<tr valign=\"top\">"                                      << endl;
  *out << "  <td>"                                                   << endl;

  *out << "<table frame=\"void\" rules=\"groups\" class=\"states\">" << std::endl;
  *out << "<colgroup> <colgroup align=\"rigth\">"                    << std::endl;
    *out << "  <tr>"                                                   << endl;
    *out << "    <th colspan=2>"                                       << endl;
    *out << "      " << "Memory Pool Usage"                            << endl;
    *out << "    </th>"                                                << endl;
    *out << "  </tr>"                                                  << endl;

        *out << "<tr>" << std::endl;
        *out << "<th >" << std::endl;
        *out << "Parameter" << std::endl;
        *out << "</th>" << std::endl;
        *out << "<th>" << std::endl;
        *out << "Value" << std::endl;
        *out << "</th>" << std::endl;
        *out << "</tr>" << std::endl;
        *out << "<tr>" << std::endl;
          *out << "<td >" << std::endl;
          *out << "Frames Received" << std::endl;
          *out << "</td>" << std::endl;
          *out << "<td align=right>" << std::endl;
          *out << framecounter_ << std::endl;
          *out << "</td>" << std::endl;
        *out << "  </tr>" << endl;
        *out << "<tr>" << std::endl;
          *out << "<td >" << std::endl;
          *out << "Events Received" << std::endl;
          *out << "</td>" << std::endl;
          *out << "<td align=right>" << std::endl;
          *out << eventcounter_ << std::endl;
          *out << "</td>" << std::endl;
        *out << "  </tr>" << endl;
        *out << "<tr>" << std::endl;
          *out << "<td >" << std::endl;
          *out << "Max Pool Memory (Bytes)" << std::endl;
          *out << "</td>" << std::endl;
          *out << "<td align=right>" << std::endl;
          *out << pool_->getMemoryUsage().getCommitted() << std::endl;
          *out << "</td>" << std::endl;
        *out << "  </tr>" << endl;
        *out << "<tr>" << std::endl;
          *out << "<td >" << std::endl;
          *out << "Memory Used (Bytes)" << std::endl;
          *out << "</td>" << std::endl;
          *out << "<td align=right>" << std::endl;
          *out << pool_->getMemoryUsage().getUsed() << std::endl;
          *out << "</td>" << std::endl;
        *out << "  </tr>" << endl;
        *out << "<tr>" << std::endl;
          *out << "<td >" << std::endl;
          *out << "Percent Memory Used (%)" << std::endl;
          *out << "</td>" << std::endl;
          *out << "<td align=right>" << std::endl;
          double memused = (double)pool_->getMemoryUsage().getUsed();
          double memmax = (double)pool_->getMemoryUsage().getCommitted();
          double percentused = 0.0;
          if(pool_->getMemoryUsage().getCommitted() != 0)
            percentused = 100.0*(memused/memmax);
          *out << std::fixed << std::showpoint
               << std::setw(6) << std::setprecision(2) << percentused << std::endl;
          *out << "</td>" << std::endl;
        *out << "  </tr>" << endl;
// performance statistics
    *out << "  <tr>"                                                   << endl;
    *out << "    <th colspan=2>"                                       << endl;
    *out << "      " << "Performance for last " << samples_ << " frames"<< endl;
    *out << "    </th>"                                                << endl;
    *out << "  </tr>"                                                  << endl;
        *out << "<tr>" << std::endl;
          *out << "<td >" << std::endl;
          *out << "Bandwidth (MB/s)" << std::endl;
          *out << "</td>" << std::endl;
          *out << "<td align=right>" << std::endl;
          *out << databw_ << std::endl;
          *out << "</td>" << std::endl;
        *out << "  </tr>" << endl;
        *out << "<tr>" << std::endl;
          *out << "<td >" << std::endl;
          *out << "Rate (Frames/s)" << std::endl;
          *out << "</td>" << std::endl;
          *out << "<td align=right>" << std::endl;
          *out << datarate_ << std::endl;
          *out << "</td>" << std::endl;
        *out << "  </tr>" << endl;
        *out << "<tr>" << std::endl;
          *out << "<td >" << std::endl;
          *out << "Latency (us/frame)" << std::endl;
          *out << "</td>" << std::endl;
          *out << "<td align=right>" << std::endl;
          *out << datalatency_ << std::endl;
          *out << "</td>" << std::endl;
        *out << "  </tr>" << endl;
        *out << "<tr>" << std::endl;
          *out << "<td >" << std::endl;
          *out << "Maximum Bandwidth (MB/s)" << std::endl;
          *out << "</td>" << std::endl;
          *out << "<td align=right>" << std::endl;
          *out << maxdatabw_ << std::endl;
          *out << "</td>" << std::endl;
        *out << "  </tr>" << endl;
        *out << "<tr>" << std::endl;
          *out << "<td >" << std::endl;
          *out << "Minimum Bandwidth (MB/s)" << std::endl;
          *out << "</td>" << std::endl;
          *out << "<td align=right>" << std::endl;
          *out << mindatabw_ << std::endl;
          *out << "</td>" << std::endl;
        *out << "  </tr>" << endl;
// mean performance statistics for whole run
    *out << "  <tr>"                                                   << endl;
    *out << "    <th colspan=2>"                                       << endl;
    *out << "      " << "Mean Performance for " << totalsamples_ << " frames, duration "
         << duration_ << " seconds" << endl;
    *out << "    </th>"                                                << endl;
    *out << "  </tr>"                                                  << endl;
        *out << "<tr>" << std::endl;
          *out << "<td >" << std::endl;
          *out << "Bandwidth (MB/s)" << std::endl;
          *out << "</td>" << std::endl;
          *out << "<td align=right>" << std::endl;
          *out << meandatabw_ << std::endl;
          *out << "</td>" << std::endl;
        *out << "  </tr>" << endl;
        *out << "<tr>" << std::endl;
          *out << "<td >" << std::endl;
          *out << "Rate (Frames/s)" << std::endl;
          *out << "</td>" << std::endl;
          *out << "<td align=right>" << std::endl;
          *out << meandatarate_ << std::endl;
          *out << "</td>" << std::endl;
        *out << "  </tr>" << endl;
        *out << "<tr>" << std::endl;
          *out << "<td >" << std::endl;
          *out << "Latency (us/frame)" << std::endl;
          *out << "</td>" << std::endl;
          *out << "<td align=right>" << std::endl;
          *out << meandatalatency_ << std::endl;
          *out << "</td>" << std::endl;
        *out << "  </tr>" << endl;

  *out << "</table>" << std::endl;

  *out << "  </td>"                                                  << endl;
  *out << "</table>"                                                 << endl;

  *out << "</body>"                                                  << endl;
  *out << "</html>"                                                  << endl;
}


/**
 * Provides factory method for the instantiation of SM applications
 */

extern "C" xdaq::Application * instantiate_testI2OReceiver(xdaq::ApplicationStub * stub )
{
        std::cout << "Going to construct a testI2OReceiver instance " << endl;
        return new testI2OReceiver(stub);
}

