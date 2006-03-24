/*
   Author: Harry Cheung, FNAL

   Description:
     Storage Manager XDAQ application. It can receive and collect
     I2O frames to remake event data and then processes them
     through an event processor. Two cmsRun style configuration 
     files are normally needed, one for the Storage Manager and
     one for the FUEventProcessor that produced the incoming
     events so that the registry can be made at startup.
     See CMS EventFilter wiki page for further notes.

   Modification:
     version 1.1 2005/11/23
       Initial implementation, the FUEventProcessor configuration
       is not used yet. This prototype uses a sample streamer
       data file that must be located at the running directory
       to get the registry. Also the SM config file name is also
       hardwired rather than read from the XML file.
     version 1.2 2006/01/26
       Changed to use cmsRun configuration contained in the XDAQ
       XML file instead of hardwired files.
     version 1.3 2006/01/31
       Changes to handle local transfers.
     version 1.4 2006/02/16
       Fix Build problems due to changes in the Message Logger.
     version 1.5 2006/02/20
       Added nullAction needed by EPS state machine.
     version 1.6 2006/02/28
       Put testStorageManager into stor:: namespace.
       Added registration and collection of data for FU senders,
       and code to display this info on the default web page.

*/

#include <exception>
#include <iostream>
#include <string>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/unistd.h>

#include "EventFilter/StorageManager/test/testStorageManager.h"

#include "FWCore/Framework/interface/EventProcessor.h"
#include "DataFormats/Common/interface/ProductRegistry.h"
#include "FWCore/Utilities/interface/ProblemTracker.h"
#include "FWCore/Utilities/interface/DebugMacros.h"
#include "FWCore/Utilities/interface/Exception.h"
#include "FWCore/MessageService/interface/MessageServicePresence.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "IOPool/Streamer/interface/HLTInfo.h"
#include "IOPool/Streamer/interface/Utilities.h"
#include "IOPool/Streamer/interface/TestFileReader.h"
#include "EventFilter/StorageManager/interface/JobController.h"
#include "PluginManager/PluginManager.h"

#include "EventFilter/StorageManager/interface/i2oStorageManagerMsg.h"

#include "xcept/tools.h"

#include "i2o/Method.h"
#include "i2o/utils/include/i2o/utils/AddressMap.h"

#include "toolbox/mem/Pool.h"
#include "xcept/tools.h"
#include "xgi/Method.h"

#include "xoap/include/xoap/SOAPEnvelope.h"
#include "xoap/include/xoap/SOAPBody.h"
#include "xoap/include/xoap/domutils.h"


#include <exception>
#include <iostream>
#include <iomanip>

#include "boost/shared_ptr.hpp"

using namespace edm;
using namespace std;

// -----------------------------------------------

static void deleteSMBuffer(void* Ref)
{
  // release the memory pool buffer
  // once the fragment collector is done with it
  stor::FragEntry* entry = (stor::FragEntry*)Ref;
  toolbox::mem::Reference *ref=(toolbox::mem::Reference*)entry->buffer_object_;
  ref->release();
}
// -----------------------------------------------

namespace stor {

using namespace edm;
using namespace std;

struct SMFUSenderList  // used to store list of FU senders
{
  SMFUSenderList(const char* hltURL,
                 const char* hltClassName,
                 const unsigned long hltLocalId,
                 const unsigned long hltInstance,
                 const unsigned long hltTid,
                 const unsigned long registrySize,
                 const char* registryData);

  char          hltURL_[MAX_I2O_SM_URLCHARS];       // FU+HLT identifiers
  char          hltClassName_[MAX_I2O_SM_URLCHARS];
  unsigned long hltLocalId_;
  unsigned long hltInstance_;
  unsigned long hltTid_;
  unsigned long registrySize_;
  char          registryData_[MAX_I2O_REGISTRY_DATASIZE];
  bool          regCheckedOK_;    // Registry checked to be same as configuration
  unsigned int  connectStatus_;   // FU+HLT connection status
  double        lastLatency_;     // Latency of last frame in microseconds
  unsigned long runNumber_;
  bool          isLocal_;         // If detected a locally sent frame chain
  unsigned long framesReceived_;
  unsigned long eventsReceived_;
  unsigned long lastEventID_;
  unsigned long lastRunID_;
  unsigned long lastFrameNum_;
  unsigned long lastTotalFrameNum_;
  unsigned long totalOutOfOrder_;
  unsigned long totalSizeReceived_;// For data only
  unsigned long totalBadEvents_;   // Update meaning: include original size check?
  toolbox::Chrono chrono_;         // Keep latency for connection check
};

SMFUSenderList::SMFUSenderList(const char* hltURL,
                 const char* hltClassName,
                 const unsigned long hltLocalId,
                 const unsigned long hltInstance,
                 const unsigned long hltTid,
                 const unsigned long registrySize,
                 const char* registryData):
  hltLocalId_(hltLocalId), hltInstance_(hltInstance), hltTid_(hltTid),
  registrySize_(registrySize)
{
  copy(hltURL, hltURL+MAX_I2O_SM_URLCHARS, hltURL_);
  copy(hltClassName, hltClassName+MAX_I2O_SM_URLCHARS, hltClassName_);
  copy(registryData, registryData+MAX_I2O_REGISTRY_DATASIZE, registryData_);
  regCheckedOK_ = false;
  /*
     Connect status
     Bit 1 = 0 disconnected (was connected before) or delete it?
           = 1 connected and received registry
     Bit 2 = 0 not yet received a data frame
           = 1 received at least one data frame
  */
  connectStatus_ = 1;
  lastLatency_ = 0.0;
  runNumber_ = 0;
  isLocal_ = false;
  framesReceived_ = 1;
  eventsReceived_ = 0;
  lastEventID_ = 0;
  lastRunID_ = 0;
  lastFrameNum_ = 0;
  lastTotalFrameNum_ = 0;
  totalOutOfOrder_ = 0;
  totalSizeReceived_ = 0;
  totalBadEvents_ = 0;

  FDEBUG(10) << "testStorageManager: Making a SMFUSenderList struct for "
            << hltURL_ << " class " << hltClassName_  << " instance "
            << hltInstance_ << " Tid " << hltTid_ << std::endl;
}

testStorageManager::testStorageManager(xdaq::ApplicationStub * s)
  throw (xdaq::exception::Exception): xdaq::Application(s),
  fsm_(0)//, ah_(0)
{
  LOG4CPLUS_INFO(this->getApplicationLogger(),"Making testStorageManager");

  //ah_ = new edm::AssertHandler();
  fsm_ = new evf::EPStateMachine(getApplicationLogger());
  fsm_->init<testStorageManager>(this);
  xdata::InfoSpace *ispace = getApplicationInfoSpace();
  // default configuration
  ispace->fireItemAvailable("STparameterSet",&offConfig_);
  ispace->fireItemAvailable("FUparameterSet",&fuConfig_);
  ispace->fireItemAvailable("stateName",&fsm_->stateName_);

  // Bind specific messages to functions
  i2o::bind(this,
            &testStorageManager::receiveRegistryMessage,
            I2O_SM_PREAMBLE,
            XDAQ_ORGANIZATION_ID);
  i2o::bind(this,
            &testStorageManager::receiveDataMessage,
            I2O_SM_DATA,
            XDAQ_ORGANIZATION_ID);
  i2o::bind(this,
            &testStorageManager::receiveOtherMessage,
            I2O_SM_OTHER,
            XDAQ_ORGANIZATION_ID);

  // Bind web interface
  xgi::bind(this,&testStorageManager::defaultWebPage, "Default");
  xgi::bind(this,&testStorageManager::css, "styles.css");
  xgi::bind(this,&testStorageManager::fusenderWebPage, "fusenderlist");
  xgi::bind(this,&testStorageManager::eventdataWebPage, "geteventdata");
  xgi::bind(this,&testStorageManager::headerdataWebPage, "getregdata");


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

}

testStorageManager::~testStorageManager()
{
  delete fsm_;
  //delete ah_;
  delete pmeter_;
}

void testStorageManager::configureAction(toolbox::Event::Reference e) 
  throw (toolbox::fsm::exception::Exception)
{
  // Get into the ready state
  // get the configuration here or in enable?

  // do this here? JBK - moved to data member
  // edm::MessageServicePresence theMessageServicePresence;
  seal::PluginManager::get()->initialise();

  //streamer format file to get registry
  // string sample_file("SMSampleFile.dat");
  // SM config file - should be from XML
  // string my_config_file("SMConfig.cfg");
  // for the real thing, give the JobController a configuration string and
  // save the registry data for checking with the one coming over the network

  string sample_config(fuConfig_.value_);
  string my_config(offConfig_.value_);

  // the rethrows below need to be XDAQ exception types (JBK)

  try {
    jc_.reset(new stor::JobController(sample_config,
                  my_config, &deleteSMBuffer));
  }
  catch(cms::Exception& e)
    {
      cerr << "Caught an exception:\n" << e.what() << endl;
      throw;
    }
  catch(seal::Error& e)
    {
      cerr << "Caught an exception:\n" << e.explainSelf() << endl;
      throw;
    }
  catch(std::exception& e)
    {
      cerr << "Caught an exception:\n" << e.what() << endl;
      throw;
    }
  catch(...)
  {
      cerr << "Caught unknown exception\n" << endl;
  }

}

void testStorageManager::enableAction(toolbox::Event::Reference e) 
  throw (toolbox::fsm::exception::Exception)
{
  // Get into running state
  jc_->start();
}

void testStorageManager::haltAction(toolbox::Event::Reference e) 
  throw (toolbox::fsm::exception::Exception)
{
  // Get into halted state
  jc_->stop();
  jc_->join();
  // we must destroy the eventprocessor to finish processing
  jc_.reset();
}

void testStorageManager::suspendAction(toolbox::Event::Reference e) 
  throw (toolbox::fsm::exception::Exception)
{
  // Get into suspend state: but not valid for Storage Manager?!
  LOG4CPLUS_INFO(this->getApplicationLogger(),
    "Suspend state not implemented in testStorageManager");
}

void testStorageManager::resumeAction(toolbox::Event::Reference e) 
  throw (toolbox::fsm::exception::Exception)
{
  // Get into suspend state: but not valid for Storage Manager?!
  LOG4CPLUS_INFO(this->getApplicationLogger(),
    "Resume not implemented in testStorageManager");
}

void testStorageManager::nullAction(toolbox::Event::Reference e) 
  throw (toolbox::fsm::exception::Exception)
{
  //this action has no effect. A warning is issued to this end
  LOG4CPLUS_WARN(this->getApplicationLogger(),
                    "Null action invoked");
}

xoap::MessageReference testStorageManager::fireEvent(xoap::MessageReference msg)
  throw (xoap::exception::Exception)
{
  xoap::SOAPPart     part      = msg->getSOAPPart();
  xoap::SOAPEnvelope env       = part.getEnvelope();
  xoap::SOAPBody     body      = env.getBody();
  DOMNode            *node     = body.getDOMNode();
  DOMNodeList        *bodyList = node->getChildNodes();
  DOMNode            *command  = 0;
  std::string        commandName;

  for (unsigned int i = 0; i < bodyList->getLength(); i++)
    {
      command = bodyList->item(i);

      if(command->getNodeType() == DOMNode::ELEMENT_NODE)
        {
          commandName = xoap::XMLCh2String(command->getLocalName());
          return fsm_->processFSMCommand(commandName);
        }
    }

  XCEPT_RAISE(xoap::exception::Exception, "Command not found");
}

////////////////////////////// I2O frame call back functions ///////////////

void testStorageManager::receiveRegistryMessage(toolbox::mem::Reference *ref)
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
  FDEBUG(10) << "testStorageManager: Received registry message from HLT " << msg->hltURL
             << " application " << msg->hltClassName << " id " << msg->hltLocalId
             << " instance " << msg->hltInstance << " tid " << msg->hltTid << std::endl;
  FDEBUG(10) << "testStorageManager: registry size " << msg->dataSize << "\n";
  // can get rid of this if not dumping the data for checking
  std::string temp4print(msg->data,msg->dataSize);
  FDEBUG(10) << "testStorageManager: registry data = " << temp4print << std::endl;
  // should be checking the registry with the one being used
  // release the frame buffer now that we are finished
  ref->release();
  
  framecounter_++;

  // for bandwidth performance measurements
  unsigned long actualFrameSize = (unsigned long)sizeof(I2O_SM_PREAMBLE_MESSAGE_FRAME);
  addMeasurement(actualFrameSize);

  // register this FU sender into the list to keep its status
  registerFUSender(&msg->hltURL[0], &msg->hltClassName[0],
                 msg->hltLocalId, msg->hltInstance, msg->hltTid,
                 msg->dataSize, &msg->data[0]);
}

void testStorageManager::receiveDataMessage(toolbox::mem::Reference *ref)
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
  FDEBUG(10) << "testStorageManager: Received data message from HLT at " << msg->hltURL 
             << " application " << msg->hltClassName << " id " << msg->hltLocalId
             << " instance " << msg->hltInstance << " tid " << msg->hltTid << std::endl;
  FDEBUG(10) << "                 for run " << msg->runID << " event " << msg->eventID
             << " total frames = " << msg->numFrames << std::endl;
  FDEBUG(10) << "testStorageManager: Frame " << msg->frameCount << " of " 
             << msg->numFrames-1 << std::endl;
  int len = msg->dataSize;
  FDEBUG(10) << "testStorageManager: received data frame size = " << len << std::endl;

  // If running with local transfers, a chain of I2O frames when posted only has the
  // head frame sent. So a single frame can complete a chain for local transfers.
  // We need to test for this. Must be head frame, more than one frame
  // and next pointer must exist.
  int is_local_chain = 0;
  if(msg->frameCount == 0 && msg->numFrames > 1 && ref->getNextReference())
  {
    // this looks like a chain of frames (local transfer)
    toolbox::mem::Reference *head = ref;
    toolbox::mem::Reference *next = 0;
    // best to check the complete chain just in case!
    unsigned int tested_frames = 1;
    next = head;
    while((next=next->getNextReference())!=0) tested_frames++;
    FDEBUG(10) << "testStorageManager: Head frame has " << tested_frames-1
               << " linked frames out of " << msg->numFrames-1 << std::endl;
    if(msg->numFrames == tested_frames)
    {
      // found a complete linked chain from the leading frame
      is_local_chain = 1;
      FDEBUG(10) << "testStorageManager: Leading frame contains a complete linked chain"
                 << " - must be local transfer" << std::endl;
      FDEBUG(10) << "testStorageManager: Breaking the chain" << std::endl;
      // break the chain and feed them to the fragment collector
      next = head;
      for(int iframe=0; iframe <(int)msg->numFrames; iframe++)
      {
         toolbox::mem::Reference *thisref=next;
         next = thisref->getNextReference();
         thisref->setNextReference(0);
         I2O_MESSAGE_FRAME         *thisstdMsg = (I2O_MESSAGE_FRAME*)thisref->getDataLocation();
         I2O_SM_DATA_MESSAGE_FRAME *thismsg    = (I2O_SM_DATA_MESSAGE_FRAME*)thisstdMsg;
         EventBuffer::ProducerBuffer b(jc_->getFragmentQueue());
         int thislen = thismsg->dataSize;
         new (b.buffer()) stor::FragEntry(thisref, thismsg->data, thislen);
         b.commit(sizeof(stor::FragEntry));
         framecounter_++;
         // for bandwidth performance measurements
         unsigned long actualFrameSize = (unsigned long)sizeof(I2O_SM_DATA_MESSAGE_FRAME);
         addMeasurement(actualFrameSize);
         // for FU sender list update
         // msg->frameCount start from 0, but in EventMsg header it starts from 1!
         bool isLocal = true;
         updateFUSender4data(&msg->hltURL[0], &msg->hltClassName[0],
           msg->hltLocalId, msg->hltInstance, msg->hltTid,
           msg->runID, msg->eventID, msg->frameCount+1, msg->numFrames,
           msg->originalSize, isLocal);
      }
    } else {
      // should never get here!
      FDEBUG(10) << "testStorageManager: Head frame has fewer linked frames "
                 << "than expected: abnormal error! " << std::endl;
    }
  }

  if (is_local_chain == 0) 
  {
    // put pointers into fragment collector queue
    EventBuffer::ProducerBuffer b(jc_->getFragmentQueue());
    /* stor::FragEntry* fe = */ new (b.buffer()) stor::FragEntry(ref, msg->data, len);
    b.commit(sizeof(stor::FragEntry));
    // Frame release is done in the deleter.
    framecounter_++;
    // for bandwidth performance measurements
    unsigned long actualFrameSize = (unsigned long)sizeof(I2O_SM_DATA_MESSAGE_FRAME);
    addMeasurement(actualFrameSize);
    // for FU sender list update
    // msg->frameCount start from 0, but in EventMsg header it starts from 1!
    bool isLocal = false;
    updateFUSender4data(&msg->hltURL[0], &msg->hltClassName[0],
      msg->hltLocalId, msg->hltInstance, msg->hltTid,
      msg->runID, msg->eventID, msg->frameCount+1, msg->numFrames,
      msg->originalSize, isLocal);
  }
}

void testStorageManager::receiveOtherMessage(toolbox::mem::Reference *ref)
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
  FDEBUG(10) << "testStorageManager: Received other message from HLT " << msg->hltURL
             << " application " << msg->hltClassName << " id " << msg->hltLocalId
             << " instance " << msg->hltInstance << " tid " << msg->hltTid << std::endl;
  FDEBUG(10) << "testStorageManager: message content " << msg->otherData << "\n";

  // Not yet processing any Other messages type

  // release the frame buffer now that we are finished
  ref->release();

  framecounter_++;

  // for bandwidth performance measurements
  unsigned long actualFrameSize = (unsigned long)sizeof(I2O_SM_OTHER_MESSAGE_FRAME);
  addMeasurement(actualFrameSize);
}
////////////////////////////// Tracking FU Sender Status  //////////////////
void stor::testStorageManager::registerFUSender(const char* hltURL,
  const char* hltClassName, const unsigned long hltLocalId,
  const unsigned long hltInstance, const unsigned long hltTid,
  const unsigned long registrySize, const char* registryData)
{
  // register FU sender into the list to keep its status
  // first check if FU is already in the list

  // register this FU sender
  SMFUSenderList *fusender = new SMFUSenderList(hltURL, hltClassName,
                 hltLocalId, hltInstance, hltTid,
                 registrySize, registryData);
  smfusenders_.push_back(*fusender);
  smfusenders_.back().chrono_.start(0);
  // check the registry from this FU against the configuration one
  bool setcode = false;
  // need to make a copy in a non-const array
  char tempregdata[MAX_I2O_REGISTRY_DATASIZE];
  copy(registryData, registryData+MAX_I2O_REGISTRY_DATASIZE, tempregdata);
  //edm::InitMsg msg(&registryData[0],registrySize,setcode);
  edm::InitMsg msg(&tempregdata[0],registrySize,setcode);
  // use available methods to check registry is a subset
  edm::JobHeaderDecoder decoder;
  std::auto_ptr<edm::SendJobHeader> header = decoder.decodeJobHeader(msg);
  //if(edm::registryIsSubset(*header, jc_->products()))
  if(edm::registryIsSubset(*header, jc_->smproducts()))
  {
    FDEBUG(10) << "registerFUSender: Received registry is okay" << std::endl;
    smfusenders_.back().regCheckedOK_ = true;
  } else {
    std::cout << "registerFUSender: Error! Received registry is not a subset!"
              << std::endl;
  };
}

void stor::testStorageManager::updateFUSender4data(const char* hltURL,
  const char* hltClassName, const unsigned long hltLocalId,
  const unsigned long hltInstance, const unsigned long hltTid,
  const unsigned long runNumber, const unsigned long eventNumber,
  const unsigned long frameNum, const unsigned long totalFrames,
  const unsigned long origdatasize, const bool isLocal)
{
  // Find this FU sender in the list
  bool problemFound = false;
  if(smfusenders_.size() > 0)
  {
    bool fusender_found = false;
    vector<SMFUSenderList>::iterator foundPos;
    for(vector<SMFUSenderList>::iterator pos = smfusenders_.begin();
        pos != smfusenders_.end(); ++pos)
    {
      if(pos->hltLocalId_ == hltLocalId && pos->hltInstance_ == hltInstance &&
         pos->hltTid_ == hltTid)
      {
        fusender_found = true;
        foundPos = pos;
      }
    }
    if(fusender_found)
    {
      // should really check this is not a duplicate frame
      // should test total frames is the same, and other tests are possible
      // check if this is the first data frame received
      if(foundPos->connectStatus_ < 2) {  //should actually check bit 2!
        foundPos->connectStatus_ = foundPos->connectStatus_ + 2; //should set bit 2!
        FDEBUG(10) << "updateFUSender4data: received first data frame" << std::endl;
        foundPos->runNumber_ = runNumber;
        foundPos->isLocal_ = isLocal;
      } else {
         if(foundPos->runNumber_ != runNumber) {
            problemFound = true;
            FDEBUG(10) << "updateFUSender4data: data frame with new run number!"
                       << " Current run " << foundPos->runNumber_
                       << " new run " << runNumber << std::endl;
         }
         // could test isLocal here
      }
      foundPos->framesReceived_++;
      foundPos->lastRunID_ = runNumber;
      foundPos->chrono_.stop(0);
      foundPos->lastLatency_ = (double) foundPos->chrono_.dusecs(); //microseconds
      foundPos->chrono_.start(0);
      // check if this frame is the last (assuming in order)
      // must also handle if there is only one frame in event
      if(totalFrames == 1) {
        // there is only one frame in this event assume frameNum = 1!
        foundPos->eventsReceived_++;
        foundPos->lastEventID_ = eventNumber;
        foundPos->lastFrameNum_ = frameNum;
        foundPos->lastTotalFrameNum_ = totalFrames;
        foundPos->totalSizeReceived_ = foundPos->totalSizeReceived_ + origdatasize;
      } else {
        // flag and count if frame (event fragment) out of order
        if(foundPos->lastEventID_ == eventNumber) {
          // check if in order and if last frame in a chain
          if(frameNum != foundPos->lastFrameNum_ + 1) {
            foundPos->totalOutOfOrder_++;
          }
          if(totalFrames != foundPos->lastTotalFrameNum_) {
            problemFound = true;
            // this is a real problem! Corrupt data frame
          }
          // if last frame in n-of-m assume it completes an event
          // frame count starts from 1
          if(frameNum == totalFrames) { //should check totalFrames
            foundPos->eventsReceived_++;
            foundPos->totalSizeReceived_ = foundPos->totalSizeReceived_ + origdatasize;
          }
          foundPos->lastFrameNum_ = frameNum;
        } else {
          // new event (assume run number does not change)
          foundPos->lastEventID_ = eventNumber;
          if(foundPos->lastFrameNum_ != foundPos->lastTotalFrameNum_ &&
             foundPos->framesReceived_ != 1) {
            // missing or frame out of order (may count multiplely!)
            foundPos->totalOutOfOrder_++;
          }
          foundPos->lastFrameNum_ = frameNum;
          foundPos->lastTotalFrameNum_ = totalFrames;
        }
      } // totalFrames=1 or not
      if(problemFound) foundPos->totalBadEvents_++;
    } // fu sender found
    else
    {
      FDEBUG(10) << "updateFUSender4data: Cannot find FU in FU Sender list!"
                 << " With URL "
                 << hltURL << " class " << hltClassName  << " instance "
                 << hltInstance << " Tid " << hltTid << std::endl;
    }
  } else {
    // problem: did not find an entry!!
    FDEBUG(10) << "updateFUSender4data: No entries at all in FU sender list!"
               << std::endl;
  }
}

////////////////////////////// Performance      ////////////////////////////
void testStorageManager::addMeasurement(unsigned long size)
{
  // for bandwidth performance measurements
  if ( pmeter_->addSample(size) )
  {
    // don't print out info by default (already in web page)
    //LOG4CPLUS_INFO(getApplicationLogger(),
    //  toolbox::toString("measured latency: %f for size %d",pmeter_->latency(), size));
    //LOG4CPLUS_INFO(getApplicationLogger(),
    //  toolbox::toString("latency:  %f, rate: %f,bandwidth %f, size: %d\n",
    //  pmeter_->latency(),pmeter_->rate(),pmeter_->bandwidth(),size));
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

////////////////////////////// Default web page ////////////////////////////
void testStorageManager::defaultWebPage(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception)
{
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

  *out << "<table frame=\"void\" rules=\"groups\" class=\"states\">" << endl;
  *out << "<colgroup> <colgroup align=\"rigth\">"                    << endl;
    *out << "  <tr>"                                                   << endl;
    *out << "    <th colspan=2>"                                       << endl;
    *out << "      " << "Memory Pool Usage"                            << endl;
    *out << "    </th>"                                                << endl;
    *out << "  </tr>"                                                  << endl;

        *out << "<tr>" << endl;
        *out << "<th >" << endl;
        *out << "Parameter" << endl;
        *out << "</th>" << endl;
        *out << "<th>" << endl;
        *out << "Value" << endl;
        *out << "</th>" << endl;
        *out << "</tr>" << endl;
        *out << "<tr>" << endl;
          *out << "<td >" << endl;
          *out << "Frames Received" << endl;
          *out << "</td>" << endl;
          *out << "<td align=right>" << endl;
          *out << framecounter_ << endl;
          *out << "</td>" << endl;
        *out << "  </tr>" << endl;
        if(pool_is_set_ == 1) 
        {
          *out << "<tr>" << endl;
            *out << "<td >" << endl;
            *out << "Memory Used (Bytes)" << endl;
            *out << "</td>" << endl;
            *out << "<td align=right>" << endl;
            *out << pool_->getMemoryUsage().getUsed() << endl;
            *out << "</td>" << endl;
          *out << "  </tr>" << endl;
        } else {
          *out << "<tr>" << endl;
            *out << "<td >" << endl;
            *out << "Memory Pool pointer not yet available" << endl;
            *out << "</td>" << endl;
          *out << "  </tr>" << endl;
        }
// performance statistics
    *out << "  <tr>"                                                   << endl;
    *out << "    <th colspan=2>"                                       << endl;
    *out << "      " << "Performance for last " << samples_ << " frames"<< endl;
    *out << "    </th>"                                                << endl;
    *out << "  </tr>"                                                  << endl;
        *out << "<tr>" << endl;
          *out << "<td >" << endl;
          *out << "Bandwidth (MB/s)" << endl;
          *out << "</td>" << endl;
          *out << "<td align=right>" << endl;
          *out << databw_ << endl;
          *out << "</td>" << endl;
        *out << "  </tr>" << endl;
        *out << "<tr>" << endl;
          *out << "<td >" << endl;
          *out << "Rate (Frames/s)" << endl;
          *out << "</td>" << endl;
          *out << "<td align=right>" << endl;
          *out << datarate_ << endl;
          *out << "</td>" << endl;
        *out << "  </tr>" << endl;
        *out << "<tr>" << endl;
          *out << "<td >" << endl;
          *out << "Latency (us/frame)" << endl;
          *out << "</td>" << endl;
          *out << "<td align=right>" << endl;
          *out << datalatency_ << endl;
          *out << "</td>" << endl;
        *out << "  </tr>" << endl;
        *out << "<tr>" << endl;
          *out << "<td >" << endl;
          *out << "Maximum Bandwidth (MB/s)" << endl;
          *out << "</td>" << endl;
          *out << "<td align=right>" << endl;
          *out << maxdatabw_ << endl;
          *out << "</td>" << endl;
        *out << "  </tr>" << endl;
        *out << "<tr>" << endl;
          *out << "<td >" << endl;
          *out << "Minimum Bandwidth (MB/s)" << endl;
          *out << "</td>" << endl;
          *out << "<td align=right>" << endl;
          *out << mindatabw_ << endl;
          *out << "</td>" << endl;
        *out << "  </tr>" << endl;
// mean performance statistics for whole run
    *out << "  <tr>"                                                   << endl;
    *out << "    <th colspan=2>"                                       << endl;
    *out << "      " << "Mean Performance for " << totalsamples_ << " frames, duration "
         << duration_ << " seconds" << endl;
    *out << "    </th>"                                                << endl;
    *out << "  </tr>"                                                  << endl;
        *out << "<tr>" << endl;
          *out << "<td >" << endl;
          *out << "Bandwidth (MB/s)" << endl;
          *out << "</td>" << endl;
          *out << "<td align=right>" << endl;
          *out << meandatabw_ << endl;
          *out << "</td>" << endl;
        *out << "  </tr>" << endl;
        *out << "<tr>" << endl;
          *out << "<td >" << endl;
          *out << "Rate (Frames/s)" << endl;
          *out << "</td>" << endl;
          *out << "<td align=right>" << endl;
          *out << meandatarate_ << endl;
          *out << "</td>" << endl;
        *out << "  </tr>" << endl;
        *out << "<tr>" << endl;
          *out << "<td >" << endl;
          *out << "Latency (us/frame)" << endl;
          *out << "</td>" << endl;
          *out << "<td align=right>" << endl;
          *out << meandatalatency_ << endl;
          *out << "</td>" << endl;
        *out << "  </tr>" << endl;

  *out << "</table>" << endl;

  *out << "  </td>"                                                  << endl;
  *out << "</table>"                                                 << endl;
// now for FU sender list statistics
  *out << "<hr/>"                                                    << endl;
  *out << "<table>"                                                  << endl;
  *out << "<tr valign=\"top\">"                                      << endl;
  *out << "  <td>"                                                   << endl;

  *out << "<table frame=\"void\" rules=\"groups\" class=\"states\">" << endl;
  *out << "<colgroup> <colgroup align=\"rigth\">"                    << endl;
    *out << "  <tr>"                                                   << endl;
    *out << "    <th colspan=2>"                                       << endl;
    *out << "      " << "FU Sender List"                            << endl;
    *out << "    </th>"                                                << endl;
    *out << "  </tr>"                                                  << endl;

    *out << "<tr>" << endl;
    *out << "<th >" << endl;
    *out << "Parameter" << endl;
    *out << "</th>" << endl;
    *out << "<th>" << endl;
    *out << "Value" << endl;
    *out << "</th>" << endl;
    *out << "</tr>" << endl;
        *out << "<tr>" << endl;
          *out << "<td >" << endl;
          *out << "Number of FU Senders" << endl;
          *out << "</td>" << endl;
          *out << "<td>" << endl;
          *out << smfusenders_.size() << endl;
          *out << "</td>" << endl;
        *out << "  </tr>" << endl;
    if(smfusenders_.size() > 0) {
      for(vector<SMFUSenderList>::iterator pos = smfusenders_.begin();
          pos != smfusenders_.end(); ++pos)
      {
        *out << "<tr>" << endl;
          *out << "<td >" << endl;
          *out << "FU Sender URL" << endl;
          *out << "</td>" << endl;
          *out << "<td align=right>" << endl;
          *out << pos->hltURL_ << endl;
          *out << "</td>" << endl;
        *out << "  </tr>" << endl;
        *out << "<tr>" << endl;
          *out << "<td >" << endl;
          *out << "FU Sender Class Name" << endl;
          *out << "</td>" << endl;
          *out << "<td>" << endl;
          *out << pos->hltClassName_ << endl;
          *out << "</td>" << endl;
        *out << "  </tr>" << endl;
        *out << "<tr>" << endl;
          *out << "<td >" << endl;
          *out << "FU Sender Instance" << endl;
          *out << "</td>" << endl;
          *out << "<td>" << endl;
          *out << pos->hltInstance_ << endl;
          *out << "</td>" << endl;
        *out << "  </tr>" << endl;
        *out << "<tr>" << endl;
          *out << "<td >" << endl;
          *out << "FU Sender Local ID" << endl;
          *out << "</td>" << endl;
          *out << "<td>" << endl;
          *out << pos->hltLocalId_ << endl;
          *out << "</td>" << endl;
        *out << "  </tr>" << endl;
        *out << "<tr>" << endl;
          *out << "<td >" << endl;
          *out << "FU Sender Tid" << endl;
          *out << "</td>" << endl;
          *out << "<td>" << endl;
          *out << pos->hltTid_ << endl;
          *out << "</td>" << endl;
        *out << "  </tr>" << endl;
        *out << "<tr>" << endl;
          *out << "<td >" << endl;
          *out << "Product registry" << endl;
          *out << "</td>" << endl;
          *out << "<td>" << endl;
          if(pos->regCheckedOK_) {
            *out << "OK" << endl;
          } else {
            *out << "Bad" << endl;
          }
          *out << "</td>" << endl;
        *out << "  </tr>" << endl;
        *out << "<tr>" << endl;
          *out << "<td >" << endl;
          *out << "Connection Status" << endl;
          *out << "</td>" << endl;
          *out << "<td>" << endl;
          *out << pos->connectStatus_ << endl;
          *out << "</td>" << endl;
        *out << "  </tr>" << endl;
        if(pos->connectStatus_ > 1) {
          *out << "<tr>" << endl;
            *out << "<td >" << endl;
            *out << "Time since last data frame (us)" << endl;
            *out << "</td>" << endl;
            *out << "<td align=right>" << endl;
            // looking at the code for Chrono this does not actually
            // stop the "stopwatch" it just calculates the time since
            // the last start was called so can use this
            pos->chrono_.stop(0);
            double timewaited = (double) pos->chrono_.dusecs(); //microseconds
            *out << timewaited << endl;
            *out << "</td>" << endl;
          *out << "  </tr>" << endl;
          *out << "<tr>" << endl;
            *out << "<td >" << endl;
            *out << "Run number" << endl;
            *out << "</td>" << endl;
            *out << "<td align=right>" << endl;
            *out << pos->runNumber_ << endl;
            *out << "</td>" << endl;
          *out << "  </tr>" << endl;
          *out << "<tr>" << endl;
            *out << "<td >" << endl;
            *out << "Running locally" << endl;
            *out << "</td>" << endl;
            *out << "<td align=right>" << endl;
            if(pos->isLocal_) {
              *out << "Yes" << endl;
            } else {
              *out << "No" << endl;
            }
            *out << "</td>" << endl;
          *out << "  </tr>" << endl;
          *out << "<tr>" << endl;
            *out << "<td >" << endl;
            *out << "Frames received" << endl;
            *out << "</td>" << endl;
            *out << "<td align=right>" << endl;
            *out << pos->framesReceived_ << endl;
            *out << "</td>" << endl;
          *out << "  </tr>" << endl;
          *out << "<tr>" << endl;
            *out << "<td >" << endl;
            *out << "Events received" << endl;
            *out << "</td>" << endl;
            *out << "<td align=right>" << endl;
            *out << pos->eventsReceived_ << endl;
            *out << "</td>" << endl;
          *out << "  </tr>" << endl;
          if(pos->eventsReceived_ > 0) {
            *out << "<tr>" << endl;
              *out << "<td >" << endl;
              *out << "Last frame latency (us)" << endl;
              *out << "</td>" << endl;
              *out << "<td align=right>" << endl;
              *out << pos->lastLatency_ << endl;
              *out << "</td>" << endl;
            *out << "  </tr>" << endl;
            *out << "<tr>" << endl;
              *out << "<td >" << endl;
              *out << "Average event size (Bytes)" << endl;
              *out << "</td>" << endl;
              *out << "<td align=right>" << endl;
              *out << pos->totalSizeReceived_/pos->eventsReceived_ << endl;
              *out << "</td>" << endl;
              *out << "<tr>" << endl;
                *out << "<td >" << endl;
                *out << "Last Run Number" << endl;
                *out << "</td>" << endl;
                *out << "<td align=right>" << endl;
                *out << pos->lastRunID_ << endl;
                *out << "</td>" << endl;
              *out << "  </tr>" << endl;
              *out << "<tr>" << endl;
                *out << "<td >" << endl;
                *out << "Last Event Number" << endl;
                *out << "</td>" << endl;
                *out << "<td align=right>" << endl;
                *out << pos->lastEventID_ << endl;
                *out << "</td>" << endl;
              *out << "  </tr>" << endl;
            } // events received endif
          *out << "  </tr>" << endl;
          *out << "<tr>" << endl;
            *out << "<td >" << endl;
            *out << "Total out of order frames" << endl;
            *out << "</td>" << endl;
            *out << "<td align=right>" << endl;
            *out << pos->totalOutOfOrder_ << endl;
            *out << "</td>" << endl;
          *out << "  </tr>" << endl;
          *out << "<tr>" << endl;
            *out << "<td >" << endl;
            *out << "Total Bad Events" << endl;
            *out << "</td>" << endl;
            *out << "<td align=right>" << endl;
            *out << pos->totalBadEvents_ << endl;
            *out << "</td>" << endl;
          *out << "  </tr>" << endl;
        } // connect status endif
      } // Sender list loop
    } //sender size test endif

  *out << "</table>" << endl;

  *out << "  </td>"                                                  << endl;
  *out << "</table>"                                                 << endl;
  //---- separate page test
  *out << "<hr/>"                                                 << endl;
  std::string url = getApplicationDescriptor()->getContextDescriptor()->getURL();
  std::string urn = getApplicationDescriptor()->getURN();
  *out << "<a href=\"" << url << "/" << urn << "/fusenderlist" << "\">" 
       << "FU Sender list web page" << "</a>" << endl;
  *out << "<hr/>"                                                 << endl;
  *out << "<a href=\"" << url << "/" << urn << "/geteventdata" << "\">" 
       << "Get an event via a web page" << "</a>" << endl;
  *out << "<hr/>"                                                 << endl;
  *out << "<a href=\"" << url << "/" << urn << "/getregdata" << "\">" 
       << "Get a header via a web page" << "</a>" << endl;

  *out << "</body>"                                                  << endl;
  *out << "</html>"                                                  << endl;
}
////////////////////////////// fusender web page ////////////////////////////
void testStorageManager::fusenderWebPage(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception)
{
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

// now for FU sender list statistics
  *out << "fu sender table"                                                  << endl;

  *out << "</body>"                                                  << endl;
  *out << "</html>"                                                  << endl;
}
////////////////////////////// get event data web page ////////////////////////////
void testStorageManager::eventdataWebPage(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception)
{
  std::ifstream infile("samplestreamer.dat",ios_base::binary | ios_base::in);
/* this sends an entire event
  char buffer[1000000];
  int i = 0;
  while(!infile.eof())
  {
    buffer[i]=(char)infile.get();
    i++;
  }
  cout << "file size = " << i-1 <<endl;
  out->write(buffer,i-1);
*/

  // this is horrible but lacking time....
  char buffer[7000000];
  int reglen = 0;
  // just get rid of registry
  infile.read((char*)&reglen,sizeof(int));
  infile.read(&buffer[0],reglen);
  // now the event data
  int len = 0;
  infile.read((char*)&len,sizeof(int));
  for (int i=0; i<len ; i++) buffer[i]=(char)infile.get();

  out->getHTTPResponseHeader().addHeader("Content-Type", "application/octet-stream");
  out->getHTTPResponseHeader().addHeader("Content-Transfer-Encoding", "binary");
  out->write(buffer,len);

// How to block if there is no data
// How to signal if end, and there will be no more data?

}
////////////////////////////// get header (registry) web page ////////////////////////////
void testStorageManager::headerdataWebPage(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception)
{
  // this is horrible but lacking time....
  vector<char> regdata(1000*1000);

  std::string filename="samplestreamer.dat";
  std::ifstream ist(filename.c_str(),ios_base::binary | ios_base::in);

  int len;
  ist.read((char*)&len,sizeof(int));
  regdata.resize(len);
  ist.read(&regdata[0],len);

  if(!ist)
  throw cms::Exception("myReadHeader","headerdataWebPage")
        << "Could not read the registry information from the test\n"
        << "event stream file \n";

  char buffer[1000000];
  for (int i=0; i<len; i++) buffer[i]=regdata[i];

  out->getHTTPResponseHeader().addHeader("Content-Type", "application/octet-stream");
  out->getHTTPResponseHeader().addHeader("Content-Transfer-Encoding", "binary");
  out->write(buffer,len);

// How to block if there is no header data
// How to signal if not yet started, so there is no registry yet?
}

} //stor namespace


/**
 * Provides factory method for the instantiation of SM applications
 */

extern "C" xdaq::Application * instantiate_testStorageManager(xdaq::ApplicationStub * stub )
{
        std::cout << "Going to construct a testStorageManager instance " << std::endl;
        return new stor::testStorageManager(stub);
}

