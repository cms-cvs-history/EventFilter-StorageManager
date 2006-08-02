/*
   Author: Harry Cheung, FNAL

   Description:
     XDAQ application to test the I2O output module. It can receive
     I2O frames and write out a streamer data file. This streamer
     data file can be converted back to a standard root file by
     the streamer file reader. This version can used as a barebones
     Storage Manager, but cannot handle output options like
     multistreams and trigger selection.
     See CMS EventFilter wiki page for further notes.

   Modification:
     version 1.1 2005/07/29
       Initial implementation.

*/

#include <exception>
#include <sstream>
#include <iostream>
#include <string>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/unistd.h>
#include <iostream>
#include <iomanip>

#include "EventFilter/StorageManager/test/testI2OReceiver.h"
//move this next line
#include "IOPool/Streamer/interface/EOFRecordBuilder.h"
#include "IOPool/Streamer/interface/OtherMessage.h"
#include "IOPool/Streamer/interface/InitMessage.h"

//#include "FWCore/Framework/interface/EventProcessor.h"
//#include "FWCore/Utilities/interface/ProblemTracker.h" 
#include "FWCore/Utilities/interface/DebugMacros.h"
//#include "FWCore/Utilities/interface/Exception.h"
//#include "IOPool/Streamer/interface/Messages.h"

#include "EventFilter/StorageManager/interface/i2oStorageManagerMsg.h"
#include "EventFilter/StorageManager/interface/SMFUSenderList.h"

#include "xcept/tools.h"

#include "i2o/Method.h"
#include "i2o/utils/include/i2o/utils/AddressMap.h"

#include "toolbox/mem/Pool.h"
#include "xcept/tools.h"
#include "xgi/Method.h"

#include "xoap/include/xoap/SOAPEnvelope.h"
#include "xoap/include/xoap/SOAPBody.h"
#include "xoap/include/xoap/domutils.h"

using namespace edm;
using namespace std;

// -----------------------------------------------

namespace stor {

/* ------ SMI2ORecFrames ---------------------------*/
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

}

using namespace stor;

/* -------------------------------------------------*/

testI2OReceiver::testI2OReceiver(xdaq::ApplicationStub * s)
  throw (xdaq::exception::Exception): xdaq::Application(s),
  fsm_(0), evtsrv_area_(10), count_4_oneinN_(0), connectedFUs_(0), storedEvents_(0)
{
    LOG4CPLUS_INFO(this->getApplicationLogger(),"Making testI2OReceiver");

  fsm_ = new stor::SMStateMachine(getApplicationLogger());
  fsm_->init<testI2OReceiver>(this);
  xdata::InfoSpace *ispace = getApplicationInfoSpace();
  // default configuration
  ispace->fireItemAvailable("STparameterSet",&offConfig_);
  ispace->fireItemAvailable("runNumber",&runNumber_);
  ispace->fireItemAvailable("stateName",&fsm_->stateName_);

  ispace->fireItemAvailable("connectedFUs",&connectedFUs_);
  ispace->fireItemAvailable("storedEvents",&storedEvents_);

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

  // Bind web interface
  xgi::bind(this,&testI2OReceiver::defaultWebPage, "Default");
  xgi::bind(this,&testI2OReceiver::css, "styles.css");
  //xgi::bind(this,&testI2OReceiver::fusenderWebPage, "fusenderlist");
  xgi::bind(this,&testI2OReceiver::eventdataWebPage, "geteventdata");
  xgi::bind(this,&testI2OReceiver::headerdataWebPage, "getregdata");

  ispace->fireItemAvailable("streamerFilename",&stfileName_);
  filename_ = "smi2ostreamout.dat";  // default - set in configureAction
  // open the file in configureAction not here so we can reconfigure
  // and filename is in the XML config file

  eventcounter_ = 0;
  framecounter_ = 0;
  pool_is_set_ = 0;
  pool_ = 0;

  // added for Event Server
  ser_prods_size_ = 0;
  serialized_prods_[0] = '\0';
  oneinN_ = 10;
  ispace->fireItemAvailable("oneinN",&oneinN_);
  FDEBUG(9) << "Streamer event server one-in-N = " << oneinN_ << endl;

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

  // what is this stuff?
  string xmlClass_ = getApplicationDescriptor()->getClassName();
  unsigned long instance_ = getApplicationDescriptor()->getInstance();
  ostringstream sourcename;
  sourcename << xmlClass_ << "_" << instance_;
  sourceId_ = sourcename.str();

}

testI2OReceiver::~testI2OReceiver()
{
  delete fsm_;
  delete pmeter_;
}

xoap::MessageReference testI2OReceiver::ParameterGet(xoap::MessageReference message) 
  throw (xoap::exception::Exception)
{
  connectedFUs_.value_ = smfusenders_.size();
  return Application::ParameterGet(message);
}
//#include "FWCore/ServiceRegistry/interface/ServiceToken.h"
//#include "FWCore/ServiceRegistry/interface/Service.h"
//#include "EventFilter/Utilities/interface/ModuleWebRegistry.h"

//#include "EventFilter/Utilities/interface/ModuleWebRegistry.h"
//#include "EventFilter/Utilities/interface/ParameterSetRetriever.h"

void testI2OReceiver::configureAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception)
{
  // Get into the ready state
  filename_ = stfileName_.toString();
  FDEBUG(9) << "Streamer filename starts with = " << filename_ << endl;
  FDEBUG(9) << "Streamer filename run number = " << runNumber_ << endl;
  std::ostringstream stm;
  stm << setfill('0') << std::setw(8) << runNumber_;
  filename_ = filename_ + "." + stm.str() + ".dat";
  //filename_ += ".";
  //filename_ += stm.str();
  //filename_ += ".dat";
  //FDEBUG(9) << "Streamer output filename = " << filename_ << endl;
  // open (new) file on configure or reconfigure
  FDEBUG(9) << "testI2OReceiver: openning output streamer file " << filename_ << "\n";
  ost_.open(filename_.c_str(),ios_base::binary | ios_base::out);
  if(!ost_)
  {
    // how do I throw exceptions in the online?
    //throw cms::Exception("Configuration","testI2OReceiver")
    //  << "cannot open file " << filename_;
    std::cerr << "testI2OReceiver: Cannot open file " << filename_ << std::endl;
  }
  FDEBUG(9) << "Streamer event server one-in-N = " << oneinN_ << endl;

  // what is this?
  //evf::ParameterSetRetriever smpset(offConfig_.value_);
  //string my_config = smpset.getAsString();

  // added for Event Server
  int value_4oneinN(oneinN_);
  if(value_4oneinN <= 0) value_4oneinN = -1;
//  jc_->set_oneinN(value_4oneinN);

  // what is this stuff?
/*
  evf::ModuleWebRegistry *mwr = 0;
  edm::ServiceRegistry::Operate operate(jc_->getToken());
  try{
    if(edm::Service<evf::ModuleWebRegistry>().isAvailable())
      mwr = edm::Service<evf::ModuleWebRegistry>().operator->();
  }
  catch(...)
    { cout <<"exception when trying to get the service registry " << endl;}
  if(mwr)
    mwr->publish(getApplicationInfoSpace());
*/
}

void testI2OReceiver::enableAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception)
{
  // Get into running state
}

void testI2OReceiver::haltAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception)
{
  // Check that all senders have closed connections
  // does this work? Not sure it waits in a separate thread of not?
  int timeout = 60;
  std::cout << "waiting for " <<  smfusenders_.size()
            << " senders to end the run " << std::endl;
  while(smfusenders_.size() > 0)
    {
      ::sleep(1);
      if(timeout-- == 0) {
        LOG4CPLUS_WARN(this->getApplicationLogger(),
                       "Timeout in SM waiting for end of run from "
                       << smfusenders_.size() << "senders ");
        break;
      }
    }

  // Get into halted state
  std::cout << "testI2OReceiver: Received halt message. Closing file " << filename_ << std::endl;
   // need an EOF record?
     uint32 dummyStatusCode = 1234;
    std::vector<uint32> hltStats;

    hltStats.push_back(32);
    hltStats.push_back(33);
    hltStats.push_back(34);
    uint64 dummy_first_event_offset_ = 0;
    uint64 dummy_last_event_offset_ = 10000;
     //HEREHERE need to change this
    EOFRecordBuilder eof(runNumber_,
                         storedEvents_,
                         dummyStatusCode,
                         hltStats,
                         dummy_first_event_offset_,
                         dummy_last_event_offset_);
  ost_.write((const char*)eof.recAddress(), eof.size() );

  ost_.close();

  // make sure serialized product registry is cleared also
  ser_prods_size_ = 0;
}

void testI2OReceiver::nullAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception)
{
  //this action has no effect. A warning is issued to this end
  LOG4CPLUS_WARN(this->getApplicationLogger(),
                    "Null action invoked");
}

xoap::MessageReference testI2OReceiver::fireEvent(xoap::MessageReference msg)
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
  FDEBUG(9) << "testI2OReceiver: Received registry message from HLT " << msg->hltURL
             << " application " << msg->hltClassName << " id " << msg->hltLocalId
             << " instance " << msg->hltInstance << " tid " << msg->hltTid << std::endl;
  FDEBUG(9) << "testI2OReceiver: registry size " << msg->dataSize << "\n";
  // can get rid of this if not dumping the data for checking
  std::string temp4print(msg->data,msg->dataSize);
  FDEBUG(9) << "testI2OReceiver: registry data = " << temp4print << std::endl;

  framecounter_++;

  // for bandwidth performance measurements
  unsigned long actualFrameSize = (unsigned long)sizeof(I2O_SM_PREAMBLE_MESSAGE_FRAME)
                                  + msg->dataSize;
  addMeasurement(actualFrameSize);
  //  std::cout << "received registry message from " << msg->hltURL << std::endl;
  // register this FU sender into the list to keep its status
  registerFUSender(&msg->hltURL[0], &msg->hltClassName[0],
                 msg->hltLocalId, msg->hltInstance, msg->hltTid,
                 msg->frameCount, msg->numFrames,
                 msg->originalSize, msg->dataPtr(), ref);
  // should not release until after registerFUSender finds all fragments
  //ref->release();

  /*
  // write out the registry
  int sz = msg->dataSize;
  ost_.write((const char*)(&sz),sizeof(int));
  ost_.write((const char*)msg->data,sz);
  // release the frame buffer now that we are finished
  ref->release();
  */ 

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
  // std::cout << "received data message from " << msg->hltURL << std::endl;
  int len = msg->dataSize;
  FDEBUG(10) << "testI2OReceiver: received data frame size = " << len << std::endl;

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

  FDEBUG(10) << "testI2OReceiver: SMframeFragments_ size is " 
             << SMframeFragments_.size() << std::endl;
  // don't release buffers until all frames from a chain are received
  // this is done in testCompleteChain

  framecounter_++;

  // for bandwidth performance measurements
  // does not work for chains transferred locally!
  unsigned long actualFrameSize = (unsigned long)sizeof(I2O_SM_DATA_MESSAGE_FRAME)
                                  + len;
  addMeasurement(actualFrameSize);
  // for FU sender list update
  // not actually test for isLocal here! Done in testCompleteChain
  bool isLocal = false;
  // msg->frameCount start from 0, but in EventMsg header it starts from 1!
  updateFUSender4data(&msg->hltURL[0], &msg->hltClassName[0],
      msg->hltLocalId, msg->hltInstance, msg->hltTid,
      msg->runID, msg->eventID, msg->frameCount+1, msg->numFrames,
      msg->originalSize, isLocal);
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
  // Don't close the file here, this just means one FU is finishing so just
  // delete it

  removeFUSender(&msg->hltURL[0], &msg->hltClassName[0],
                 msg->hltLocalId, msg->hltInstance, msg->hltTid);

  // should really be checking if more event data from incomplete events are
  // still coming or new events that somehow came after the other message packet!
  //std::cout << "testI2OReceiver: Received halt message. Closing file " << filename_ << std::endl;
  //ost_.close();
  // release the frame buffer now that we are finished
  ref->release();

  framecounter_++;

  // for bandwidth performance measurements
  unsigned long actualFrameSize = (unsigned long)sizeof(I2O_SM_OTHER_MESSAGE_FRAME);
  addMeasurement(actualFrameSize);
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

  int value_4oneinN(oneinN_);
  if(value_4oneinN <= 0) value_4oneinN = -1;

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
    //-char tempbuffer[MAX_I2O_SM_DATAFRAMES * MAX_I2O_SM_DATASIZE];
    //-int maxsize = MAX_I2O_SM_DATAFRAMES * MAX_I2O_SM_DATASIZE;
    //cout << ">>>> size of mybuffer_ is " << sizeof(mybuffer_) << endl;
    //int maxsize = 7000000; // should set this to sizeof(mybuffer_)
    int maxsize = sizeof(mybuffer_);
    if(origsize > maxsize)
      std::cerr << "testI2OReceiver: error event data size = " << origsize << " larger than "
                << " max size = " << maxsize << std::endl;
    FDEBUG(10) << "testI2OReceiver: getting data for frame 0" << std::endl;
    FDEBUG(10) << "testI2OReceiver: datasize = " << msg->dataSize << std::endl;
    int sz = msg->dataSize;
    for(int j=0; j < sz; j++)
      mybuffer_[j] = msg->dataPtr()[j];
      //-tempbuffer[j] = msg->data[j];
    // remake the Header for the leading frame/fragment
    //-EventMsg msgBuffer(tempbuffer,sz);
    //-edm::RunNumber_t runid     = msgBuffer.getRunNumber();
    //-edm::EventNumber_t eventid = msgBuffer.getEventNumber();
    //-char dummyBuffer[MAX_I2O_SM_DATAFRAMES * MAX_I2O_SM_DATASIZE];
    //-EventMsg msgFrag(dummyBuffer, origsize, eventid, runid, 1, 1);
    //-unsigned int headerNeededSize = sizeof(MsgCode::Codes)+sizeof(EventMsg::Header);
    //-for(int j=0; j < (int)headerNeededSize; j++) 
    //-  tempbuffer[j] = dummyBuffer[j];

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
      for(int j=0; j < sz; j++)
        mybuffer_[next_index+j] = msgcurr->dataPtr()[j];
      //-for(int j=0; j < sz-(int)headerNeededSize; j++)
        //-tempbuffer[next_index+j] = msgcurr->data[headerNeededSize+j];
      //next_index = next_index + sz;
      //-next_index = next_index + sz - headerNeededSize;
      next_index = next_index + sz;
      prev = curr;
    }
    //- tempbuffer is filled with whole chain data
    // mybuffer_ is filled with whole chain data
    FDEBUG(10) << "testI2OReceiver: Belated checking data size = " << next_index 
               << " original size = " << msg->originalSize << std::endl;
    if(next_index == (int)origsize)
    {
      //ost_.write((const char*)(&origsize),sizeof(int));
      ost_.write((const char*)mybuffer_,origsize);

      count_4_oneinN_++;
      if(count_4_oneinN_ == value_4oneinN)
        {
	  EventMsgView eventView(mybuffer_,hltBitCount,l1BitCount);
          evtsrv_area_.push_back(eventView);
          count_4_oneinN_ = 0;
        }
      //-ost_.write((const char*)tempbuffer,origsize);
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
      for(int j=0; j < sz; j++)
        mybuffer_[j] = msg->dataPtr()[j];
      //ost_.write((const char*)(&sz),sizeof(int));
      ost_.write((const char*)mybuffer_,sz);

      count_4_oneinN_++;
      if(count_4_oneinN_ == value_4oneinN)
        {
	  EventMsgView eventView(mybuffer_,hltBitCount,l1BitCount);
          evtsrv_area_.push_back(eventView);
          count_4_oneinN_ = 0;
        }
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


////////////////////////////// Tracking FU Sender Status  //////////////////
void stor::testI2OReceiver::registerFUSender(const char* hltURL,
  const char* hltClassName, const unsigned long hltLocalId,
  const unsigned long hltInstance, const unsigned long hltTid,
  const unsigned long frameCount, const unsigned long numFrames,
  const unsigned long registrySize, const char* registryData,
  toolbox::mem::Reference *ref)
{  
  // register FU sender into the list to keep its status
  // first check if this FU is already in the list
  if(!smfusenders_.empty())
  {
    // see if this FUsender already has some registry fragments
    // should also test if its complete!! (Would mean a reconnect)
    FDEBUG(9) << "registerFUSender: checking if this FU Sender already registered"
               << std::endl;
    int FUFound = 0;
    list<SMFUSenderList>::iterator foundPos;
    for(list<SMFUSenderList>::iterator pos = smfusenders_.begin(); 
        pos != smfusenders_.end(); ++pos)
    {
       FDEBUG(9) << "registerFUSender: testing if same FU sender" << std::endl;
      if(pos->hltLocalId_ == hltLocalId && pos->hltInstance_ == hltInstance &&
         pos->hltTid_ == hltTid && pos->sameURL(hltURL) &&
         pos->sameClassName(hltClassName))
      { // should check there are no entries with duplicate HLT ids
        FUFound = 1;
        foundPos = pos;
      }
    }
    if(FUFound == 0)
    {
      FDEBUG(9) << "registerFUSender: found a different FU Sender with frame " 
                << frameCount << " from URL "
                << hltURL << " and Tid " << hltTid << std::endl;
      // register this FU sender
      SMFUSenderList *fusender = new SMFUSenderList(hltURL, hltClassName,
                     hltLocalId, hltInstance, hltTid, numFrames,
                     registrySize, registryData);
      smfusenders_.push_back(*fusender);
      LOG4CPLUS_INFO(this->getApplicationLogger(),"register FU sender at " 
         << hltURL << " list size is " << smfusenders_.size());
      smfusenders_.back().chrono_.start(0);
      smfusenders_.back().totFrames_ = numFrames;
      smfusenders_.back().currFrames_ = 1; // should use actual frame if out of order
                                           // but currFrames_ is also the count!
      //smfusenders_.back().currFrames_ = frameCount;
      smfusenders_.back().frameRefs_[frameCount] = ref;
      // now should check if frame is complete and deal with it
      list<SMFUSenderList>::iterator testPos = --smfusenders_.end();
      testCompleteFUReg(testPos);
    } else {
      FDEBUG(10) << "registerFUSender: found another frame " << frameCount << " from URL "
                 << hltURL << " and Tid " << hltTid << std::endl;
      // should really check this is not a duplicate frame
      // should check if already all frames were received (indicates reconnect maybe)
      // should test total frames is the same, and other tests are possible
      foundPos->currFrames_++;
      foundPos->frameRefs_[frameCount] = ref;
      // now should check if frame is complete and deal with it
      testCompleteFUReg(foundPos);
    }
  } else { // no FU sender has ever been registered yet
    // register this FU sender
    SMFUSenderList *fusender = new SMFUSenderList(hltURL, hltClassName,
                   hltLocalId, hltInstance, hltTid, numFrames,
                   registrySize, registryData);
    smfusenders_.push_back(*fusender);
    LOG4CPLUS_INFO(this->getApplicationLogger(),"register FU sender at " 
      << hltURL << " list size is " << smfusenders_.size());

    smfusenders_.back().chrono_.start(0);
    smfusenders_.back().totFrames_ = numFrames;
    smfusenders_.back().currFrames_ = 1;
    smfusenders_.back().frameRefs_[frameCount] = ref;
    // now should check if frame is complete and deal with it
    list<SMFUSenderList>::iterator testPos = --smfusenders_.end();
    testCompleteFUReg(testPos);
  }
}

void stor::testI2OReceiver::testCompleteFUReg(list<SMFUSenderList>::iterator pos)
{
  //  Check that a given FU Sender has sent all frames for a registry
  //  If so store the serialized registry and check it
  //  Does not handle yet when a second registry is sent 
  //  from the same FUSender (e.g. reconnects)
  //
  if(pos->totFrames_ == 1)
  {
    // chain is complete as there is only one frame
    toolbox::mem::Reference *head = 0;
    head = pos->frameRefs_[0];
    FDEBUG(10) << "testCompleteFUReg: No chain as only one frame" << std::endl;
    // copy the whole registry for each FU sender and
    // test the registry against the one being used in Storage Manager
    pos->regAllReceived_ = true;
    copyAndTestRegistry(pos, head);
    // free the complete chain buffer by freeing the head
    head->release();
  }
  else
  {
    if(pos->currFrames_ == pos->totFrames_)
    {
      FDEBUG(10) << "testCompleteFUReg: Received fragment completes a chain that has " 
                 << pos->totFrames_
                 << " frames " << std::endl;
      pos->regAllReceived_ = true;
      toolbox::mem::Reference *head = 0;
      toolbox::mem::Reference *tail = 0;
      if(pos->totFrames_ > 1)
      {
        FDEBUG(10) << "testCompleteFUReg: Remaking the chain" << std::endl;
        for(int i=0; i < (int)(pos->totFrames_)-1 ; i++)
        {
          FDEBUG(10) << "testCompleteFUReg: setting next reference for frame " << i << std::endl;
          head = pos->frameRefs_[i];
          tail = pos->frameRefs_[i+1];
          head->setNextReference(tail);
        }
      }
      head = pos->frameRefs_[0];
      FDEBUG(10) << "testCompleteFUReg: Original chain remade" << std::endl;
      // Deal with the chain
      copyAndTestRegistry(pos, head);
      // free the complete chain buffer by freeing the head
      head->release();
    } else {
    // If running with local transfers, a chain of I2O frames when posted only has the
    // head frame sent. So a single frame can complete a chain for local transfers.
    // We need to test for this. Must be head frame and next pointer must exist.
      if(pos->currFrames_ == 1) // first is always the head??
      {
        toolbox::mem::Reference *head = 0;
        toolbox::mem::Reference *next = 0;
        // can crash here if first received frame is not first frame!
        head = pos->frameRefs_[0];
        // best to check the complete chain just in case!
        unsigned int tested_frames = 1;
        next = head;
        while((next=next->getNextReference())!=0) tested_frames++;
        FDEBUG(10) << "testCompleteFUReg: Head frame has " << tested_frames-1
                   << " linked frames out of " << pos->totFrames_-1 << std::endl;
        if(pos->totFrames_ == tested_frames)
        {
          // found a complete linked chain from the leading frame
          FDEBUG(10) << "testI2OReceiver: Leading frame contains a complete linked chain"
                     << " - must be local transfer" << std::endl;
          pos->regAllReceived_ = true;
          // Deal with the chain
          copyAndTestRegistry(pos, head);
          // free the complete chain buffer by freeing the head
          head->release();
        }
      }
    }
  }
}

void stor::testI2OReceiver::copyAndTestRegistry(list<SMFUSenderList>::iterator pos,
  toolbox::mem::Reference *head)
{
  // Copy the registry fragments into the one place for saving
  //
  FDEBUG(9) << "copyAndTestRegistry: Saving and checking the registry" << std::endl;
  I2O_MESSAGE_FRAME         *stdMsg =
    (I2O_MESSAGE_FRAME*)head->getDataLocation();
  I2O_SM_PREAMBLE_MESSAGE_FRAME *msg    =
    (I2O_SM_PREAMBLE_MESSAGE_FRAME*)stdMsg;
  // get total size and check with original size
  int origsize = msg->originalSize;
  int totalsize2check = 0;
  // should check the size is correct before defining and filling array!!
  char* tempbuffer = new char[origsize];

  if(msg->numFrames > 1)
  {
    FDEBUG(9) << "copyAndTestRegistry: populating registry buffer from chain for "
               << msg->hltURL << " and Tid " << msg->hltTid << std::endl;
    FDEBUG(9) << "copyAndTestRegistry: getting data for frame 0" << std::endl;
    FDEBUG(9) << "copyAndTestRegistry: datasize = " << msg->dataSize << std::endl;
    int sz = msg->dataSize;
    totalsize2check = totalsize2check + sz;
    if(totalsize2check > origsize) {
      std::cerr << "copyAndTestRegistry: total registry fragment size " << sz
             << " is larger than original size " << origsize 
             << " abort copy and test" << std::endl;
      pos->regCheckedOK_ = false;
      pos->registrySize_ = 0;
      delete [] tempbuffer;
      return;
    }
    for(int j=0; j < sz; j++)
      tempbuffer[j] = msg->dataPtr()[j];
    // do not need to remake the Header for the leading frame/fragment
    // as InitMsg does not contain fragment count and total size
    int next_index = sz;
    toolbox::mem::Reference *curr = 0;
    toolbox::mem::Reference *prev = head;
    for(int i=0; i < (int)(msg->numFrames)-1 ; i++)
    {
      FDEBUG(9) << "copyAndTestRegistry: getting data for frame " << i+1 << std::endl;
      curr = prev->getNextReference(); // should test if this exists!

      I2O_MESSAGE_FRAME         *stdMsgcurr =
        (I2O_MESSAGE_FRAME*)curr->getDataLocation();
      I2O_SM_PREAMBLE_MESSAGE_FRAME *msgcurr    =
        (I2O_SM_PREAMBLE_MESSAGE_FRAME*)stdMsgcurr;

      FDEBUG(9) << "copyAndTestRegistry: datasize = " << msgcurr->dataSize << std::endl;
      int sz = msgcurr->dataSize;
      totalsize2check = totalsize2check + sz;
      if(totalsize2check > origsize) {
        std::cerr << "copyAndTestRegistry: total registry fragment size " << sz
               << " is larger than original size " << origsize 
               << " abort copy and test" << std::endl;
        pos->regCheckedOK_ = false;
        pos->registrySize_ = 0;
        delete [] tempbuffer;
        return;
      }
      for(int j=0; j < sz; j++)
        tempbuffer[next_index+j] = msgcurr->dataPtr()[j];
      next_index = next_index + sz;
      prev = curr;
    }
    if(totalsize2check != origsize) {
       std::cerr << "copyAndTestRegistry: Error! Remade registry size " << totalsize2check
                 << " not equal to original size " << origsize << std::endl;
    }
    // tempbuffer is filled with whole chain data
    pos->registrySize_ = origsize; // should already be done
    copy(tempbuffer, tempbuffer+origsize, pos->registryData_);
  } else {
    FDEBUG(9) << "copyAndTestRegistry: populating registry buffer from single frame for "
               << msg->hltURL << " and Tid " << msg->hltTid << std::endl;
    FDEBUG(9) << "copyAndTestRegistry: getting data for frame 0" << std::endl;
    FDEBUG(9) << "copyAndTestRegistry: datasize = " << msg->dataSize << std::endl;
    int sz = msg->dataSize;
    for(int j=0; j < sz; j++)
      tempbuffer[j] = msg->dataPtr()[j];
    // tempbuffer is filled with all data
    pos->registrySize_ = origsize; // should already be done
    copy(tempbuffer, tempbuffer+origsize, pos->registryData_);
  }
  // now test this registry against the SM config one
// -- don't do the test yet - need to save and the first and test against that instead
  try
  { // to test registry can seg fault if registry is bad!
    // check the registry from this FU against the configuration one
    // need to make a copy in a non-const array
    //bool setcode = false;
    //char* tempregdata = new char[registrySize];
    //copy(registryData, registryData+registrySize, tempregdata);
    //edm::InitMsg msg(&tempregdata[0],registrySize,setcode);
 //-   edm::InitMsg testmsg(&tempbuffer[0],origsize,setcode);
    // use available methods to check registry is a subset
 //-    edm::JobHeaderDecoder decoder;
 //-   std::auto_ptr<edm::SendJobHeader> header = decoder.decodeJobHeader(testmsg);
    //if(edm::registryIsSubset(*header, jc_->products()))
 //-   if(edm::registryIsSubset(*header, jc_->smproducts()))
 //-   {
      FDEBUG(9) << "copyAndTestRegistry: Received registry is okay" << std::endl;
      pos->regCheckedOK_ = true;

      // 01-Aug-2006, KAB: save the HLT and L1 trigger bit lengths
      InitMsgView initView(&tempbuffer[0]);
      hltBitCount = initView.get_hlt_bit_cnt();
      l1BitCount = initView.get_l1_bit_cnt();
      // KAB end

      // save the correct serialized product registry for Event Server
      if(ser_prods_size_ == 0)
      {
        for(int i=0; i<(int)origsize; i++)
          serialized_prods_[i]=tempbuffer[i];
        ser_prods_size_ = origsize;
        FDEBUG(9) << "Saved serialized registry for Event Server, size " 
                  << ser_prods_size_ << std::endl;
        // write out the registry once for file and run
        // should check this is the first thing written to the file!
        //ost_.write((const char*)(&ser_prods_size_),sizeof(int));
        ost_.write((const char*)serialized_prods_,ser_prods_size_);
      }
 //-   } else {
 //-     FDEBUG(9) << "copyAndTestRegistry: Error! Received registry is not a subset!"
 //-               << " for URL " << pos->hltURL_ << " and Tid " << pos->hltTid_
 //-               << std::endl;
 //-     LOG4CPLUS_INFO(this->getApplicationLogger(),
 //-       "copyAndTestRegistry: Error! Received registry is not a subset!"
 //-        << " for URL " << pos->hltURL_ << " and Tid " << pos->hltTid_);
 //-     pos->regCheckedOK_ = false;
 //-     pos->registrySize_ = 0;
 //-   }
  }
  catch(...)
  {
    std::cerr << "copyAndTestRegistry: Error! Received registry is not a subset!"
              << " Or caught problem checking registry! "
              << " for URL " << pos->hltURL_ << " and Tid " << pos->hltTid_
              << std::endl;
    LOG4CPLUS_INFO(this->getApplicationLogger(),
        "copyAndTestRegistry: Error! Received registry is not a subset!"
         << " Or caught problem checking registry! "
         << " for URL " << pos->hltURL_ << " and Tid " << pos->hltTid_);
    pos->regCheckedOK_ = false;
    pos->registrySize_ = 0;
    delete [] tempbuffer;  // does a throw basically immediately return?
    throw; // do we want to do this?
  }
  delete [] tempbuffer;
}

void stor::testI2OReceiver::updateFUSender4data(const char* hltURL,
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
    list<SMFUSenderList>::iterator foundPos;
    for(list<SMFUSenderList>::iterator pos = smfusenders_.begin();
        pos != smfusenders_.end(); ++pos)
    {
//      if(pos->hltLocalId_ == hltLocalId && pos->hltInstance_ == hltInstance &&
//         pos->hltTid_ == hltTid)
      if(pos->hltLocalId_ == hltLocalId && pos->hltInstance_ == hltInstance &&
         pos->hltTid_ == hltTid && pos->sameURL(hltURL) &&
         pos->sameClassName(hltClassName))
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
      // check if run number is same which came with configuration, complain otherwise !!!
      // this->runNumber_ comes from the RunBase class that testI2OReceiver inherits from
      if(runNumber != runNumber_)
	{
	  LOG4CPLUS_ERROR(this->getApplicationLogger(),"Run Number from event stream = " << runNumber
			  << " From " << hltURL 
			  << " Different from Run Number from configuration = " << runNumber_);
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
	storedEvents_.value_++;
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
	    storedEvents_.value_++;	
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
      LOG4CPLUS_INFO(this->getApplicationLogger(),
                 "updateFUSender4data: Cannot find FU in FU Sender list!"
                 << " With URL "
                 << hltURL << " class " << hltClassName  << " instance "
                 << hltInstance << " Tid " << hltTid);
    }
  } else {
    // problem: did not find an entry!!
    FDEBUG(10) << "updateFUSender4data: No entries at all in FU sender list!"
               << std::endl;
  }
}


void stor::testI2OReceiver::removeFUSender(const char* hltURL,
  const char* hltClassName, const unsigned long hltLocalId,
  const unsigned long hltInstance, const unsigned long hltTid)
{
  // Find this FU sender in the list
  if(!smfusenders_.empty())
  {
    LOG4CPLUS_INFO(this->getApplicationLogger(),"removing FU sender at " << hltURL);
    bool fusender_found = false;
    list<SMFUSenderList>::iterator foundPos;
    for(list<SMFUSenderList>::iterator pos = smfusenders_.begin();
        pos != smfusenders_.end(); ++pos)
    {
//      if(pos->hltLocalId_ == hltLocalId && pos->hltInstance_ == hltInstance &&
//         pos->hltTid_ == hltTid)
      if(pos->hltLocalId_ == hltLocalId && pos->hltInstance_ == hltInstance &&
         pos->hltTid_ == hltTid && pos->sameURL(hltURL) &&
         pos->sameClassName(hltClassName))
      {
        fusender_found = true;
        foundPos = pos;
      }
    }
    if(fusender_found)
    {
      smfusenders_.erase(foundPos);
    }
    else
      {
	LOG4CPLUS_ERROR(this->getApplicationLogger(),
			"Spurious end-of-run received for FU not in Sender list!"
			<< " With URL "
			<< hltURL << " class " << hltClassName  << " instance "
			<< hltInstance << " Tid " << hltTid);
	
      }
  }
  else
    LOG4CPLUS_ERROR(this->getApplicationLogger(),
		    "end-of-run received for FU but no sender in Sender list!"
		    << " With URL "
		    << hltURL << " class " << hltClassName  << " instance "
		    << hltInstance << " Tid " << hltTid);
  

}


////////////////////////////// Performance      ////////////////////////////
void testI2OReceiver::addMeasurement(unsigned long size)
{
  // for bandwidth performance measurements
  if ( pmeter_->addSample(size) )
  {
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
void testI2OReceiver::defaultWebPage(xgi::Input *in, xgi::Output *out)
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
        *out << "<tr>" << std::endl;
          *out << "<td >" << std::endl;
          *out << "Events Received" << std::endl;
          *out << "</td>" << std::endl;
          *out << "<td align=right>" << std::endl;
          *out << eventcounter_ << std::endl;
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
// Event Server Statistics
/*
    *out << "  <tr>"                                                   << endl;
    *out << "    <th colspan=2>"                                       << endl;
    *out << "      " << "Event Server saving one in  " << oneinN_ << " events" << endl;
    *out << "    </th>"                                                << endl;
    *out << "  </tr>"                                                  << endl;
      if(ser_prods_size_ != 0) {
        if(jc_.use_count() != 0) {
          *out << "<tr>" << endl;
            *out << "<td >" << endl;
            *out << "Ring Buffer is empty?" << endl;
            *out << "</td>" << endl;
            *out << "<td>" << endl;
            if(jc_->isEmpty()) *out << "Y" << endl;
            else *out << "N" << endl;
            *out << "</td>" << endl;
          *out << "  </tr>" << endl;
          *out << "<tr>" << endl;
            *out << "<td >" << endl;
            *out << "Ring Buffer is full?" << endl;
            *out << "</td>" << endl;
            *out << "<td>" << endl;
            if(jc_->isFull()) *out << "Y" << endl;
            else *out << "N" << endl;
            *out << "</td>" << endl;
          *out << "  </tr>" << endl;
        }
      }
*/

  *out << "</table>" << std::endl;

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
      for(list<SMFUSenderList>::iterator pos = smfusenders_.begin();
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
          if(pos->regAllReceived_) {
            *out << "All Received" << endl;
          } else {
            *out << "Partially received" << endl;
          }
          *out << "</td>" << endl;
        *out << "  </tr>" << endl;
        *out << "<tr>" << endl;
          *out << "<td >" << endl;
          *out << "Product registry" << endl;
          *out << "</td>" << endl;
          *out << "<td>" << endl;
          if(pos->regCheckedOK_) {
            *out << "Checked OK" << endl;
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
/*
  *out << "<hr/>"                                                 << endl;
  std::string url = getApplicationDescriptor()->getContextDescriptor()->getURL();
  std::string urn = getApplicationDescriptor()->getURN();
  *out << "<a href=\"" << url << "/" << urn << "/fusenderlist" << "\">"
       << "FU Sender list web page" << "</a>" << endl;
  *out << "<hr/>"                                                 << endl;
*/
  /*
  *out << "<a href=\"" << url << "/" << urn << "/geteventdata" << "\">"
       << "Get an event via a web page" << "</a>" << endl;
  *out << "<hr/>"                                                 << endl;
  *out << "<a href=\"" << url << "/" << urn << "/getregdata" << "\">"
       << "Get a header via a web page" << "</a>" << endl;
  */

  *out << "</body>"                                                  << endl;
  *out << "</html>"                                                  << endl;
}


////////////////////////////// get event data web page ////////////////////////////
void testI2OReceiver::eventdataWebPage(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception)
{
  //char buffer[7000000]; // change me to a data member!!!!!
  int len=0;
  bool empty = true;

  // should first test if storageManager is in halted state

  if(ser_prods_size_ != 0) 
  {
    {
      boost::mutex::scoped_lock sl(halt_lock_);
      empty = evtsrv_area_.isEmpty();
    }

    if(empty)
    {// do what? For now return a zero length stream. Should return MsgCode NODATA
      //len = 0;
      out->getHTTPResponseHeader().addHeader("Content-Type", "application/octet-stream");
      out->getHTTPResponseHeader().addHeader("Content-Transfer-Encoding", "binary");
      out->write(mybuffer_,len);
    }
    else
    {
      {
        boost::mutex::scoped_lock sl(halt_lock_);
	EventMsgView msgView = evtsrv_area_.pop_front();
	unsigned char* pos = (unsigned char*) &mybuffer_[0];
	unsigned char* from = msgView.startAddress();
	int dsize = msgView.size();
	copy(from,from+dsize,pos);
        len = dsize;
        FDEBUG(10) << "sending event " << msgView.event() << std::endl;
      }

      out->getHTTPResponseHeader().addHeader("Content-Type", "application/octet-stream");
      out->getHTTPResponseHeader().addHeader("Content-Transfer-Encoding", "binary");
      out->write(mybuffer_,len);
    }
  } // else send end of run as reponse
  else
  {
    OtherMessageBuilder othermsg(&mybuffer_[0],Header::DONE);
    len = othermsg.size();
    out->getHTTPResponseHeader().addHeader("Content-Type", "application/octet-stream");
    out->getHTTPResponseHeader().addHeader("Content-Transfer-Encoding", "binary");
    out->write(mybuffer_,len);
  }

// How to block if there is no data
// How to signal if end, and there will be no more data?

}
////////////////////////////// get header (registry) web page ////////////////////////////
void testI2OReceiver::headerdataWebPage(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception)
{
  // Need to use the saved serialzied registry
  // should really serialize the one in jc_ JobController instance
  if(ser_prods_size_ == 0)
  { // not available yet - return zero length stream, should return MsgCode NOTREADY
    int len = 0;
    //char buffer[2000000];
    out->getHTTPResponseHeader().addHeader("Content-Type", "application/octet-stream");
    out->getHTTPResponseHeader().addHeader("Content-Transfer-Encoding", "binary");
    out->write(mybuffer_,len);
  } else {
    int len = ser_prods_size_;
    //char buffer[2000000];
    for (int i=0; i<len; i++) mybuffer_[i]=serialized_prods_[i];

    out->getHTTPResponseHeader().addHeader("Content-Type", "application/octet-stream");
    out->getHTTPResponseHeader().addHeader("Content-Transfer-Encoding", "binary");
    out->write(mybuffer_,len);
  }

// How to block if there is no header data
// How to signal if not yet started, so there is no registry yet?
}



/**
 * Provides factory method for the instantiation of SM applications
 */

extern "C" xdaq::Application * instantiate_testI2OReceiver(xdaq::ApplicationStub * stub )
{
        std::cout << "Going to construct a testI2OReceiver instance " << endl;
        return new testI2OReceiver(stub);
}

