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
     version 1.9 2006/03/30
       Using own SMStateMachine instead of the EPStateMachine from
       the event filter processor. Added proof of principle
       implementation of an Event Server. HTTP gets from a cmsRun
       using the EventStreamHttpReader is reponded to by a binary
       octet-stream containing the serialized event in a edm::EventMsg. 
       The first valid product registry edm::InitMsg is saved so it can 
       be sent when requested.
          Events to consumers are sent from a ring buffer of size 10.
       Only 1 in every N events are saved into the ring buffer. For 
       this test implementation one cannot select on trigger bits. 
       N is 10 by default but can be overridden by setting oneinN in
       the testStorageManager section of the xdaq config xml file.

*/

#include <exception>
#include <iostream>
#include <string>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/statfs.h>
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
#include "IOPool/Streamer/interface/MsgHeader.h"
#include "IOPool/Streamer/interface/InitMessage.h"
#include "IOPool/Streamer/interface/StreamTranslator.h"
#include "IOPool/Streamer/interface/OtherMessage.h"
#include "EventFilter/StorageManager/interface/JobController.h"
#include "PluginManager/PluginManager.h"

#include "EventFilter/StorageManager/interface/i2oStorageManagerMsg.h"

#include "IOPool/Streamer/interface/ConsRegMessage.h"
#include "EventFilter/StorageManager/interface/ConsumerPipe.h"

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
// check code for INIT message 
// and all messages work like this (going into a queue)
  // do not delete the memory for the single (first) INIT message
  // it is stored in the local data member for event server
  if(entry->code_ != Header::INIT) 
  {
    toolbox::mem::Reference *ref=(toolbox::mem::Reference*)entry->buffer_object_;
    ref->release();
  }
}
// -----------------------------------------------

using namespace stor;

testStorageManager::testStorageManager(xdaq::ApplicationStub * s)
  throw (xdaq::exception::Exception): xdaq::Application(s),
  fsm_(0), ah_(0), writeStreamerOnly_(false), connectedFUs_(0), storedEvents_(0)
{
  LOG4CPLUS_INFO(this->getApplicationLogger(),"Making testStorageManager");

  ah_ = new edm::AssertHandler();
  fsm_ = new stor::SMStateMachine(getApplicationLogger());
  fsm_->init<testStorageManager>(this);
  xdata::InfoSpace *ispace = getApplicationInfoSpace();
  // default configuration
  ispace->fireItemAvailable("STparameterSet",&offConfig_);
  ispace->fireItemAvailable("FUparameterSet",&fuConfig_);
  ispace->fireItemAvailable("runNumber",&runNumber_);
  ispace->fireItemAvailable("stateName",&fsm_->stateName_);

  ispace->fireItemAvailable("connectedFUs",&connectedFUs_);
  ispace->fireItemAvailable("storedEvents",&storedEvents_);

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
  xgi::bind(this,&testStorageManager::streamerOutputWebPage, "streameroutput");
  xgi::bind(this,&testStorageManager::eventdataWebPage, "geteventdata");
  xgi::bind(this,&testStorageManager::headerdataWebPage, "getregdata");
  xgi::bind(this,&testStorageManager::consumerWebPage, "registerConsumer");


  eventcounter_ = 0;
  framecounter_ = 0;
  pool_is_set_ = 0;
  pool_ = 0;

// Variables needed for streamer file writing
// should be getting these from SM config file - put them in xml for now
// until we do it in StreamerOutputService ctor
  ispace->fireItemAvailable("streamerOnly",&streamer_only_);
  ispace->fireItemAvailable("filePath",&filePath_);
  ispace->fireItemAvailable("mailboxPath",&mailboxPath_);
  ispace->fireItemAvailable("setupLabel",&setupLabel_);
  ispace->fireItemAvailable("streamLabel",&streamLabel_);
  ispace->fireItemAvailable("maxFileSize",&maxFileSize_);
  ispace->fireItemAvailable("highWaterMark",&highWaterMark_);
  // default only here - actually set configureAction if defined in XML file
  path_ = "./";
  mpath_ = "./"; //mailbox path
  setup_ = "cms";
  stream_ = "main";
  maxFileSize_ = 1073741824;
  highWaterMark_ = 0.9;

  // added for Event Server
  ser_prods_size_ = 0;
  serialized_prods_[0] = '\0';
  oneinN_ = 10;
  ispace->fireItemAvailable("oneinN",&oneinN_);
  maxESEventRate_ = 10.0;  // hertz
  ispace->fireItemAvailable("maxESEventRate",&maxESEventRate_);
  activeConsumerTimeout_ = 300;  // seconds
  ispace->fireItemAvailable("activeConsumerTimeout",&activeConsumerTimeout_);
  idleConsumerTimeout_ = 600;  // seconds
  ispace->fireItemAvailable("idleConsumerTimeout",&idleConsumerTimeout_);
  consumerQueueSize_ = 5;
  ispace->fireItemAvailable("consumerQueueSize",&consumerQueueSize_);

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

  string xmlClass_ = getApplicationDescriptor()->getClassName();
  unsigned long instance_ = getApplicationDescriptor()->getInstance();
  ostringstream sourcename;
  sourcename << xmlClass_ << "_" << instance_;
  sourceId_ = sourcename.str();
}

testStorageManager::~testStorageManager()
{
  delete fsm_;
  delete ah_;
  delete pmeter_;
}

xoap::MessageReference testStorageManager::ParameterGet(xoap::MessageReference message)	throw (xoap::exception::Exception)
{
  connectedFUs_.value_ = smfusenders_.size();
  return Application::ParameterGet(message);
}
#include "FWCore/ServiceRegistry/interface/ServiceToken.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "EventFilter/Utilities/interface/ModuleWebRegistry.h"

#include "EventFilter/Utilities/interface/ModuleWebRegistry.h"
#include "EventFilter/Utilities/interface/ParameterSetRetriever.h"

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
  evf::ParameterSetRetriever fupset(fuConfig_.value_);
  evf::ParameterSetRetriever smpset(offConfig_.value_);
  string sample_config = fupset.getAsString();
  string my_config = smpset.getAsString();

  // For streamer file writing to make it more like OutServ for POOL root files
  // we should pass the my_config string to the streamer writer
  // which is created by the fragment collector.
  // Need to find out how to parse the config string there p for now do:
  if(streamer_only_.toString() == "true" || streamer_only_.toString() == "TRUE" ||
     streamer_only_.toString() == "True" ) {
    LOG4CPLUS_INFO(this->getApplicationLogger(),"Writing Streamer files");
    writeStreamerOnly_ = true;
  } else if(streamer_only_.toString() == "false" || streamer_only_.toString() == "FALSE" ||
     streamer_only_.toString() == "False" ) {
    LOG4CPLUS_INFO(this->getApplicationLogger(),"Writing PoolOutputModule ROOT files");
    writeStreamerOnly_ = false;
  } else {
    LOG4CPLUS_WARN(this->getApplicationLogger(),
	           "Unrecognized streamerOnly option "
		       << streamer_only_.toString() << " will use default false");
    LOG4CPLUS_INFO(this->getApplicationLogger(),"Using Output Module");
    writeStreamerOnly_ = false;
  }
  smConfigString_ = my_config;
  path_ = filePath_.toString();
  mpath_ = mailboxPath_.toString();
  setup_ = setupLabel_.toString();
  stream_ = streamLabel_.toString();
  FDEBUG(9) << "Streamer filename run number = " << runNumber_ << endl;
  std::ostringstream stm;
  stm << setup_ << "." << setfill('0') << std::setw(8) << runNumber_
      << "." << stream_ << "." << sourceId_;
  filen_ = stm.str();
  FDEBUG(9) << "Streamer filename starts with = " << filen_ << endl;

// HEREHERE
  // the rethrows below need to be XDAQ exception types (JBK)

  // 10-Aug-2006, KAB: ensure reasonable values for event server params
  if (maxESEventRate_ < 0.0) maxESEventRate_ = 0.0;
  xdata::Integer cutoff(1);
  if (consumerQueueSize_ < cutoff) consumerQueueSize_ = cutoff;
  //cout << "maxESEventRate = " << maxESEventRate_ << endl;
  //cout << "consumerQueueSize = " << consumerQueueSize_ << endl;

  try {
    jc_.reset(new stor::JobController(sample_config,
                                      my_config, &deleteSMBuffer));
    // added for Event Server
    int value_4oneinN(oneinN_);
    if(value_4oneinN <= 0) value_4oneinN = -1;
    jc_->set_oneinN(value_4oneinN);
    jc_->set_outoption(writeStreamerOnly_);
    unsigned long max(maxFileSize_);
    double high(highWaterMark_);
    // check that the directories exist
    struct stat buf;
    int retVal = stat(path_.c_str(), &buf);
    if(retVal !=0)
    {
      edm::LogWarning("testStorageManager") << "Output directory " << path_ 
            << " does not exist. Error=" << errno ;
    }
    jc_->set_outfile(filen_, max, high, path_, mpath_);

    boost::shared_ptr<EventServer>
      eventServer(new EventServer(value_4oneinN, maxESEventRate_));
    jc_->setEventServer(eventServer);
  }
  catch(cms::Exception& e)
    {
      XCEPT_RAISE (toolbox::fsm::exception::Exception, 
		   e.explainSelf());
    }
  catch(seal::Error& e)
    {
      XCEPT_RAISE (toolbox::fsm::exception::Exception, 
		   e.explainSelf());
    }
  catch(std::exception& e)
    {
      XCEPT_RAISE (toolbox::fsm::exception::Exception, 
		   e.what());
    }
  catch(...)
  {
      XCEPT_RAISE (toolbox::fsm::exception::Exception, 
		   "Unknown Exception");
  }
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
  // Check that all senders have closed connections
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
  jc_->stop();
  jc_->join();
  // we must destroy the eventprocessor to finish processing

  // make sure serialized product registry is cleared also
  ser_prods_size_ = 0;

  boost::mutex::scoped_lock sl(halt_lock_);
  jc_.reset();
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
  //std::string temp4print(msg->dataPtr(),msg->dataSize);
  //FDEBUG(10) << "testStorageManager: registry data = " << temp4print << std::endl;
  
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
  // std::cout << "received data message from " << msg->hltURL << std::endl;
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
//HEREHERE must give it the 1 of N for this fragment (starts from 0 in i2o header)
         new (b.buffer()) stor::FragEntry(thisref, (char*)(thismsg->dataPtr()), thislen,
                  thismsg->frameCount+1, thismsg->numFrames, Header::EVENT, thismsg->eventID);
         b.commit(sizeof(stor::FragEntry));
//HEREHERE
         framecounter_++;
         // for bandwidth performance measurements
         // Following is wrong for the last frame because frame sent is
         // is actually larger than the size taken by actual data
         unsigned long actualFrameSize = (unsigned long)sizeof(I2O_SM_DATA_MESSAGE_FRAME)
                                         +thislen;
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
//HEREHERE must give it the 1 of N for this fragment (starts from 0 in i2o header)
    /* stor::FragEntry* fe = */ new (b.buffer()) stor::FragEntry(ref, (char*)(msg->dataPtr()), len,
                                msg->frameCount+1, msg->numFrames, Header::EVENT, msg->eventID);
    b.commit(sizeof(stor::FragEntry));
//HEREHERE
    // Frame release is done in the deleter.
    framecounter_++;
    // for bandwidth performance measurements
    // Following is wrong for the last frame because frame sent is
    // is actually larger than the size taken by actual data
    unsigned long actualFrameSize = (unsigned long)sizeof(I2O_SM_DATA_MESSAGE_FRAME)
                                    + len;
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
  FDEBUG(9) << "testStorageManager: Received other message from HLT " << msg->hltURL
             << " application " << msg->hltClassName << " id " << msg->hltLocalId
             << " instance " << msg->hltInstance << " tid " << msg->hltTid << std::endl;
  FDEBUG(9) << "testStorageManager: message content " << msg->otherData << "\n";
  // Not yet processing any Other messages type
  // the only "other" message is an end-of-run. It is awaited to process a request to halt the storage manager
  
  removeFUSender(&msg->hltURL[0], &msg->hltClassName[0],
		 msg->hltLocalId, msg->hltInstance, msg->hltTid);
  
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
  const unsigned long frameCount, const unsigned long numFrames,
  const unsigned long registrySize, const char* registryData,
  toolbox::mem::Reference *ref)
{  
  // no longer need to pass the registry pointer - can take it out
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
                << frameCount << " for URL "
                << hltURL << " and Tid " << hltTid << std::endl;
      // register this FU sender
      SMFUSenderList *fusender = new SMFUSenderList(hltURL, hltClassName,
                     hltLocalId, hltInstance, hltTid, numFrames,
                     registrySize, registryData);
      smfusenders_.push_back(*fusender);
      LOG4CPLUS_INFO(this->getApplicationLogger(),"register FU sender at " << hltURL << " list size is " << smfusenders_.size());
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
      FDEBUG(10) << "registerFUSender: found another frame " << frameCount << " for URL "
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
    LOG4CPLUS_INFO(this->getApplicationLogger(),"register FU sender at " << hltURL << " list size is " << smfusenders_.size());

    smfusenders_.back().chrono_.start(0);
    smfusenders_.back().totFrames_ = numFrames;
    smfusenders_.back().currFrames_ = 1;
    smfusenders_.back().frameRefs_[frameCount] = ref;
    // now should check if frame is complete and deal with it
    list<SMFUSenderList>::iterator testPos = --smfusenders_.end();
    testCompleteFUReg(testPos);
  }
}

void stor::testStorageManager::testCompleteFUReg(list<SMFUSenderList>::iterator pos)
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
        // can crash here is first received frame is not first frame!
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


void stor::testStorageManager::copyAndTestRegistry(list<SMFUSenderList>::iterator pos,
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
  try
  { // to test registry can seg fault if registry is bad!
    // check the registry from this FU against the configuration one
    // need to make a copy in a non-const array
    //bool setcode = false;
    //char* tempregdata = new char[registrySize];
    //copy(registryData, registryData+registrySize, tempregdata);
    //edm::InitMsg msg(&tempregdata[0],registrySize,setcode);
//HEREHERE
    //edm::InitMsg testmsg(&tempbuffer[0],origsize,setcode);
    InitMsgView testmsg(&tempbuffer[0]);
    // use available methods to check registry is a subset
    //edm::JobHeaderDecoder decoder;
    //std::auto_ptr<edm::SendJobHeader> header = decoder.decodeJobHeader(testmsg);
    std::auto_ptr<edm::SendJobHeader> header = StreamTranslator::deserializeRegistry(testmsg);
// HEREHEREHERE
// put init message into queue if it is the first (see below)
//    stor::FragEntry(ref, (char*)(msg->dataPtr()), len,
//                                          msg->frameCount+1, msg->numFrames);
// make copy of bytes and deleter for it to put into queue
// to HEREHERE?
    //if(edm::registryIsSubset(*header, jc_->products()))
    if(edm::registryIsSubset(*header, jc_->smproducts()))
    {
      FDEBUG(9) << "copyAndTestRegistry: Received registry is okay" << std::endl;
      pos->regCheckedOK_ = true;
      // save the correct serialized product registry for Event Server
      if(ser_prods_size_ == 0)
      {
        for(int i=0; i<(int)origsize; i++)
          serialized_prods_[i]=tempbuffer[i];
        ser_prods_size_ = origsize;
        FDEBUG(9) << "Saved serialized registry for Event Server, size " 
                  << ser_prods_size_ << std::endl;
//HEREHEREHERE
        // this is the first serialized registry coming over, queue for output
        if(writeStreamerOnly_)
        {
          EventBuffer::ProducerBuffer b(jc_->getFragmentQueue());
          new (b.buffer()) stor::FragEntry(&serialized_prods_[0], &serialized_prods_[0], ser_prods_size_,
                                1, 1, Header::INIT, 0); // use fixed 0 as ID
          b.commit(sizeof(stor::FragEntry));
        }
        // with it to get the hlt and l1 cnt for fragColl and get
        // init message to fragcoll to write it out
        jc_->set_hlt_bit_count(testmsg.get_hlt_bit_cnt());
        jc_->set_l1_bit_count(testmsg.get_l1_bit_cnt());
      }
    } else {
      FDEBUG(9) << "copyAndTestRegistry: Error! Received registry is not a subset!"
                << " for URL " << pos->hltURL_ << " and Tid " << pos->hltTid_
                << std::endl;
      LOG4CPLUS_INFO(this->getApplicationLogger(),
        "copyAndTestRegistry: Error! Received registry is not a subset!"
         << " for URL " << pos->hltURL_ << " and Tid " << pos->hltTid_);
      pos->regCheckedOK_ = false;
      pos->registrySize_ = 0;
    }
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
      // this->runNumber_ comes from the RunBase class that testStorageManager inherits from
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


void stor::testStorageManager::removeFUSender(const char* hltURL,
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
        *out << "<tr>" << endl;
          *out << "<td >" << endl;
          *out << "Events Received" << endl;
          *out << "</td>" << endl;
          *out << "<td align=right>" << endl;
          *out << storedEvents_ << endl;
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
// Event Server Statistics
    *out << "  <tr>"                                                   << endl;
    *out << "    <th colspan=2>"                                       << endl;
    *out << "      " << "Event Server saving one in  " << oneinN_ << " events" << endl;
    *out << "    </th>"                                                << endl;
    *out << "  </tr>"                                                  << endl;
// should first test if jc_ is valid
      if(ser_prods_size_ != 0) {
        boost::mutex::scoped_lock sl(halt_lock_);
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
  *out << "<hr/>"                                                 << endl;
  std::string url = getApplicationDescriptor()->getContextDescriptor()->getURL();
  std::string urn = getApplicationDescriptor()->getURN();
  *out << "<a href=\"" << url << "/" << urn << "/fusenderlist" << "\">" 
       << "FU Sender list web page" << "</a>" << endl;
  *out << "<hr/>"                                                 << endl;
  *out << "<a href=\"" << url << "/" << urn << "/streameroutput" << "\">" 
       << "Streamer Output Status web page" << "</a>" << endl;
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

// now for FU sender list statistics: put this in!
  *out << "fu sender table not done yet"                             << endl;

  *out << "</body>"                                                  << endl;
  *out << "</html>"                                                  << endl;
}
////////////////////////////// streamer file output web page ////////////////////////////
void testStorageManager::streamerOutputWebPage(xgi::Input *in, xgi::Output *out)
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

    struct statfs64 buf;
    int retVal = statfs64(path_.c_str(), &buf);
    if(retVal!=0)
      edm::LogWarning("OutServ") << "Could not stat output filesystem for path " << path_ << std::endl;

    unsigned long btotal = 0;
    unsigned long bfree = 0;
    unsigned long blksize = 0;
    if(retVal==0)
    {
      blksize = buf.f_bsize;
      btotal = buf.f_blocks;
      bfree  = buf.f_bfree;
    }
  *out << "<P>Current Path= " << path_                                   << endl;
  *out << "<P>Current mailBoxPath= " << mpath_                           << endl;

  *out << "<P>FileSystem status: " << setw(5) 
       << (float(bfree)/float(btotal))*100. 
       << "% free "                                                  << endl;
  
  // should first test if jc_ is valid
      if(ser_prods_size_ != 0) {
        boost::mutex::scoped_lock sl(halt_lock_);
        if(jc_.use_count() != 0) {
          std::list<std::string>& files = jc_->get_filelist();
          std::list<std::string>& currfiles = jc_->get_currfiles();

          *out << "<P>#    name                             evt        size     " << endl;
          for(list<string>::const_iterator it = files.begin();
              it != files.end(); it++)
              *out << "<P> " <<*it << endl;
          for(list<string>::const_iterator it = currfiles.begin();
              it != files.end(); it++)
              *out << "<P>CurrentFile = " <<*it << endl;
        }
      }

  *out << "</body>"                                                  << endl;
  *out << "</html>"                                                  << endl;
}
////////////////////////////// get event data web page ////////////////////////////
void testStorageManager::eventdataWebPage(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception)
{
  // default the message length to zero
  int len=0;

  // 24-Aug-2006, KAB: determine the consumer ID from the event request
  // message, if it is available.
  unsigned int consumerId = 0;
  std::string lengthString = in->getenv("CONTENT_LENGTH");
  unsigned long contentLength = std::atol(lengthString.c_str());
  if (contentLength > 0) {
    auto_ptr< vector<char> > bufPtr(new vector<char>(contentLength));
    in->read(&(*bufPtr)[0], contentLength);
    OtherMessageView requestMessage(&(*bufPtr)[0]);
    if (requestMessage.code() == Header::EVENT_REQUEST)
    {
      uint8 *bodyPtr = requestMessage.msgBody();
      consumerId = convert32(bodyPtr);
    }
  }

  // should first test if testStorageManager is in halted state
  if(ser_prods_size_ != 0) 
  {
    if (consumerId == 0)
    {
      boost::mutex::scoped_lock sl(halt_lock_);
      if (! (jc_->isEmpty()))
      {
        EventMsgView msgView = jc_->pop_front();
        unsigned char* pos = (unsigned char*) &mybuffer_[0];
        unsigned char* from = msgView.startAddress();
        int dsize = msgView.size();

        copy(from,from+dsize,pos);
        len = dsize;
        FDEBUG(10) << "sending event " << msgView.event() << std::endl;
      }
    }
    else
    {
      boost::shared_ptr<EventServer> eventServer;
      if (jc_.get() != NULL)
      {
        eventServer = jc_->getEventServer();
      }
      if (eventServer.get() != NULL)
      {
        boost::shared_ptr< std::vector<char> > bufPtr =
          eventServer->getEvent(consumerId);
        if (bufPtr.get() != NULL)
        {
          EventMsgView msgView(&(*bufPtr)[0]);

          unsigned char* pos = (unsigned char*) &mybuffer_[0];
          unsigned char* from = msgView.startAddress();
          int dsize = msgView.size();

          copy(from,from+dsize,pos);
          len = dsize;
          FDEBUG(10) << "sending event " << msgView.event() << std::endl;
        }
      }
    }

    out->getHTTPResponseHeader().addHeader("Content-Type", "application/octet-stream");
    out->getHTTPResponseHeader().addHeader("Content-Transfer-Encoding", "binary");
    out->write(mybuffer_,len);
  } // else send end of run as reponse
  else
  {
// HEREHERE
    //edm::MsgCode msg(&mybuffer_[0], 4, edm::MsgCode::DONE);
    //len = msg.totalSize();
    // this is not working
    OtherMessageBuilder othermsg(&mybuffer_[0],Header::DONE);
    len = othermsg.size();
    //std::cout << "making other message code = " << othermsg.code()
    //          << " and size = " << othermsg.size() << std::endl;

// HEREHERE
    out->getHTTPResponseHeader().addHeader("Content-Type", "application/octet-stream");
    out->getHTTPResponseHeader().addHeader("Content-Transfer-Encoding", "binary");
    out->write(mybuffer_,len);
  }

// How to block if there is no data
// How to signal if end, and there will be no more data?

}
////////////////////////////// get header (registry) web page ////////////////////////////
void testStorageManager::headerdataWebPage(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception)
{
  // 10-Aug-2006, KAB: determine the consumer ID from the header request
  // message, if it is available.
  auto_ptr< vector<char> > httpPostData;
  unsigned int consumerId = 0;
  std::string lengthString = in->getenv("CONTENT_LENGTH");
  unsigned long contentLength = std::atol(lengthString.c_str());
  if (contentLength > 0) {
    auto_ptr< vector<char> > bufPtr(new vector<char>(contentLength));
    in->read(&(*bufPtr)[0], contentLength);
    OtherMessageView requestMessage(&(*bufPtr)[0]);
    if (requestMessage.code() == Header::HEADER_REQUEST)
    {
      uint8 *bodyPtr = requestMessage.msgBody();
      consumerId = convert32(bodyPtr);
    }

    // save the post data for use outside the "if" block scope in case it is
    // useful later (it will still get deleted at the end of the method)
    httpPostData = bufPtr;
  }

  // Need to use the saved serialzied registry
  // should really serialize the one in jc_ JobController instance
  if(ser_prods_size_ == 0)
  { // not available yet - return zero length stream, should return MsgCode NOTREADY
    int len = 0;
    out->getHTTPResponseHeader().addHeader("Content-Type", "application/octet-stream");
    out->getHTTPResponseHeader().addHeader("Content-Transfer-Encoding", "binary");
    out->write(mybuffer_,len);
  } else {

    // 31-Aug-2006, KAB: overlay an INIT message view on the serialized
    // products array so that we can initialize the consumer event selection
    InitMsgView initView(serialized_prods_);
    if (jc_.get() != NULL)
    {
      boost::shared_ptr<EventServer> eventServer = jc_->getEventServer();
      if (eventServer.get() != NULL)
      {
        boost::shared_ptr<ConsumerPipe> consPtr =
          eventServer->getConsumer(consumerId);
        if (consPtr.get() != NULL)
        {
          consPtr->initializeSelection(initView);
        }
      }
    }

    int len = ser_prods_size_;
    for (int i=0; i<len; i++) mybuffer_[i]=serialized_prods_[i];

    out->getHTTPResponseHeader().addHeader("Content-Type", "application/octet-stream");
    out->getHTTPResponseHeader().addHeader("Content-Transfer-Encoding", "binary");
    out->write(mybuffer_,len);
  }

// How to block if there is no header data
// How to signal if not yet started, so there is no registry yet?
}
////////////////////////////// consumer registration web page ////////////////////////////
void testStorageManager::consumerWebPage(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception)
{
  std::string consumerName = "None provided";
  std::string consumerPriority = "normal";
  std::string consumerRequest = "<>";

  // read the consumer registration message from the http input stream
  std::string lengthString = in->getenv("CONTENT_LENGTH");
  unsigned long contentLength = std::atol(lengthString.c_str());
  if (contentLength > 0)
  {
    auto_ptr< vector<char> > bufPtr(new vector<char>(contentLength));
    in->read(&(*bufPtr)[0], contentLength);
    ConsRegRequestView requestMessage(&(*bufPtr)[0]);
    consumerName = requestMessage.getConsumerName();
    consumerPriority = requestMessage.getConsumerPriority();
    std::string reqString = requestMessage.getRequestParameterSet();
    if (reqString.size() >= 2) consumerRequest = reqString;
  }

  // create the buffer to hold the registration reply message
  const int BUFFER_SIZE = 100;
  char msgBuff[BUFFER_SIZE];

  // fetch the event server
  // (it and/or the job controller may not have been created yet)
  boost::shared_ptr<EventServer> eventServer;
  if (jc_.get() != NULL)
  {
    eventServer = jc_->getEventServer();
  }

  // if no event server, tell the consumer that we're not ready
  if (eventServer.get() == NULL)
  {
    // build the registration response into the message buffer
    ConsRegResponseBuilder respMsg(msgBuff, BUFFER_SIZE,
                                   ConsRegResponseBuilder::ES_NOT_READY, 0);
    // debug message so that compiler thinks respMsg is used
    FDEBUG(20) << "Registration response size =  " <<
      respMsg.size() << std::endl;
  }
  else
  {
    // construct a parameter set from the consumer request
    boost::shared_ptr<edm::ParameterSet>
      requestParamSet(new edm::ParameterSet(consumerRequest));

    // create the local consumer interface and add it to the event server
    boost::shared_ptr<ConsumerPipe>
      consPtr(new ConsumerPipe(consumerName, consumerPriority,
                               activeConsumerTimeout_.value_,
                               idleConsumerTimeout_.value_,
                               requestParamSet));
    eventServer->addConsumer(consPtr);

    // build the registration response into the message buffer
    ConsRegResponseBuilder respMsg(msgBuff, BUFFER_SIZE,
                                   0, consPtr->getConsumerId());
    // debug message so that compiler thinks respMsg is used
    FDEBUG(20) << "Registration response size =  " <<
      respMsg.size() << std::endl;
  }

  // send the response
  ConsRegResponseView responseMessage(msgBuff);
  int len = responseMessage.size();
  for (int i=0; i<len; i++) mybuffer_[i]=msgBuff[i];

  out->getHTTPResponseHeader().addHeader("Content-Type", "application/octet-stream");
  out->getHTTPResponseHeader().addHeader("Content-Transfer-Encoding", "binary");
  out->write(mybuffer_,len);
}


/**
 * Provides factory method for the instantiation of SM applications
 */

extern "C" xdaq::Application * instantiate_testStorageManager(xdaq::ApplicationStub * stub )
{
        std::cout << "Going to construct a testStorageManager instance " << std::endl;
        return new stor::testStorageManager(stub);
}

