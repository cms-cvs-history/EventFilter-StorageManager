// $Id: StorageManager.cc,v 1.92.4.74 2009/04/09 11:26:12 mommsen Exp $

#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>

#include "EventFilter/StorageManager/interface/StorageManager.h"
#include "EventFilter/StorageManager/interface/ConsumerPipe.h"
#include "EventFilter/StorageManager/interface/FUProxy.h"
#include "EventFilter/StorageManager/interface/RunMonitorCollection.h"
#include "EventFilter/StorageManager/interface/FragmentMonitorCollection.h"
#include "EventFilter/StorageManager/interface/StateMachine.h"

#include "EventFilter/Utilities/interface/i2oEvfMsgs.h"
#include "EventFilter/Utilities/interface/ModuleWebRegistry.h"
#include "EventFilter/Utilities/interface/ModuleWebRegistry.h"

#include "FWCore/Utilities/interface/DebugMacros.h"
#include "FWCore/ServiceRegistry/interface/ServiceToken.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "FWCore/RootAutoLibraryLoader/interface/RootAutoLibraryLoader.h"
#include "FWCore/PluginManager/interface/PluginManager.h"
#include "FWCore/PluginManager/interface/standard.h"
#include "FWCore/ParameterSet/interface/PythonProcessDesc.h"

#include "IOPool/Streamer/interface/MsgHeader.h"
#include "IOPool/Streamer/interface/InitMessage.h"
#include "IOPool/Streamer/interface/OtherMessage.h"
#include "IOPool/Streamer/interface/ConsRegMessage.h"
#include "IOPool/Streamer/interface/HLTInfo.h"
#include "IOPool/Streamer/interface/Utilities.h"
#include "IOPool/Streamer/interface/StreamerInputSource.h"

#include "xcept/tools.h"

#include "i2o/Method.h"
#include "i2o/utils/AddressMap.h"

#include "toolbox/task/WorkLoopFactory.h"

#include "xcept/tools.h"

#include "xdaq2rc/RcmsStateNotifier.h"

#include "xdata/InfoSpaceFactory.h"

#include "xdaq/NamespaceURI.h"
#include "xoap/Method.h"

#include "xoap/SOAPEnvelope.h"
#include "xoap/SOAPBody.h"
#include "xoap/domutils.h"

#include "boost/lexical_cast.hpp"
#include "boost/algorithm/string/case_conv.hpp"
#include "cgicc/Cgicc.h"

#include <sys/statfs.h>
#include "zlib.h"

// namespace stor {
//   extern bool getSMFC_exceptionStatus();
//   extern std::string getSMFC_reason4Exception();
// }

using namespace edm;
using namespace std;
using namespace stor;


namespace 
{
  void
  parseFileEntry(const std::string &in, 
		 std::string &out, unsigned int &nev, unsigned long long &sz)
  {
    unsigned int no;
    stringstream pippo;
    pippo << in;
    pippo >> no >> out >> nev >> sz;
  }

  void
  checkDirectoryOK(xdata::String const& p)
  {
    struct stat64 buf;
    // The const-cast is needed because xdata::String::toString() is
    // not declared const.
    string path(const_cast<xdata::String&>(p).toString());
    
    int retVal = stat64(path.c_str(), &buf);
    if(retVal !=0 )
      {
	edm::LogError("StorageManager") << "Directory or file " << path
					<< " does not exist. Error=" << errno ;
	throw cms::Exception("StorageManager","checkDirectoryOK")
	  << "Directory or file " << path << " does not exist. Error=" << errno << std::endl;
      }
  }
}


StorageManager::StorageManager(xdaq::ApplicationStub * s)
  throw (xdaq::exception::Exception) :
  xdaq::Application(s),
  reasonForFailedState_(),
  ah_(0), 
  mybuffer_(7000000),
  connectedRBs_(0), 
  _wrapper_notifier( this ),
  _webPageHelper( getApplicationDescriptor(),
    "$Id: StorageManager.cc,v 1.92.4.74 2009/04/09 11:26:12 mommsen Exp $ $Name:  $")
{  
  LOG4CPLUS_INFO(this->getApplicationLogger(),"Making StorageManager");

  ah_   = new edm::AssertHandler();

  setupFlashList();

  xdata::InfoSpace *ispace = getApplicationInfoSpace();

  ispace->fireItemAvailable("connectedRBs",  &connectedRBs_);
  ispace->fireItemAvailable("receivedEventsForOutMod",  &receivedEventsFromOutMod_);
  ispace->fireItemAvailable("namesOfOutMod",      &namesOfOutMod_);

  // Bind specific messages to functions
  i2o::bind(this,
            &StorageManager::receiveRegistryMessage,
            I2O_SM_PREAMBLE,
            XDAQ_ORGANIZATION_ID);
  i2o::bind(this,
            &StorageManager::receiveDataMessage,
            I2O_SM_DATA,
            XDAQ_ORGANIZATION_ID);
  i2o::bind(this,
            &StorageManager::receiveErrorDataMessage,
            I2O_SM_ERROR,
            XDAQ_ORGANIZATION_ID);
  /* no longer used it seems? Don't delete yet
  i2o::bind(this,
            &StorageManager::receiveOtherMessage,
            I2O_SM_OTHER,
            XDAQ_ORGANIZATION_ID);
  */
  i2o::bind(this,
            &StorageManager::receiveDQMMessage,
            I2O_SM_DQM,
            XDAQ_ORGANIZATION_ID);

  // Bind callbacks:
  xoap::bind( this,
              &StorageManager::configuring,
              "Configure",
              XDAQ_NS_URI );
  xoap::bind( this,
              &StorageManager::enabling,
              "Enable",
              XDAQ_NS_URI );
  xoap::bind( this,
              &StorageManager::stopping,
              "Stop",
              XDAQ_NS_URI );
  xoap::bind( this,
              &StorageManager::halting,
              "Halt",
              XDAQ_NS_URI );

  // Bind web interface
  xgi::bind(this,&StorageManager::defaultWebPage,       "Default");
  xgi::bind(this,&StorageManager::oldDefaultWebPage,    "oldDefault");
  xgi::bind(this,&StorageManager::storedDataWebPage,    "storedData");
  xgi::bind(this,&StorageManager::css,                  "styles.css");
  xgi::bind(this,&StorageManager::rbsenderWebPage,      "rbsenderlist");
  xgi::bind(this,&StorageManager::fileStatisticsWebPage,"fileStatistics");
  xgi::bind(this,&StorageManager::eventdataWebPage,     "geteventdata");
  xgi::bind(this,&StorageManager::headerdataWebPage,    "getregdata");
  xgi::bind(this,&StorageManager::consumerWebPage,      "registerConsumer");
  xgi::bind(this,&StorageManager::consumerListWebPage,  "consumerList");
  xgi::bind(this,&StorageManager::DQMeventdataWebPage,  "getDQMeventdata");
  xgi::bind(this,&StorageManager::DQMconsumerWebPage,   "registerDQMConsumer");
  xgi::bind(this,&StorageManager::eventServerWebPage,   "EventServerStats");
  pool_is_set_    = 0;
  pool_           = 0;

  // for performance measurements

  receivedEventsFromOutMod_.reserve(10);
  receivedEventsFromOutMod_.clear();
  receivedEventsMap_.clear();
  avEventSizeMap_.clear();
  avCompressRatioMap_.clear();
  modId2ModOutMap_.clear();
  storedEventsMap_.clear();

  // need the line below so that deserializeRegistry can run
  // in order to compare two registries (cannot compare byte-for-byte) (if we keep this)
  // need line below anyway in case we deserialize DQMEvents for collation
  edm::RootAutoLibraryLoader::enable();

  // set application icon for hyperdaq
  getApplicationDescriptor()->setAttribute("icon", "/evf/images/smicon.jpg");

  sharedResourcesPtr_.reset(new SharedResources());

  unsigned long instance = getApplicationDescriptor()->getInstance();
  sharedResourcesPtr_->_configuration.reset(new Configuration(ispace,
                                                              instance));

  QueueConfigurationParams queueParams =
    sharedResourcesPtr_->_configuration->getQueueConfigurationParams();
  sharedResourcesPtr_->_commandQueue.
    reset(new CommandQueue(queueParams._commandQueueSize));
  sharedResourcesPtr_->_fragmentQueue.
    reset(new FragmentQueue(queueParams._fragmentQueueSize));
  sharedResourcesPtr_->_registrationQueue.
    reset(new RegistrationQueue(queueParams._registrationQueueSize));
  sharedResourcesPtr_->_streamQueue.
    reset(new StreamQueue(queueParams._streamQueueSize));

  sharedResourcesPtr_->_statisticsReporter.reset(new StatisticsReporter(this));
  sharedResourcesPtr_->_initMsgCollection.reset(new InitMsgCollection());
  sharedResourcesPtr_->_diskWriterResources.reset(new DiskWriterResources());

  sharedResourcesPtr_->_smRBSenderList = &smrbsenders_;

  sharedResourcesPtr_->
    _discardManager.reset(new DiscardManager(getApplicationContext(),
                                             getApplicationDescriptor()));

  // Start the workloops
  // TODO: add try/catch block and handle exceptions
  sharedResourcesPtr_->_statisticsReporter->startWorkLoop();

  fragmentProcessor_ = new FragmentProcessor( sharedResourcesPtr_,
                                              _wrapper_notifier );
  fragmentProcessor_->startWorkLoop(utils::getIdentifier(getApplicationDescriptor()));

  diskWriter_ = new DiskWriter(this, sharedResourcesPtr_);
  diskWriter_->startWorkLoop();

}

StorageManager::~StorageManager()
{
  delete ah_;
  delete fragmentProcessor_;
  delete diskWriter_;
}

xoap::MessageReference
StorageManager::ParameterGet(xoap::MessageReference message)
  throw (xoap::exception::Exception)
{
  connectedRBs_.value_ = smrbsenders_.size();
  return Application::ParameterGet(message);
}


////////// *** I2O frame call back functions /////////////////////////////////////////////
void StorageManager::receiveRegistryMessage(toolbox::mem::Reference *ref)
{
  // get the memory pool pointer for statistics if not already set
  if(pool_is_set_ == 0)
  {
    pool_ = ref->getBuffer()->getPool();
    pool_is_set_ = 1;
  }

  FragmentMonitorCollection& fragMonCollection = sharedResourcesPtr_->_statisticsReporter->getFragmentMonitorCollection();

  I2O_MESSAGE_FRAME         *stdMsg  = (I2O_MESSAGE_FRAME*) ref->getDataLocation();
  I2O_SM_PREAMBLE_MESSAGE_FRAME *msg = (I2O_SM_PREAMBLE_MESSAGE_FRAME*) stdMsg;

  FDEBUG(10) << "StorageManager: Received registry message from HLT " << msg->hltURL
             << " application " << msg->hltClassName << " id " << msg->hltLocalId
             << " instance " << msg->hltInstance << " tid " << msg->hltTid
             << " rbBufferID " << msg->rbBufferID << " outModID " << msg->outModID
             << " fuProcID " << msg->fuProcID  << " fuGUID 0x" << std::hex
             << msg->fuGUID << std::dec << std::endl;
  FDEBUG(10) << "StorageManager: registry size " << msg->dataSize << "\n";

  int len = msg->dataSize;

  // *** check the Storage Manager is in the Ready or Enabled state first!
  if( externallyVisibleState() != "Enabled" && externallyVisibleState() != "Ready" )
  {
    LOG4CPLUS_ERROR( this->getApplicationLogger(),
                     "Received INIT message but not in Ready/Enabled state! Current state = "
                     << externallyVisibleState() << " INIT from " << msg->hltURL
                     << " application " << msg->hltClassName );
    // just release the memory at least - is that what we want to do?
    ref->release();
    return;
  }

  // add this output module to the monitoring
  bool alreadyStoredOutMod = false;
  uint32 moduleId = msg->outModID;
  std::string dmoduleLabel("ID_" + boost::lexical_cast<std::string>(msg->outModID));
  if(modId2ModOutMap_.find(moduleId) != modId2ModOutMap_.end()) alreadyStoredOutMod = true;
  if(!alreadyStoredOutMod) {
    modId2ModOutMap_.insert(std::make_pair(moduleId,dmoduleLabel));
    receivedEventsMap_.insert(std::make_pair(dmoduleLabel,0));
    avEventSizeMap_.insert(std::make_pair(dmoduleLabel,
      boost::shared_ptr<ForeverAverageCounter>(new ForeverAverageCounter()) ));
    avCompressRatioMap_.insert(std::make_pair(dmoduleLabel,
      boost::shared_ptr<ForeverAverageCounter>(new ForeverAverageCounter()) ));
  }

  I2OChain i2oChain(ref);
  sharedResourcesPtr_->_fragmentQueue->enq_wait(i2oChain);

  // for bandwidth performance measurements
  unsigned long actualFrameSize =
    (unsigned long)sizeof(I2O_SM_PREAMBLE_MESSAGE_FRAME) + len;
  fragMonCollection.addEventFragmentSample(actualFrameSize);
}

void StorageManager::receiveDataMessage(toolbox::mem::Reference *ref)
{
  // get the memory pool pointer for statistics if not already set
  if(pool_is_set_ == 0)
  {
    pool_ = ref->getBuffer()->getPool();
    pool_is_set_ = 1;
  }

  RunMonitorCollection& runMonCollection = sharedResourcesPtr_->_statisticsReporter->getRunMonitorCollection();
  FragmentMonitorCollection& fragMonCollection = sharedResourcesPtr_->_statisticsReporter->getFragmentMonitorCollection();

  I2O_MESSAGE_FRAME         *stdMsg =
    (I2O_MESSAGE_FRAME*)ref->getDataLocation();
  I2O_SM_DATA_MESSAGE_FRAME *msg    =
    (I2O_SM_DATA_MESSAGE_FRAME*)stdMsg;
  FDEBUG(10)   << "StorageManager: Received data message from HLT at " << msg->hltURL 
	       << " application " << msg->hltClassName << " id " << msg->hltLocalId
	       << " instance " << msg->hltInstance << " tid " << msg->hltTid
	       << " rbBufferID " << msg->rbBufferID << " outModID " << msg->outModID
               << " fuProcID " << msg->fuProcID  << " fuGUID 0x" << std::hex
               << msg->fuGUID << std::dec << std::endl;
  FDEBUG(10)   << "                 for run " << msg->runID << " event " << msg->eventID
	       << " total frames = " << msg->numFrames << std::endl;
  FDEBUG(10)   << "StorageManager: Frame " << msg->frameCount << " of " 
	       << msg->numFrames-1 << std::endl;
  
  int len = msg->dataSize;

  // check the storage Manager is in the Ready state first!
  if( externallyVisibleState() != "Enabled")
  {
    LOG4CPLUS_ERROR(this->getApplicationLogger(),
                       "Received EVENT message but not in Enabled state! Current state = "
                       << externallyVisibleState() << " EVENT from" << msg->hltURL
                       << " application " << msg->hltClassName);
    // just release the memory at least - is that what we want to do?
    ref->release();
    return;
  }

  // should only do this test if the first data frame from each FU?
  // check if run number is the same as that in Run configuration,
  // complain otherwise !!!
  //
  // "The run number check should move somewhere else once we know the
  // right place to put it" (Kurt).
  if(msg->runID != getRunNumber())
    {
      LOG4CPLUS_ERROR(this->getApplicationLogger(),"Run Number from event stream = "
		      << msg->runID << " From " << msg->hltURL
                      << " Different from Run Number from configuration = " << getRunNumber());
    }

  // for data sender list update
  // msg->frameCount start from 0, but in EventMsg header it starts from 1!
  bool isLocal = false;
  int status = 
    smrbsenders_.updateSender4data(&msg->hltURL[0], &msg->hltClassName[0],
                                   msg->hltLocalId, msg->hltInstance, msg->hltTid,
                                   msg->runID, msg->eventID, msg->frameCount+1, msg->numFrames,
                                   msg->originalSize, isLocal, msg->outModID);

  if(status == 1) {
    runMonCollection.getRunNumbersSeenMQ().addSample(msg->runID);
    runMonCollection.getEventIDsReceivedMQ().addSample(msg->eventID);

    uint32 moduleId = msg->outModID;
    if (modId2ModOutMap_.find(moduleId) != modId2ModOutMap_.end()) {
      std::string moduleLabel = modId2ModOutMap_[moduleId];
      ++(receivedEventsMap_[moduleLabel]);
      avEventSizeMap_[moduleLabel]->addSample((double)msg->originalSize);
      // TODO: get the uncompressed size to find compression ratio for stats
    }
    else {
      LOG4CPLUS_WARN(this->getApplicationLogger(),
                     "StorageManager::receiveDataMessage: "
                     << "Unable to find output module label when "
                     << "accumulating statistics for event "
                     << msg->eventID << ", output module "
                     << msg->outModID << ".");
    }
  }
  if(status == -1) {
    LOG4CPLUS_ERROR(this->getApplicationLogger(),
                    "updateSender4data: Cannot find RB in Data Sender list!"
                    << " With URL "
                    << msg->hltURL << " class " << msg->hltClassName  << " instance "
                    << msg->hltInstance << " Tid " << msg->hltTid);
  }

  I2OChain i2oChain(ref);
  sharedResourcesPtr_->_fragmentQueue->enq_wait(i2oChain);

  // for bandwidth performance measurements
  unsigned long actualFrameSize =
    (unsigned long)sizeof(I2O_SM_DATA_MESSAGE_FRAME) + len;
  fragMonCollection.addEventFragmentSample(actualFrameSize);
}

void StorageManager::receiveErrorDataMessage(toolbox::mem::Reference *ref)
{
  // get the memory pool pointer for statistics if not already set
  if(pool_is_set_ == 0)
  {
    pool_ = ref->getBuffer()->getPool();
    pool_is_set_ = 1;
  }

  RunMonitorCollection& runMonCollection = sharedResourcesPtr_->_statisticsReporter->getRunMonitorCollection();
  FragmentMonitorCollection& fragMonCollection = sharedResourcesPtr_->_statisticsReporter->getFragmentMonitorCollection();

  I2O_MESSAGE_FRAME         *stdMsg =
    (I2O_MESSAGE_FRAME*)ref->getDataLocation();
  I2O_SM_DATA_MESSAGE_FRAME *msg    =
    (I2O_SM_DATA_MESSAGE_FRAME*)stdMsg;
  FDEBUG(10)   << "StorageManager: Received error data message from HLT at " << msg->hltURL 
	       << " application " << msg->hltClassName << " id " << msg->hltLocalId
	       << " instance " << msg->hltInstance << " tid " << msg->hltTid
               << " rbBufferID " << msg->rbBufferID << " outModID " << msg->outModID
               << " fuProcID " << msg->fuProcID  << " fuGUID 0x" << std::hex
               << msg->fuGUID << std::dec << std::endl;
  FDEBUG(10)   << "                 for run " << msg->runID << " event " << msg->eventID
	       << " total frames = " << msg->numFrames << std::endl;
  FDEBUG(10)   << "StorageManager: Frame " << msg->frameCount << " of " 
	       << msg->numFrames-1 << std::endl;
  
  int len = msg->dataSize;

  // check the storage Manager is in the Ready state first!
  if(externallyVisibleState() != "Enabled")
  {
    LOG4CPLUS_ERROR(this->getApplicationLogger(),
                       "Received ERROR message but not in Enabled state! Current state = "
                       << externallyVisibleState() << " ERROR from" << msg->hltURL
                       << " application " << msg->hltClassName);
    // just release the memory at least - is that what we want to do?
    ref->release();
    return;
  }

  // all access to the I2O message must happen before sending the fragment
  // to the fragment queue to avoid a race condition in which the buffer
  // is release before we finish here
  runMonCollection.getRunNumbersSeenMQ().addSample(msg->runID);
  runMonCollection.getErrorEventIDsReceivedMQ().addSample(msg->eventID);

  I2OChain i2oChain(ref);
  sharedResourcesPtr_->_fragmentQueue->enq_wait(i2oChain);

  // for bandwidth performance measurements
  unsigned long actualFrameSize =
    (unsigned long)sizeof(I2O_SM_DATA_MESSAGE_FRAME) + len;
  fragMonCollection.addEventFragmentSample(actualFrameSize);
}

void StorageManager::receiveDQMMessage(toolbox::mem::Reference *ref)
{
  // get the memory pool pointer for statistics if not already set
  if(pool_is_set_ == 0)
  {
    pool_ = ref->getBuffer()->getPool();
    pool_is_set_ = 1;
  }

  FragmentMonitorCollection& fragMonCollection = sharedResourcesPtr_->_statisticsReporter->getFragmentMonitorCollection();

  I2O_MESSAGE_FRAME         *stdMsg =
    (I2O_MESSAGE_FRAME*)ref->getDataLocation();
  I2O_SM_DQM_MESSAGE_FRAME *msg    =
    (I2O_SM_DQM_MESSAGE_FRAME*)stdMsg;
  FDEBUG(10) << "StorageManager: Received DQM message from HLT at " << msg->hltURL 
             << " application " << msg->hltClassName << " id " << msg->hltLocalId
             << " instance " << msg->hltInstance << " tid " << msg->hltTid
             << " rbBufferID " << msg->rbBufferID << " folderID " << msg->folderID
             << " fuProcID " << msg->fuProcID  << " fuGUID 0x" << std::hex
            << msg->fuGUID << std::dec << std::endl;
  FDEBUG(10) << "                 for run " << msg->runID << " eventATUpdate = " << msg->eventAtUpdateID
             << " total frames = " << msg->numFrames << std::endl;
  FDEBUG(10) << "StorageManager: Frame " << msg->frameCount << " of " 
             << msg->numFrames-1 << std::endl;
  int len = msg->dataSize;
  FDEBUG(10) << "StorageManager: received DQM frame size = " << len << std::endl;

  // check the storage Manager is in the Ready state first!
  if(externallyVisibleState() != "Enabled")
  {
    LOG4CPLUS_ERROR(this->getApplicationLogger(),
                       "Received DQM message but not in Enabled state! Current state = "
                       << externallyVisibleState() << " DQMMessage from" << msg->hltURL
                       << " application " << msg->hltClassName);
    // just release the memory at least - is that what we want to do?
    ref->release();
    return;
  }

  I2OChain i2oChain(ref);
  sharedResourcesPtr_->_fragmentQueue->enq_wait(i2oChain);

  // for bandwidth performance measurements
  unsigned long actualFrameSize =
    (unsigned long)sizeof(I2O_SM_DQM_MESSAGE_FRAME) + len;
  fragMonCollection.addDQMEventFragmentSample(actualFrameSize);
}

//////////// *** Default web page ///////////////////////////////////////////////
void StorageManager::defaultWebPage(xgi::Input *in, xgi::Output *out)
throw (xgi::exception::Exception)
{
  std::string errorMsg = "Failed to create the default webpage";
  
  try
  {
    _webPageHelper.defaultWebPage(
      out,
      sharedResourcesPtr_,
      pool_
    );
  }
  catch(std::exception &e)
  {
    errorMsg += ": ";
    errorMsg += e.what();
    
    LOG4CPLUS_ERROR(getApplicationLogger(), errorMsg);
    XCEPT_RAISE(xgi::exception::Exception, errorMsg);
  }
  catch(...)
  {
    errorMsg += ": Unknown exception";
    
    LOG4CPLUS_ERROR(getApplicationLogger(), errorMsg);
    XCEPT_RAISE(xgi::exception::Exception, errorMsg);
  }
}


//////////// *** Store data web page //////////////////////////////////////////////////////////
void StorageManager::storedDataWebPage(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception)
{
  std::string errorMsg = "Failed to create the stored data webpage";

  try
  {
    _webPageHelper.storedDataWebPage(
      out,
      sharedResourcesPtr_->_statisticsReporter
    );
  }
  catch(std::exception &e)
  {
    errorMsg += ": ";
    errorMsg += e.what();
    
    LOG4CPLUS_ERROR(getApplicationLogger(), errorMsg);
    XCEPT_RAISE(xgi::exception::Exception, errorMsg);
  }
  catch(...)
  {
    errorMsg += ": Unknown exception";
    
    LOG4CPLUS_ERROR(getApplicationLogger(), errorMsg);
    XCEPT_RAISE(xgi::exception::Exception, errorMsg);
  }


}


void StorageManager::oldDefaultWebPage(xgi::Input *in, xgi::Output *out)
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
  *out << "</head><body>"                                            << endl;
    *out << "<table border=\"0\" width=\"100%\">"                      << endl;
    *out << "<tr>"                                                     << endl;
    *out << "  <td align=\"left\">"                                    << endl;
    *out << "    <img"                                                 << endl;
    *out << "     align=\"middle\""                                    << endl;
    *out << "     src=\"/evf/images/smicon.jpg\""		       << endl;
    *out << "     alt=\"main\""                                        << endl;
    *out << "     width=\"64\""                                        << endl;
    *out << "     height=\"64\""                                       << endl;
    *out << "     border=\"\"/>"                                       << endl;
    *out << "    <b>"                                                  << endl;
    *out << getApplicationDescriptor()->getClassName() << " instance "
         << getApplicationDescriptor()->getInstance()                  << endl;
    *out << "      " << externallyVisibleState()                   << endl;
    *out << "    </b>"                                                 << endl;
    *out << "  </td>"                                                  << endl;
    *out << "  <td width=\"32\">"                                      << endl;
    *out << "    <a href=\"/urn:xdaq-application:lid=3\">"             << endl;
    *out << "      <img"                                               << endl;
    *out << "       align=\"middle\""                                  << endl;
    *out << "       src=\"/hyperdaq/images/HyperDAQ.jpg\""    << endl;
    *out << "       alt=\"HyperDAQ\""                                  << endl;
    *out << "       width=\"32\""                                      << endl;
    *out << "       height=\"32\""                                      << endl;
    *out << "       border=\"\"/>"                                     << endl;
    *out << "    </a>"                                                 << endl;
    *out << "  </td>"                                                  << endl;
    *out << "  <td width=\"32\">"                                      << endl;
    *out << "  </td>"                                                  << endl;
    *out << "</tr>"                                                    << endl;
    if( externallyVisibleState() == "Failed")
    {
      *out << "<tr>"					     << endl;
      *out << " <td>"					     << endl;
      *out << "<textarea rows=" << 5 << " cols=60 scroll=yes";
      *out << " readonly title=\"Reason For Failed\">"		     << endl;
      *out << reasonForFailedState_                                  << endl;
      *out << "</textarea>"                                          << endl;
      *out << " </td>"					     << endl;
      *out << "</tr>"					     << endl;
    }
    *out << "</table>"                                                 << endl;

  *out << "<table frame=\"void\" rules=\"groups\" class=\"states\""	 << endl;
  *out << " readonly title=\"Note: parts of this info updates every 10 sec !!!\">"<< endl;
  *out << "<colgroup> <colgroup align=\"right\">"			 << endl;
    *out << "  <tr>"						 	 << endl;
    *out << "    <th colspan=7>"					 << endl;
    *out << "      " << "Storage Manager Statistics"			 << endl;
    *out << "    </th>"							 << endl;
    *out << "  </tr>"							 << endl;
        *out << "<tr class=\"special\">" << endl;
          *out << "<td >" << endl;
          *out << "Output Module" << endl;
          *out << "</td>" << endl;
          *out << "<td align=center>" << endl;
          *out << "Events" << endl;
          *out << "</td>" << endl;
          *out << "<td align=center>" << endl;
          *out << "Size (MB)" << endl;
          *out << "</td>" << endl;
          *out << "<td align=center>" << endl;
          *out << "Size/Evt (KB)" << endl;
          *out << "</td>" << endl;
          *out << "<td align=center>" << endl;
          *out << "RMS (KB)" << endl;
          *out << "</td>" << endl;
          *out << "<td align=center>" << endl;
          *out << "Min (KB)" << endl;
          *out << "</td>" << endl;
          *out << "<td align=center>" << endl;
          *out << "Max (KB)" << endl;
          *out << "</td>" << endl;
        *out << "</tr>" << endl;
        boost::shared_ptr<InitMsgCollection> initMsgCollection =
          sharedResourcesPtr_->_initMsgCollection;
        idMap_iter oi(modId2ModOutMap_.begin()), oe(modId2ModOutMap_.end());
        for( ; oi != oe; ++oi) {
          std::string outputModuleLabel = oi->second;
          if (initMsgCollection.get() != NULL &&
              initMsgCollection->getOutputModuleName(oi->first) != "") {
            outputModuleLabel = initMsgCollection->getOutputModuleName(oi->first);
          }
          *out << "<tr>" << endl;
            *out << "<td >" << endl;
            *out << outputModuleLabel << endl;
            *out << "</td>" << endl;
            *out << "<td align=right>" << endl;
            //*out << receivedEventsMap_[oi->second] << endl;
            *out << receivedEventsMap_[oi->second] << " (" << avEventSizeMap_[oi->second]->getSampleCount() << ") "<< endl;
            *out << "</td>" << endl;
            *out << "<td align=right>" << endl;
            *out << avEventSizeMap_[oi->second]->getValueSum()/(double)0x100000 << endl;
            *out << "</td>" << endl;
            *out << "<td align=right>" << endl;
            *out << avEventSizeMap_[oi->second]->getValueAverage()/(double)0x400 << endl;
            *out << "</td>" << endl;
            *out << "<td align=right>" << endl;
            *out << avEventSizeMap_[oi->second]->getValueRMS()/(double)0x400 << endl;
            *out << "</td>" << endl;
            *out << "<td align=right>" << endl;
            *out << avEventSizeMap_[oi->second]->getValueMin()/(double)0x400 << endl;
            *out << "</td>" << endl;
            *out << "<td align=right>" << endl;
            *out << avEventSizeMap_[oi->second]->getValueMax()/(double)0x400 << endl;
            *out << "</td>" << endl;
          *out << "</tr>" << endl;
        }
    *out << "</table>" << endl;

// now for RB sender list statistics
  *out << "<hr/>"                                                    << endl;
  *out << "<table>"                                                  << endl;
  *out << "<tr valign=\"top\">"                                      << endl;
  *out << "  <td>"                                                   << endl;

  *out << "<table frame=\"void\" rules=\"groups\" class=\"states\">" << endl;
  *out << "<colgroup> <colgroup align=\"rigth\">"                    << endl;
    *out << "  <tr>"                                                   << endl;
    *out << "    <th colspan=2>"                                       << endl;
    *out << "      " << "RB Sender Information"                            << endl;
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
          *out << "Number of RB Senders" << endl;
          *out << "</td>" << endl;
          *out << "<td>" << endl;
          *out << smrbsenders_.numberOfRB() << endl;
          *out << "</td>" << endl;
        *out << "  </tr>" << endl;
        *out << "<tr>" << endl;
          *out << "<td >" << endl;
          *out << "Number of OM per FU" << endl;
          *out << "</td>" << endl;
          *out << "<td>" << endl;
          *out << smrbsenders_.numberOfOM() << endl;
          *out << "</td>" << endl;
        *out << "  </tr>" << endl;
        *out << "<tr>" << endl;
          *out << "<td >" << endl;
          *out << "Sanity check number in sender list" << endl;
          *out << "</td>" << endl;
          *out << "<td>" << endl;
          *out << smrbsenders_.size() << endl;
          *out << "</td>" << endl;
        *out << "  </tr>" << endl;

  *out << "</table>" << endl;

  DiskWritingParams dwParams =
    sharedResourcesPtr_->_configuration->getDiskWritingParams();

  *out << "<table frame=\"void\" rules=\"groups\" class=\"states\">" << endl;
  *out << "<colgroup> <colgroup align=\"rigth\">"                    << endl;
    *out << "  <tr>"                                                   << endl;
    *out << "    <th colspan=2>"                                       << endl;
    *out << "      " << "SM Configuration Information "                << endl;
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
    *out << "<tr class=\"special\">" << endl;
      *out << "<td colspan=2>" << endl;
      *out << "SM cfg string" << endl;
      *out << "</td>" << endl;
    *out << "  </tr>" << endl;
    *out << "<tr>"					     << endl;
      *out << " <td colspan=2>"				     << endl;
      *out << "<textarea rows=" << 10 << " cols=100 scroll=yes";
      *out << " readonly title=\"SM config\">"		     << endl;
      *out << dwParams._streamConfiguration                  << endl;
      *out << "</textarea>"                                  << endl;
      *out << " </td>"					     << endl;
    *out << "</tr>"					     << endl;

  *out << "</table>" << endl;

  *out << "  </td>"                                                  << endl;
  *out << "</table>"                                                 << endl;
  //---- separate pages for RB senders and Streamer Output
  *out << "<hr/>"                                                 << endl;
  std::string url = getApplicationDescriptor()->getContextDescriptor()->getURL();
  std::string urn = getApplicationDescriptor()->getURN();
  *out << "<a href=\"" << url << "/" << urn << "\">" 
       << "New default web page" << "</a>" << endl;
  *out << "<hr/>"                                                 << endl;
  *out << "<a href=\"" << url << "/" << urn << "/rbsenderlist" << "\">" 
       << "RB Sender list web page" << "</a>" << endl;
  *out << "<hr/>"                                                 << endl;
  *out << "<a href=\"" << url << "/" << urn << "/streameroutput" << "\">" 
       << "Streamer Output Status web page" << "</a>" << endl;
  *out << "<hr/>"                                                 << endl;
  *out << "<a href=\"" << url << "/" << urn << "/EventServerStats?update=off"
       << "\">Event Server Statistics" << "</a>" << endl;
  /* --- leave these here to debug event server problems
  *out << "<a href=\"" << url << "/" << urn << "/geteventdata" << "\">" 
       << "Get an event via a web page" << "</a>" << endl;
  *out << "<hr/>"                                                 << endl;
  *out << "<a href=\"" << url << "/" << urn << "/getregdata" << "\">" 
       << "Get a header via a web page" << "</a>" << endl;
  */

  *out << "</body>"                                                  << endl;
  *out << "</html>"                                                  << endl;
}


//////////// *** rbsender web page //////////////////////////////////////////////////////////
void StorageManager::rbsenderWebPage(xgi::Input *in, xgi::Output *out)
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
  *out << "</head><body>"                                            << endl;
    *out << "<table border=\"0\" width=\"100%\">"                      << endl;
    *out << "<tr>"                                                     << endl;
    *out << "  <td align=\"left\">"                                    << endl;
    *out << "    <img"                                                 << endl;
    *out << "     align=\"middle\""                                    << endl;
    *out << "     src=\"/rubuilder/fu/images/fu64x64.gif\""     << endl;
    *out << "     alt=\"main\""                                        << endl;
    *out << "     width=\"64\""                                        << endl;
    *out << "     height=\"64\""                                       << endl;
    *out << "     border=\"\"/>"                                       << endl;
    *out << "    <b>"                                                  << endl;
    *out << getApplicationDescriptor()->getClassName() << " instance "
         << getApplicationDescriptor()->getInstance()                  << endl;
    *out << "      " << externallyVisibleState()                   << endl;
    *out << "    </b>"                                                 << endl;
    *out << "  </td>"                                                  << endl;
    *out << "  <td width=\"32\">"                                      << endl;
    *out << "    <a href=\"/urn:xdaq-application:lid=3\">"             << endl;
    *out << "      <img"                                               << endl;
    *out << "       align=\"middle\""                                  << endl;
    *out << "       src=\"/hyperdaq/images/HyperDAQ.jpg\""    << endl;
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
    *out << "       src=\"/rubuilder/fu/images/debug32x32.gif\""         << endl;
    *out << "       alt=\"debug\""                                     << endl;
    *out << "       width=\"32\""                                      << endl;
    *out << "       height=\"32\""                                     << endl;
    *out << "       border=\"\"/>"                                     << endl;
    *out << "    </a>"                                                 << endl;
    *out << "  </td>"                                                  << endl;
    *out << "</tr>"                                                    << endl;
    if( externallyVisibleState() == "Failed")
    {
      *out << "<tr>"					     << endl;
      *out << " <td>"					     << endl;
      *out << "<textarea rows=" << 5 << " cols=60 scroll=yes";
      *out << " readonly title=\"Reason For Failed\">"		     << endl;
      *out << reasonForFailedState_                                  << endl;
      *out << "</textarea>"                                          << endl;
      *out << " </td>"					     << endl;
      *out << "</tr>"					     << endl;
    }
    *out << "</table>"                                                 << endl;

  *out << "<hr/>"                                                    << endl;

// now for RB sender list statistics
  *out << "<table>"                                                  << endl;
  *out << "<tr valign=\"top\">"                                      << endl;
  *out << "  <td>"                                                   << endl;

  *out << "<table frame=\"void\" rules=\"groups\" class=\"states\">" << endl;
  *out << "<colgroup> <colgroup align=\"rigth\">"                    << endl;
    *out << "  <tr>"                                                   << endl;
    *out << "    <th colspan=2>"                                       << endl;
    *out << "      " << "RBxFUxOM Sender List"                            << endl;
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
          *out << "Number of RB Senders" << endl;
          *out << "</td>" << endl;
          *out << "<td>" << endl;
          *out << smrbsenders_.numberOfRB() << endl;
          *out << "</td>" << endl;
        *out << "  </tr>" << endl;
        *out << "<tr>" << endl;
          *out << "<td >" << endl;
          *out << "Number of OM per FU" << endl;
          *out << "</td>" << endl;
          *out << "<td>" << endl;
          *out << smrbsenders_.numberOfOM() << endl;
          *out << "</td>" << endl;
        *out << "  </tr>" << endl;
        *out << "<tr>" << endl;
          *out << "<td >" << endl;
          *out << "Number in list of senders" << endl;
          *out << "</td>" << endl;
          *out << "<td>" << endl;
          *out << smrbsenders_.size() << endl;
          *out << "</td>" << endl;
        *out << "  </tr>" << endl;
    std::vector<boost::shared_ptr<SMFUSenderStats> > vrbstats = smrbsenders_.getSenderStats();
    if(!vrbstats.empty()) {
      for(vector<boost::shared_ptr<SMFUSenderStats> >::iterator pos = vrbstats.begin();
          pos != vrbstats.end(); ++pos)
      {
        *out << "<tr>" << endl;
          *out << "<td >" << endl;
          *out << "RB Sender URL" << endl;
          *out << "</td>" << endl;
          *out << "<td align=right>" << endl;
          char hlturl[MAX_I2O_SM_URLCHARS];
          copy(&(((*pos)->hltURL_)->at(0)), 
               &(((*pos)->hltURL_)->at(0)) + ((*pos)->hltURL_)->size(),
               hlturl);
          hlturl[((*pos)->hltURL_)->size()] = '\0';
          *out << hlturl << endl;
          *out << "</td>" << endl;
        *out << "  </tr>" << endl;
        *out << "<tr>" << endl;
          *out << "<td >" << endl;
          *out << "RB Sender Class Name" << endl;
          *out << "</td>" << endl;
          *out << "<td align=right>" << endl;
          char hltclass[MAX_I2O_SM_URLCHARS];
          copy(&(((*pos)->hltClassName_)->at(0)), 
               &(((*pos)->hltClassName_)->at(0)) + ((*pos)->hltClassName_)->size(),
               hltclass);
          hltclass[((*pos)->hltClassName_)->size()] = '\0';
          *out << hltclass << endl;
          *out << "</td>" << endl;
        *out << "  </tr>" << endl;
        *out << "<tr>" << endl;
          *out << "<td >" << endl;
          *out << "RB Sender Instance" << endl;
          *out << "</td>" << endl;
          *out << "<td align=right>" << endl;
          *out << (*pos)->hltInstance_ << endl;
          *out << "</td>" << endl;
        *out << "  </tr>" << endl;
        *out << "<tr>" << endl;
          *out << "<td >" << endl;
          *out << "RB Sender Local ID" << endl;
          *out << "</td>" << endl;
          *out << "<td align=right>" << endl;
          *out << (*pos)->hltLocalId_ << endl;
          *out << "</td>" << endl;
        *out << "  </tr>" << endl;
        *out << "<tr>" << endl;
          *out << "<td >" << endl;
          *out << "RB Sender Tid" << endl;
          *out << "</td>" << endl;
          *out << "<td align=right>" << endl;
          *out << (*pos)->hltTid_ << endl;
          *out << "</td>" << endl;
        *out << "  </tr>" << endl;
        *out << "<tr>" << endl;
          *out << "<td >" << endl;
          *out << "FU Sender process id" << endl;
          *out << "</td>" << endl;
          *out << "<td align=right>" << endl;
          *out << (*pos)->fuProcId_ << endl;
          *out << "</td>" << endl;
        *out << "  </tr>" << endl;
        *out << "<tr>" << endl;
          *out << "<td >" << endl;
          *out << "Number of registries received (output modules)" << endl;
          *out << "</td>" << endl;
          *out << "<td align=right>" << endl;
          *out << (*pos)->registryCollection_.outModName_.size() << endl;
          *out << "</td>" << endl;
        *out << "  </tr>" << endl;
        *out << "<tr><td bgcolor=\"#999933\" height=\"1\" colspan=\"2\"></td></tr>" << endl;
        // Loop over number of registries
        if(!(*pos)->registryCollection_.outModName_.empty()) {
          for(vector<std::string>::iterator idx = (*pos)->registryCollection_.outModName_.begin();
              idx != (*pos)->registryCollection_.outModName_.end(); ++idx)
          {
            *out << "<tr>" << endl;
              *out << "<td >" << endl;
              *out << "Output Module Name" << endl;
              *out << "</td>" << endl;
              *out << "<td align=right>" << endl;
              *out << (*idx) << endl;
              *out << "</td>" << endl;
            *out << "  </tr>" << endl;
            *out << "<tr>" << endl;
              *out << "<td >" << endl;
              *out << "Output Module Id" << endl;
              *out << "</td>" << endl;
              *out << "<td align=right>" << endl;
              *out << (*pos)->registryCollection_.outModName2ModId_[*idx] << endl;
              *out << "</td>" << endl;
            *out << "  </tr>" << endl;
            *out << "<tr>" << endl;
              *out << "<td >" << endl;
              *out << "Product registry size (bytes)" << endl;
              *out << "</td>" << endl;
              *out << "<td align=right>" << endl;
              *out << (*pos)->registryCollection_.registrySizeMap_[*idx] << endl;
              *out << "</td>" << endl;
            *out << "  </tr>" << endl;
            *out << "<tr><td bgcolor=\"#999933\" height=\"1\" colspan=\"2\"></td></tr>" << endl;
          }
        }
        *out << "<tr>" << endl;
          *out << "<td>" << endl;
          *out << "Connection Status" << endl;
          *out << "</td>" << endl;
          *out << "<td align=right>" << endl;
          *out << (*pos)->connectStatus_ << endl;
          *out << "</td>" << endl;
        *out << "  </tr>" << endl;
        if((*pos)->connectStatus_ > 1) {
          *out << "<tr>" << endl;
            *out << "<td >" << endl;
            *out << "Time since last data frame (ms)" << endl;
            *out << "</td>" << endl;
            *out << "<td align=right>" << endl;
            *out << (*pos)->timeWaited_ << endl;
            *out << "</td>" << endl;
          *out << "  </tr>" << endl;
          *out << "<tr>" << endl;
            *out << "<td >" << endl;
            *out << "Run number" << endl;
            *out << "</td>" << endl;
            *out << "<td align=right>" << endl;
            *out << (*pos)->runNumber_ << endl;
            *out << "</td>" << endl;
          *out << "  </tr>" << endl;
          *out << "<tr>" << endl;
            *out << "<td >" << endl;
            *out << "Running locally" << endl;
            *out << "</td>" << endl;
            *out << "<td align=right>" << endl;
            if((*pos)->isLocal_) {
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
            *out << (*pos)->framesReceived_ << endl;
            *out << "</td>" << endl;
          *out << "  </tr>" << endl;
          *out << "<tr>" << endl;
            *out << "<td >" << endl;
            *out << "Events received" << endl;
            *out << "</td>" << endl;
            *out << "<td align=right>" << endl;
            *out << (*pos)->eventsReceived_ << endl;
            *out << "</td>" << endl;
          *out << "  </tr>" << endl;
          *out << "<tr>" << endl;
            *out << "<td >" << endl;
            *out << "Total Bytes received" << endl;
            *out << "</td>" << endl;
            *out << "<td align=right>" << endl;
            *out << (*pos)->totalSizeReceived_ << endl;
            *out << "</td>" << endl;
          *out << "  </tr>" << endl;
          *out << "<tr><td bgcolor=\"#999933\" height=\"1\" colspan=\"2\"></td></tr>" << endl;
          // Loop over number of output modules
          if(!(*pos)->registryCollection_.outModName_.empty()) {
            for(vector<std::string>::iterator idx = (*pos)->registryCollection_.outModName_.begin();
                idx != (*pos)->registryCollection_.outModName_.end(); ++idx)
            {
              *out << "<tr>" << endl;
                *out << "<td >" << endl;
                *out << "Output Module Name" << endl;
                *out << "</td>" << endl;
                *out << "<td align=right>" << endl;
                *out << (*idx) << endl;
                *out << "</td>" << endl;
              *out << "  </tr>" << endl;
              *out << "<tr>" << endl;
                *out << "<td >" << endl;
                *out << "Frames received" << endl;
                *out << "</td>" << endl;
                *out << "<td align=right>" << endl;
                *out << (*pos)->datCollection_.framesReceivedMap_[*idx] << endl;
                *out << "</td>" << endl;
              *out << "  </tr>" << endl;
              *out << "<tr>" << endl;
                *out << "<td >" << endl;
                *out << "Events received" << endl;
                *out << "</td>" << endl;
                *out << "<td align=right>" << endl;
                *out << (*pos)->datCollection_.eventsReceivedMap_[*idx] << endl;
                *out << "</td>" << endl;
              *out << "  </tr>" << endl;
              *out << "<tr>" << endl;
                *out << "<td >" << endl;
                *out << "Total Bytes received" << endl;
                *out << "</td>" << endl;
                *out << "<td align=right>" << endl;
                *out << (*pos)->datCollection_.totalSizeReceivedMap_[*idx] << endl;
                *out << "</td>" << endl;
              *out << "  </tr>" << endl;
              *out << "<tr><td bgcolor=\"#999933\" height=\"1\" colspan=\"2\"></td></tr>" << endl;
            }
          }
          if((*pos)->eventsReceived_ > 0) {
            *out << "<tr>" << endl;
              *out << "<td >" << endl;
              *out << "Last frame latency (us)" << endl;
              *out << "</td>" << endl;
              *out << "<td align=right>" << endl;
              *out << (*pos)->lastLatency_ << endl;
              *out << "</td>" << endl;
            *out << "  </tr>" << endl;
            *out << "<tr>" << endl;
              *out << "<td >" << endl;
              *out << "Average event size (Bytes)" << endl;
              *out << "</td>" << endl;
              *out << "<td align=right>" << endl;
              *out << (*pos)->totalSizeReceived_/(*pos)->eventsReceived_ << endl;
              *out << "</td>" << endl;
              *out << "<tr>" << endl;
                *out << "<td >" << endl;
                *out << "Last Run Number" << endl;
                *out << "</td>" << endl;
                *out << "<td align=right>" << endl;
                *out << (*pos)->lastRunID_ << endl;
                *out << "</td>" << endl;
              *out << "  </tr>" << endl;
              *out << "<tr>" << endl;
                *out << "<td >" << endl;
                *out << "Last Event Number" << endl;
                *out << "</td>" << endl;
                *out << "<td align=right>" << endl;
                *out << (*pos)->lastEventID_ << endl;
                *out << "</td>" << endl;
              *out << "  </tr>" << endl;
            } // events received endif
          *out << "  </tr>" << endl;
          *out << "<tr>" << endl;
            *out << "<td >" << endl;
            *out << "Total out of order frames" << endl;
            *out << "</td>" << endl;
            *out << "<td align=right>" << endl;
            *out << (*pos)->totalOutOfOrder_ << endl;
            *out << "</td>" << endl;
          *out << "  </tr>" << endl;
          *out << "<tr>" << endl;
            *out << "<td >" << endl;
            *out << "Total Bad Events" << endl;
            *out << "</td>" << endl;
            *out << "<td align=right>" << endl;
            *out << (*pos)->totalBadEvents_ << endl;
            *out << "</td>" << endl;
          *out << "  </tr>" << endl;
        } // connect status endif
      } // Sender list loop
    } //sender size test endif

  *out << "</table>" << endl;

  *out << "  </td>"                                                  << endl;
  *out << "</table>"                                                 << endl;

  *out << "</body>"                                                  << endl;
  *out << "</html>"                                                  << endl;
}


void StorageManager::fileStatisticsWebPage(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception)
{
  std::string errorMsg = "Failed to create the file statistics webpage";

  try
  {
    _webPageHelper.filesWebPage(
      out,
      sharedResourcesPtr_->_statisticsReporter
    );
  }
  catch(std::exception &e)
  {
    errorMsg += ": ";
    errorMsg += e.what();
    
    LOG4CPLUS_ERROR(getApplicationLogger(), errorMsg);
    XCEPT_RAISE(xgi::exception::Exception, errorMsg);
  }
  catch(...)
  {
    errorMsg += ": Unknown exception";
    
    LOG4CPLUS_ERROR(getApplicationLogger(), errorMsg);
    XCEPT_RAISE(xgi::exception::Exception, errorMsg);
  }

}


//////////// *** get event data web page //////////////////////////////////////////////////////////
void StorageManager::eventdataWebPage(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception)
{
  // default the message length to zero
  int len=0;

  // determine the consumer ID from the event request
  // message, if it is available.
  unsigned int consumerId = 0;
  int consumerInitMsgCount = -1;
  std::string lengthString = in->getenv("CONTENT_LENGTH");
  unsigned long contentLength = std::atol(lengthString.c_str());
  if (contentLength > 0) 
    {
      auto_ptr< vector<char> > bufPtr(new vector<char>(contentLength));
      in->read(&(*bufPtr)[0], contentLength);
      OtherMessageView requestMessage(&(*bufPtr)[0]);
      if (requestMessage.code() == Header::EVENT_REQUEST)
	{
	  uint8 *bodyPtr = requestMessage.msgBody();
	  consumerId = convert32(bodyPtr);
          if (requestMessage.bodySize() >= (2 * sizeof(char_uint32)))
            {
              bodyPtr += sizeof(char_uint32);
              consumerInitMsgCount = convert32(bodyPtr);
            }
	}
    }

  // first test if StorageManager is in Enabled state and registry is filled
  // this must be the case for valid data to be present
  if(externallyVisibleState() == "Enabled" &&
     sharedResourcesPtr_->_initMsgCollection.get() != NULL &&
     sharedResourcesPtr_->_initMsgCollection->size() > 0)
  {
    boost::shared_ptr<EventServer> eventServer =
      sharedResourcesPtr_->_oldEventServer;
    if (eventServer.get() != NULL)
    {
      // if we've stored a "registry warning" in the consumer pipe, send
      // that instead of an event so that the consumer can react to
      // the warning
      boost::shared_ptr<ConsumerPipe> consPtr =
        eventServer->getConsumer(consumerId);
      if (consPtr.get() != NULL && consPtr->hasRegistryWarning())
      {
        std::vector<char> registryWarning = consPtr->getRegistryWarning();
        const char* from = &registryWarning[0];
        unsigned int msize = registryWarning.size();
        if(mybuffer_.capacity() < msize) mybuffer_.resize(msize);
        unsigned char* pos = (unsigned char*) &mybuffer_[0];

        copy(from,from+msize,pos);
        len = msize;
        consPtr->clearRegistryWarning();
      }
      // if the consumer is an instance of the proxy server and
      // it knows about fewer INIT messages than we do, tell it
      // that new INIT message(s) are available
      else if (consPtr.get() != NULL && consPtr->isProxyServer() &&
               consumerInitMsgCount >= 0 &&
               sharedResourcesPtr_->_initMsgCollection->size() > consumerInitMsgCount)
      {
        OtherMessageBuilder othermsg(&mybuffer_[0],
                                     Header::NEW_INIT_AVAILABLE);
        len = othermsg.size();
      }
      // otherwise try to send an event
      else
      {
        boost::shared_ptr< std::vector<char> > bufPtr =
          eventServer->getEvent(consumerId);
        if (bufPtr.get() != NULL)
        {
          EventMsgView msgView(&(*bufPtr)[0]);

          unsigned char* from = msgView.startAddress();
          unsigned int dsize = msgView.size();
          if(mybuffer_.capacity() < dsize) mybuffer_.resize(dsize);
          unsigned char* pos = (unsigned char*) &mybuffer_[0];

          copy(from,from+dsize,pos);
          len = dsize;
          FDEBUG(10) << "sending event " << msgView.event() << std::endl;
        }
      }
    }
    
    out->getHTTPResponseHeader().addHeader("Content-Type", "application/octet-stream");
    out->getHTTPResponseHeader().addHeader("Content-Transfer-Encoding", "binary");
    out->write((char*) &mybuffer_[0],len);
  } // else send end of run as reponse
  else
    {
      OtherMessageBuilder othermsg(&mybuffer_[0],Header::DONE);
      len = othermsg.size();
      //std::cout << "making other message code = " << othermsg.code()
      //          << " and size = " << othermsg.size() << std::endl;
      
      out->getHTTPResponseHeader().addHeader("Content-Type", "application/octet-stream");
      out->getHTTPResponseHeader().addHeader("Content-Transfer-Encoding", "binary");
      out->write((char*) &mybuffer_[0],len);
    }
  
  // How to block if there is no data
  // How to signal if end, and there will be no more data?
  
}


//////////// *** get header (registry) web page ////////////////////////////////////////
void StorageManager::headerdataWebPage(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception)
{
  unsigned int len = 0;

  // determine the consumer ID from the header request
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

  // first test if StorageManager is in Enabled state and registry is filled
  // this must be the case for valid data to be present
  if(externallyVisibleState() == "Enabled" &&
     sharedResourcesPtr_->_initMsgCollection.get() != NULL &&
     sharedResourcesPtr_->_initMsgCollection->size() > 0)
    {
      std::string errorString;
      InitMsgSharedPtr serializedProds;
      boost::shared_ptr<EventServer> eventServer =
        sharedResourcesPtr_->_oldEventServer;
      if (eventServer.get() != NULL)
      {
        boost::shared_ptr<ConsumerPipe> consPtr =
          eventServer->getConsumer(consumerId);
        if (consPtr.get() != NULL)
        {
          // limit this (and other) interaction with the InitMsgCollection
          // to a single thread so that we can present a coherent
          // picture to consumers
          boost::mutex::scoped_lock sl(consumerInitMsgLock_);
          boost::shared_ptr<InitMsgCollection> initMsgCollection =
            sharedResourcesPtr_->_initMsgCollection;
          try
          {
            if (consPtr->isProxyServer())
            {
              // If the INIT message collection has more than one element,
              // we build up a special response message that contains all
              // of the INIT messages in the collection (code = INIT_SET).
              // We can use an InitMsgBuffer to do this (and assign it
              // to the serializedProds local variable) because it
              // is really just a vector of char (it doesn't have any
              // internal structure that limits it to being used just for
              // single INIT messages).
              if (initMsgCollection->size() > 1)
              {
                serializedProds = initMsgCollection->getFullCollection();
              }
              else
              {
                serializedProds = initMsgCollection->getLastElement();
              }
            }
            else
            {
              std::string hltOMLabel = consPtr->getHLTOutputSelection();
              serializedProds =
                initMsgCollection->getElementForOutputModule(hltOMLabel);
            }
            if (serializedProds.get() != NULL)
            {
              uint8* regPtr = &(*serializedProds)[0];
              HeaderView hdrView(regPtr);

              // if the response that we're sending is an INIT_SET rather
              // than a single INIT message, we simply use the first INIT
              // message in the collection to initialize the local 
              // ConsumerPipe.  Since all we need is the
              // full trigger list, any of the INIT messages should be fine
              // (because all of them should have the same full trigger list).
              if (hdrView.code() == Header::INIT_SET) {
                OtherMessageView otherView(&(*serializedProds)[0]);
                regPtr = otherView.msgBody();
              }

              Strings triggerNameList;
              InitMsgView initView(regPtr);
              initView.hltTriggerNames(triggerNameList);

              uint32 outputModuleId;
              if (initView.protocolVersion() >= 6) {
                outputModuleId = initView.outputModuleId();
              }
              else {
                std::string moduleLabel = initView.outputModuleLabel();
                uLong crc = crc32(0L, Z_NULL, 0);
                Bytef* crcbuf = (Bytef*) moduleLabel.data();
                crc = crc32(crc, crcbuf, moduleLabel.length());
                outputModuleId = static_cast<uint32>(crc);
              }
              consPtr->initializeSelection(triggerNameList,
                                           outputModuleId);
            }
          }
          catch (const edm::Exception& excpt)
          {
            errorString = excpt.what();
          }
          catch (const cms::Exception& excpt)
          {
            //errorString.append(excpt.what());
            errorString.append("ERROR: The configuration for this ");
            errorString.append("consumer does not specify an HLT output ");
            errorString.append("module.\nPlease specify one of the HLT ");
            errorString.append("output modules listed below as the ");
            errorString.append("SelectHLTOutput parameter ");
            errorString.append("in the InputSource configuration.\n");
            errorString.append(initMsgCollection->getSelectionHelpString());
            errorString.append("\n");
          }
        }
      }
      if (errorString.length() > 0) {
        len = errorString.length();
      }
      else if (serializedProds.get() != NULL) {
        len = serializedProds->size();
      }
      else {
        len = 0;
      }
      if (mybuffer_.capacity() < len) mybuffer_.resize(len);
      if (errorString.length() > 0) {
        const char *errorBytes = errorString.c_str();
        for (unsigned int i=0; i<len; ++i) mybuffer_[i]=errorBytes[i];
      }
      else if (serializedProds.get() != NULL) {
        for (unsigned int i=0; i<len; ++i) mybuffer_[i]=(*serializedProds)[i];
      }
    }

  out->getHTTPResponseHeader().addHeader("Content-Type", "application/octet-stream");
  out->getHTTPResponseHeader().addHeader("Content-Transfer-Encoding", "binary");
  out->write((char*) &mybuffer_[0],len);
  
  // How to block if there is no header data
  // How to signal if not yet started, so there is no registry yet?
}

void StorageManager::consumerListWebPage(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception)
{
  char buffer[65536];

  out->getHTTPResponseHeader().addHeader("Content-Type", "application/xml");
  sprintf(buffer,
	  "<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>\n<Monitor>\n");
  out->write(buffer,strlen(buffer));

  if(externallyVisibleState() == "Enabled")
  {
    sprintf(buffer, "<ConsumerList>\n");
    out->write(buffer,strlen(buffer));

    boost::shared_ptr<EventServer> eventServer =
      sharedResourcesPtr_->_oldEventServer;
    if (eventServer.get() != NULL)
    {
      std::map< uint32, boost::shared_ptr<ConsumerPipe> > consumerTable = 
	eventServer->getConsumerTable();
      std::map< uint32, boost::shared_ptr<ConsumerPipe> >::const_iterator 
	consumerIter;
      for (consumerIter = consumerTable.begin();
	   consumerIter != consumerTable.end();
	   ++consumerIter)
      {
	boost::shared_ptr<ConsumerPipe> consumerPipe = consumerIter->second;
	sprintf(buffer, "<Consumer>\n");
	out->write(buffer,strlen(buffer));

	if (consumerPipe->isProxyServer()) {
	  sprintf(buffer, "<Name>Proxy Server</Name>\n");
	}
	else {
	  sprintf(buffer, "<Name>%s</Name>\n",
	          consumerPipe->getConsumerName().c_str());
	}
	out->write(buffer,strlen(buffer));

	sprintf(buffer, "<ID>%d</ID>\n", consumerPipe->getConsumerId());
	out->write(buffer,strlen(buffer));

	sprintf(buffer, "<Time>%d</Time>\n", 
		(int)consumerPipe->getLastEventRequestTime());
	out->write(buffer,strlen(buffer));

	sprintf(buffer, "<Host>%s</Host>\n", 
		consumerPipe->getHostName().c_str());
	out->write(buffer,strlen(buffer));

	sprintf(buffer, "<Events>%d</Events>\n", consumerPipe->getEvents());
	out->write(buffer,strlen(buffer));

	sprintf(buffer, "<Failed>%d</Failed>\n", 
		consumerPipe->getPushEventFailures());
	out->write(buffer,strlen(buffer));

	sprintf(buffer, "<Idle>%d</Idle>\n", consumerPipe->isIdle());
	out->write(buffer,strlen(buffer));

	sprintf(buffer, "<Disconnected>%d</Disconnected>\n", 
		consumerPipe->isDisconnected());
	out->write(buffer,strlen(buffer));

	sprintf(buffer, "<Ready>%d</Ready>\n", consumerPipe->isReadyForEvent());
	out->write(buffer,strlen(buffer));

	sprintf(buffer, "</Consumer>\n");
	out->write(buffer,strlen(buffer));
      }
    }
    boost::shared_ptr<DQMEventServer> dqmServer =
      sharedResourcesPtr_->_oldDQMEventServer;
    if (dqmServer.get() != NULL)
    {
      std::map< uint32, boost::shared_ptr<DQMConsumerPipe> > dqmTable = 
	dqmServer->getConsumerTable();
      std::map< uint32, boost::shared_ptr<DQMConsumerPipe> >::const_iterator 
	dqmIter;
      for (dqmIter = dqmTable.begin();
	   dqmIter != dqmTable.end();
	   ++dqmIter)
      {
	boost::shared_ptr<DQMConsumerPipe> dqmPipe = dqmIter->second;
	sprintf(buffer, "<DQMConsumer>\n");
	out->write(buffer,strlen(buffer));

	if (dqmPipe->isProxyServer()) {
	  sprintf(buffer, "<Name>Proxy Server</Name>\n");
	}
	else {
	  sprintf(buffer, "<Name>%s</Name>\n",
	          dqmPipe->getConsumerName().c_str());
	}
	out->write(buffer,strlen(buffer));

	sprintf(buffer, "<ID>%d</ID>\n", dqmPipe->getConsumerId());
	out->write(buffer,strlen(buffer));

	sprintf(buffer, "<Time>%d</Time>\n", 
		(int)dqmPipe->getLastEventRequestTime());
	out->write(buffer,strlen(buffer));

	sprintf(buffer, "<Host>%s</Host>\n", 
		dqmPipe->getHostName().c_str());
	out->write(buffer,strlen(buffer));

	sprintf(buffer, "<Events>%d</Events>\n", dqmPipe->getEvents());
	out->write(buffer,strlen(buffer));

	sprintf(buffer, "<Failed>%d</Failed>\n", 
		dqmPipe->getPushEventFailures());
	out->write(buffer,strlen(buffer));

	sprintf(buffer, "<Idle>%d</Idle>\n", dqmPipe->isIdle());
	out->write(buffer,strlen(buffer));

	sprintf(buffer, "<Disconnected>%d</Disconnected>\n", 
		dqmPipe->isDisconnected());
	out->write(buffer,strlen(buffer));

	sprintf(buffer, "<Ready>%d</Ready>\n", dqmPipe->isReadyForEvent());
	out->write(buffer,strlen(buffer));

	sprintf(buffer, "</DQMConsumer>\n");
	out->write(buffer,strlen(buffer));
      }
    }
    sprintf(buffer, "</ConsumerList>\n");
    out->write(buffer,strlen(buffer));
  }
  sprintf(buffer, "</Monitor>");
  out->write(buffer,strlen(buffer));
}

//////////////////// event server statistics web page //////////////////
void StorageManager::eventServerWebPage(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception)
{
  // We should make the HTML header and the page banner common
  std::string url =
    getApplicationDescriptor()->getContextDescriptor()->getURL();
  std::string urn = getApplicationDescriptor()->getURN();

  // determine whether we're automatically updating the page
  // --> if the SM is not enabled, assume that users want updating turned
  // --> ON so that they don't A) think that is is ON (when it's not) and
  // --> B) wait forever thinking that something is wrong.
  //bool autoUpdate = true;
  // 11-Jun-2008, KAB - changed auto update default to OFF
  bool autoUpdate = false;
  if(externallyVisibleState() == "Enabled") {
    cgicc::Cgicc cgiWrapper(in);
    cgicc::const_form_iterator updateRef = cgiWrapper.getElement("update");
    if (updateRef != cgiWrapper.getElements().end()) {
      std::string updateString =
        boost::algorithm::to_lower_copy(updateRef->getValue());
      if (updateString == "off") {
        autoUpdate = false;
      }
      else {
        autoUpdate = true;
      }
    }
  }

  *out << "<html>" << std::endl;
  *out << "<head>" << std::endl;
  if (autoUpdate) {
    *out << "<meta http-equiv=\"refresh\" content=\"10\">" << std::endl;
  }
  *out << "<link type=\"text/css\" rel=\"stylesheet\"";
  *out << " href=\"/" << urn << "/styles.css\"/>" << std::endl;
  *out << "<title>" << getApplicationDescriptor()->getClassName()
       << " Instance " << getApplicationDescriptor()->getInstance()
       << "</title>" << std::endl;
  *out << "<style type=\"text/css\">" << std::endl;
  *out << "  .noBotMarg {margin-bottom:0px;}" << std::endl;
  *out << "</style>" << std::endl;
  *out << "</head><body>" << std::endl;

  *out << "<table border=\"1\" width=\"100%\">"                      << endl;
  *out << "<tr>"                                                     << endl;
  *out << "  <td align=\"left\">"                                    << endl;
  *out << "    <img"                                                 << endl;
  *out << "     align=\"middle\""                                    << endl;
  *out << "     src=\"/evf/images/smicon.jpg\""                      << endl;
  *out << "     alt=\"main\""                                        << endl;
  *out << "     width=\"64\""                                        << endl;
  *out << "     height=\"64\""                                       << endl;
  *out << "     border=\"\"/>"                                       << endl;
  *out << "    <b>"                                                  << endl;
  *out << getApplicationDescriptor()->getClassName() << " Instance "
       << getApplicationDescriptor()->getInstance();
  *out << ", State is " << externallyVisibleState()              << endl;
  *out << "    </b>"                                                 << endl;
  *out << "  </td>"                                                  << endl;
  *out << "  <td width=\"32\">"                                      << endl;
  *out << "    <a href=\"/urn:xdaq-application:lid=3\">"             << endl;
  *out << "      <img"                                               << endl;
  *out << "       align=\"middle\""                                  << endl;
  *out << "       src=\"/hyperdaq/images/HyperDAQ.jpg\""             << endl;
  *out << "       alt=\"HyperDAQ\""                                  << endl;
  *out << "       width=\"32\""                                      << endl;
  *out << "       height=\"32\""                                     << endl;
  *out << "       border=\"\"/>"                                     << endl;
  *out << "    </a>"                                                 << endl;
  *out << "  </td>"                                                  << endl;
  *out << "</tr>"                                                    << endl;
  if( externallyVisibleState() == "Failed")
  {
    *out << "<tr>"                                                   << endl;
    *out << " <td>"                                                  << endl;
    *out << "<textarea rows=" << 5 << " cols=60 scroll=yes";
    *out << " readonly title=\"Reason For Failed\">"                 << endl;
    *out << reasonForFailedState_                                    << endl;
    *out << "</textarea>"                                            << endl;
    *out << " </td>"                                                 << endl;
    *out << "</tr>"                                                  << endl;
  }
  *out << "</table>"                                                 << endl;

  EventServingParams esParams =
    sharedResourcesPtr_->_configuration->getEventServingParams();

  if(externallyVisibleState() == "Enabled")
  {
    boost::shared_ptr<EventServer> eventServer =
      sharedResourcesPtr_->_oldEventServer;
    boost::shared_ptr<InitMsgCollection> initMsgCollection =
      sharedResourcesPtr_->_initMsgCollection;
    if (eventServer.get() != NULL && initMsgCollection.get() != NULL)
    {
      if (initMsgCollection->size() > 0)
      {
        int displayedConsumerCount = 0;
        double eventSum = 0.0;
        double eventRateSum = 0.0;
        double dataRateSum = 0.0;

        double now = ForeverCounter::getCurrentTime();
        *out << "<table border=\"0\" width=\"100%\">" << std::endl;
        *out << "<tr>" << std::endl;
        *out << "  <td width=\"25%\" align=\"center\">" << std::endl;
        *out << "  </td>" << std::endl;
        *out << "    &nbsp;" << std::endl;
        *out << "  <td width=\"50%\" align=\"center\">" << std::endl;
        *out << "    <font size=\"+2\"><b>Event Server Statistics</b></font>"
             << std::endl;
        *out << "    <br/>" << std::endl;
        *out << "    Data rates are reported in MB/sec." << std::endl;
        *out << "    <br/>" << std::endl;
        *out << "    Maximum input event rate is "
             << eventServer->getMaxEventRate() << " Hz." << std::endl;
        *out << "    <br/>" << std::endl;
        *out << "    Maximum input data rate is "
             << eventServer->getMaxDataRate() << " MB/sec." << std::endl;
        *out << "    <br/>" << std::endl;
        *out << "    Consumer queue size is " << esParams._consumerQueueSize
             << "." << std::endl;
        *out << "    <br/>" << std::endl;
        *out << "    Selected HLT output module is "
             << eventServer->getHLTOutputSelection()
             << "." << std::endl;
        *out << "  </td>" << std::endl;
        *out << "  <td width=\"25%\" align=\"center\">" << std::endl;
        if (autoUpdate) {
          *out << "    <a href=\"" << url << "/" << urn
               << "/EventServerStats?update=off\">Turn updating OFF</a>"
               << std::endl;
        }
        else {
          *out << "    <a href=\"" << url << "/" << urn
               << "/EventServerStats?update=on\">Turn updating ON</a>"
               << std::endl;
        }
        *out << "    <br/><br/>" << std::endl;
        *out << "    <a href=\"" << url << "/" << urn
             << "\">Back to SM Status</a>"
             << std::endl;
        *out << "  </td>" << std::endl;
        *out << "</tr>" << std::endl;
        *out << "</table>" << std::endl;

        *out << "<h3>Event Server:</h3>" << std::endl;
        *out << "<h4 class=\"noBotMarg\">Input Events, Recent Results:</h4>" << std::endl;
        *out << "<font size=\"-1\">(Events can be double-counted if they are sent by multiple output modules.)</font><br/><br/>" << std::endl;
        *out << "<table border=\"1\" width=\"100%\">" << std::endl;
        *out << "<tr>" << std::endl;
        *out << "  <th>HLT Output Module</th>" << std::endl;
        *out << "  <th>Event Count</th>" << std::endl;
        *out << "  <th>Event Rate</th>" << std::endl;
        *out << "  <th>Data Rate</th>" << std::endl;
        *out << "  <th>Duration (sec)</th>" << std::endl;
        *out << "</tr>" << std::endl;

        eventSum = 0.0;
        eventRateSum = 0.0;
        dataRateSum = 0.0;
        for (int idx = 0; idx < initMsgCollection->size(); ++idx) {
          InitMsgSharedPtr serializedProds = initMsgCollection->getElementAt(idx);
          InitMsgView initView(&(*serializedProds)[0]);
          uint32 outputModuleId = initView.outputModuleId();

          eventSum += eventServer->getEventCount(EventServer::SHORT_TERM_STATS,
                                                 EventServer::INPUT_STATS,
                                                 outputModuleId, now);
          eventRateSum += eventServer->getEventRate(EventServer::SHORT_TERM_STATS,
                                                    EventServer::INPUT_STATS,
                                                    outputModuleId, now);
          dataRateSum += eventServer->getDataRate(EventServer::SHORT_TERM_STATS,
                                                  EventServer::INPUT_STATS,
                                                  outputModuleId, now);

          *out << "<tr>" << std::endl;
          *out << "  <td align=\"center\">" << initView.outputModuleLabel()
               << "</td>" << std::endl;
          *out << "  <td align=\"center\">"
               << eventServer->getEventCount(EventServer::SHORT_TERM_STATS,
                                             EventServer::INPUT_STATS,
                                             outputModuleId, now)
               << "</td>" << std::endl;
          *out << "  <td align=\"center\">"
               << eventServer->getEventRate(EventServer::SHORT_TERM_STATS,
                                            EventServer::INPUT_STATS,
                                            outputModuleId, now)
               << "</td>" << std::endl;
          *out << "  <td align=\"center\">"
               << eventServer->getDataRate(EventServer::SHORT_TERM_STATS,
                                           EventServer::INPUT_STATS,
                                           outputModuleId, now)
               << "</td>" << std::endl;
          *out << "  <td align=\"center\">"
               << eventServer->getDuration(EventServer::SHORT_TERM_STATS,
                                           EventServer::INPUT_STATS,
                                           outputModuleId, now)
               << "</td>" << std::endl;
          *out << "</tr>" << std::endl;
        }

        // add a row with the totals
        if (initMsgCollection->size() > 1) {
          *out << "<tr>" << std::endl;
          *out << "  <td align=\"center\">Totals</td>" << std::endl;
          *out << "  <td align=\"center\">" << eventSum << "</td>" << std::endl;
          *out << "  <td align=\"center\">" << eventRateSum << "</td>" << std::endl;
          *out << "  <td align=\"center\">" << dataRateSum << "</td>" << std::endl;
          *out << "  <td align=\"center\">&nbsp;</td>" << std::endl;
          *out << "</tr>" << std::endl;
        }
        *out << "</table>" << std::endl;

        *out << "<h4 class=\"noBotMarg\">Accepted Unique Events, Recent Results:</h4>" << std::endl;
        *out << "<font size=\"-1\">(Events can be double-counted if they are sent by multiple output modules.)</font><br/><br/>" << std::endl;
        *out << "<table border=\"1\" width=\"100%\">" << std::endl;
        *out << "<tr>" << std::endl;
        *out << "  <th>HLT Output Module</th>" << std::endl;
        *out << "  <th>Event Count</th>" << std::endl;
        *out << "  <th>Event Rate</th>" << std::endl;
        *out << "  <th>Data Rate</th>" << std::endl;
        *out << "  <th>Duration (sec)</th>" << std::endl;
        *out << "</tr>" << std::endl;

        eventSum = 0.0;
        eventRateSum = 0.0;
        dataRateSum = 0.0;
        for (int idx = 0; idx < initMsgCollection->size(); ++idx) {
          InitMsgSharedPtr serializedProds = initMsgCollection->getElementAt(idx);
          InitMsgView initView(&(*serializedProds)[0]);
          uint32 outputModuleId = initView.outputModuleId();

          eventSum += eventServer->getEventCount(EventServer::SHORT_TERM_STATS,
                                                 EventServer::UNIQUE_ACCEPT_STATS,
                                                 outputModuleId, now);
          eventRateSum += eventServer->getEventRate(EventServer::SHORT_TERM_STATS,
                                                    EventServer::UNIQUE_ACCEPT_STATS,
                                                    outputModuleId, now);
          dataRateSum += eventServer->getDataRate(EventServer::SHORT_TERM_STATS,
                                                  EventServer::UNIQUE_ACCEPT_STATS,
                                                  outputModuleId, now);

          *out << "<tr>" << std::endl;
          *out << "  <td align=\"center\">" << initView.outputModuleLabel()
               << "</td>" << std::endl;
          *out << "  <td align=\"center\">"
               << eventServer->getEventCount(EventServer::SHORT_TERM_STATS,
                                             EventServer::UNIQUE_ACCEPT_STATS,
                                             outputModuleId, now)
               << "</td>" << std::endl;
          *out << "  <td align=\"center\">"
               << eventServer->getEventRate(EventServer::SHORT_TERM_STATS,
                                            EventServer::UNIQUE_ACCEPT_STATS,
                                            outputModuleId, now)
               << "</td>" << std::endl;
          *out << "  <td align=\"center\">"
               << eventServer->getDataRate(EventServer::SHORT_TERM_STATS,
                                           EventServer::UNIQUE_ACCEPT_STATS,
                                           outputModuleId, now)
               << "</td>" << std::endl;
          *out << "  <td align=\"center\">"
               << eventServer->getDuration(EventServer::SHORT_TERM_STATS,
                                           EventServer::UNIQUE_ACCEPT_STATS,
                                           outputModuleId, now)
               << "</td>" << std::endl;
          *out << "</tr>" << std::endl;
        }

        // add a row with the totals
        if (initMsgCollection->size() > 1) {
          *out << "<tr>" << std::endl;
          *out << "  <td align=\"center\">Totals</td>" << std::endl;
          *out << "  <td align=\"center\">" << eventSum << "</td>" << std::endl;
          *out << "  <td align=\"center\">" << eventRateSum << "</td>" << std::endl;
          *out << "  <td align=\"center\">" << dataRateSum << "</td>" << std::endl;
          *out << "  <td align=\"center\">&nbsp;</td>" << std::endl;
          *out << "</tr>" << std::endl;
        }
        *out << "</table>" << std::endl;

        *out << "<h4 class=\"noBotMarg\">Accepted Events To All Consumers, Recent Results:</h4>" << std::endl;
        *out << "<font size=\"-1\">(Events can be double-counted if they are sent by multiple output modules or if they are sent to multiple consumers.)</font><br/><br/>" << std::endl;
        *out << "<table border=\"1\" width=\"100%\">" << std::endl;
        *out << "<tr>" << std::endl;
        *out << "  <th>HLT Output Module</th>" << std::endl;
        *out << "  <th>Event Count</th>" << std::endl;
        *out << "  <th>Event Rate</th>" << std::endl;
        *out << "  <th>Data Rate</th>" << std::endl;
        *out << "  <th>Duration (sec)</th>" << std::endl;
        *out << "</tr>" << std::endl;

        eventSum = 0.0;
        eventRateSum = 0.0;
        dataRateSum = 0.0;
        for (int idx = 0; idx < initMsgCollection->size(); ++idx) {
          InitMsgSharedPtr serializedProds = initMsgCollection->getElementAt(idx);
          InitMsgView initView(&(*serializedProds)[0]);
          uint32 outputModuleId = initView.outputModuleId();

          eventSum += eventServer->getEventCount(EventServer::SHORT_TERM_STATS,
                                                 EventServer::OUTPUT_STATS,
                                                 outputModuleId, now);
          eventRateSum += eventServer->getEventRate(EventServer::SHORT_TERM_STATS,
                                                    EventServer::OUTPUT_STATS,
                                                    outputModuleId, now);
          dataRateSum += eventServer->getDataRate(EventServer::SHORT_TERM_STATS,
                                                  EventServer::OUTPUT_STATS,
                                                  outputModuleId, now);

          *out << "<tr>" << std::endl;
          *out << "  <td align=\"center\">" << initView.outputModuleLabel()
               << "</td>" << std::endl;
          *out << "  <td align=\"center\">"
               << eventServer->getEventCount(EventServer::SHORT_TERM_STATS,
                                             EventServer::OUTPUT_STATS,
                                             outputModuleId, now)
               << "</td>" << std::endl;
          *out << "  <td align=\"center\">"
               << eventServer->getEventRate(EventServer::SHORT_TERM_STATS,
                                            EventServer::OUTPUT_STATS,
                                            outputModuleId, now)
               << "</td>" << std::endl;
          *out << "  <td align=\"center\">"
               << eventServer->getDataRate(EventServer::SHORT_TERM_STATS,
                                           EventServer::OUTPUT_STATS,
                                           outputModuleId, now)
               << "</td>" << std::endl;
          *out << "  <td align=\"center\">"
               << eventServer->getDuration(EventServer::SHORT_TERM_STATS,
                                           EventServer::OUTPUT_STATS,
                                           outputModuleId, now)
               << "</td>" << std::endl;
          *out << "</tr>" << std::endl;
        }

        // add a row with the totals
        if (initMsgCollection->size() > 1) {
          *out << "<tr>" << std::endl;
          *out << "  <td align=\"center\">Totals</td>" << std::endl;
          *out << "  <td align=\"center\">" << eventSum << "</td>" << std::endl;
          *out << "  <td align=\"center\">" << eventRateSum << "</td>" << std::endl;
          *out << "  <td align=\"center\">" << dataRateSum << "</td>" << std::endl;
          *out << "  <td align=\"center\">&nbsp;</td>" << std::endl;
          *out << "</tr>" << std::endl;
        }
        *out << "</table>" << std::endl;

        *out << "<h4 class=\"noBotMarg\">Input Events, Full Results:</h4>" << std::endl;
        *out << "<font size=\"-1\">(Events can be double-counted if they are sent by multiple output modules.)</font><br/><br/>" << std::endl;
        *out << "<table border=\"1\" width=\"100%\">" << std::endl;
        *out << "<tr>" << std::endl;
        *out << "  <th>HLT Output Module</th>" << std::endl;
        *out << "  <th>Event Count</th>" << std::endl;
        *out << "  <th>Event Rate</th>" << std::endl;
        *out << "  <th>Data Rate</th>" << std::endl;
        *out << "  <th>Duration (sec)</th>" << std::endl;
        *out << "</tr>" << std::endl;

        eventSum = 0.0;
        eventRateSum = 0.0;
        dataRateSum = 0.0;
        for (int idx = 0; idx < initMsgCollection->size(); ++idx) {
          InitMsgSharedPtr serializedProds = initMsgCollection->getElementAt(idx);
          InitMsgView initView(&(*serializedProds)[0]);
          uint32 outputModuleId = initView.outputModuleId();

          eventSum += eventServer->getEventCount(EventServer::LONG_TERM_STATS,
                                                 EventServer::INPUT_STATS,
                                                 outputModuleId, now);
          eventRateSum += eventServer->getEventRate(EventServer::LONG_TERM_STATS,
                                                    EventServer::INPUT_STATS,
                                                    outputModuleId, now);
          dataRateSum += eventServer->getDataRate(EventServer::LONG_TERM_STATS,
                                                  EventServer::INPUT_STATS,
                                                  outputModuleId, now);

          *out << "<tr>" << std::endl;
          *out << "  <td align=\"center\">" << initView.outputModuleLabel()
               << "</td>" << std::endl;
          *out << "  <td align=\"center\">"
               << eventServer->getEventCount(EventServer::LONG_TERM_STATS,
                                             EventServer::INPUT_STATS,
                                             outputModuleId, now)
               << "</td>" << std::endl;
          *out << "  <td align=\"center\">"
               << eventServer->getEventRate(EventServer::LONG_TERM_STATS,
                                            EventServer::INPUT_STATS,
                                            outputModuleId, now)
               << "</td>" << std::endl;
          *out << "  <td align=\"center\">"
               << eventServer->getDataRate(EventServer::LONG_TERM_STATS,
                                           EventServer::INPUT_STATS,
                                           outputModuleId, now)
               << "</td>" << std::endl;
          *out << "  <td align=\"center\">"
               << eventServer->getDuration(EventServer::LONG_TERM_STATS,
                                           EventServer::INPUT_STATS,
                                           outputModuleId, now)
               << "</td>" << std::endl;
          *out << "</tr>" << std::endl;
        }

        // add a row with the totals
        if (initMsgCollection->size() > 1) {
          *out << "<tr>" << std::endl;
          *out << "  <td align=\"center\">Totals</td>" << std::endl;
          *out << "  <td align=\"center\">" << eventSum << "</td>" << std::endl;
          *out << "  <td align=\"center\">" << eventRateSum << "</td>" << std::endl;
          *out << "  <td align=\"center\">" << dataRateSum << "</td>" << std::endl;
          *out << "  <td align=\"center\">&nbsp;</td>" << std::endl;
          *out << "</tr>" << std::endl;
        }
        *out << "</table>" << std::endl;

        *out << "<h4 class=\"noBotMarg\">Accepted Unique Events, Full Results:</h4>" << std::endl;
        *out << "<font size=\"-1\">(Events can be double-counted if they are sent by multiple output modules.)</font><br/><br/>" << std::endl;
        *out << "<table border=\"1\" width=\"100%\">" << std::endl;
        *out << "<tr>" << std::endl;
        *out << "  <th>HLT Output Module</th>" << std::endl;
        *out << "  <th>Event Count</th>" << std::endl;
        *out << "  <th>Event Rate</th>" << std::endl;
        *out << "  <th>Data Rate</th>" << std::endl;
        *out << "  <th>Duration (sec)</th>" << std::endl;
        *out << "</tr>" << std::endl;

        eventSum = 0.0;
        eventRateSum = 0.0;
        dataRateSum = 0.0;
        for (int idx = 0; idx < initMsgCollection->size(); ++idx) {
          InitMsgSharedPtr serializedProds = initMsgCollection->getElementAt(idx);
          InitMsgView initView(&(*serializedProds)[0]);
          uint32 outputModuleId = initView.outputModuleId();

          eventSum += eventServer->getEventCount(EventServer::LONG_TERM_STATS,
                                                 EventServer::UNIQUE_ACCEPT_STATS,
                                                 outputModuleId, now);
          eventRateSum += eventServer->getEventRate(EventServer::LONG_TERM_STATS,
                                                    EventServer::UNIQUE_ACCEPT_STATS,
                                                    outputModuleId, now);
          dataRateSum += eventServer->getDataRate(EventServer::LONG_TERM_STATS,
                                                  EventServer::UNIQUE_ACCEPT_STATS,
                                                  outputModuleId, now);

          *out << "<tr>" << std::endl;
          *out << "  <td align=\"center\">" << initView.outputModuleLabel()
               << "</td>" << std::endl;
          *out << "  <td align=\"center\">"
               << eventServer->getEventCount(EventServer::LONG_TERM_STATS,
                                             EventServer::UNIQUE_ACCEPT_STATS,
                                             outputModuleId, now)
               << "</td>" << std::endl;
          *out << "  <td align=\"center\">"
               << eventServer->getEventRate(EventServer::LONG_TERM_STATS,
                                            EventServer::UNIQUE_ACCEPT_STATS,
                                            outputModuleId, now)
               << "</td>" << std::endl;
          *out << "  <td align=\"center\">"
               << eventServer->getDataRate(EventServer::LONG_TERM_STATS,
                                           EventServer::UNIQUE_ACCEPT_STATS,
                                           outputModuleId, now)
               << "</td>" << std::endl;
          *out << "  <td align=\"center\">"
               << eventServer->getDuration(EventServer::LONG_TERM_STATS,
                                           EventServer::UNIQUE_ACCEPT_STATS,
                                           outputModuleId, now)
               << "</td>" << std::endl;
          *out << "</tr>" << std::endl;
        }

        // add a row with the totals
        if (initMsgCollection->size() > 1) {
          *out << "<tr>" << std::endl;
          *out << "  <td align=\"center\">Totals</td>" << std::endl;
          *out << "  <td align=\"center\">" << eventSum << "</td>" << std::endl;
          *out << "  <td align=\"center\">" << eventRateSum << "</td>" << std::endl;
          *out << "  <td align=\"center\">" << dataRateSum << "</td>" << std::endl;
          *out << "  <td align=\"center\">&nbsp;</td>" << std::endl;
          *out << "</tr>" << std::endl;
        }
        *out << "</table>" << std::endl;

        *out << "<h4 class=\"noBotMarg\">Accepted Events To All Consumers, Full Results:</h4>" << std::endl;
        *out << "<font size=\"-1\">(Events can be double-counted if they are sent by multiple output modules or if they are sent to multiple consumers.)</font><br/><br/>" << std::endl;
        *out << "<table border=\"1\" width=\"100%\">" << std::endl;
        *out << "<tr>" << std::endl;
        *out << "  <th>HLT Output Module</th>" << std::endl;
        *out << "  <th>Event Count</th>" << std::endl;
        *out << "  <th>Event Rate</th>" << std::endl;
        *out << "  <th>Data Rate</th>" << std::endl;
        *out << "  <th>Duration (sec)</th>" << std::endl;
        *out << "</tr>" << std::endl;

        eventSum = 0.0;
        eventRateSum = 0.0;
        dataRateSum = 0.0;
        for (int idx = 0; idx < initMsgCollection->size(); ++idx) {
          InitMsgSharedPtr serializedProds = initMsgCollection->getElementAt(idx);
          InitMsgView initView(&(*serializedProds)[0]);
          uint32 outputModuleId = initView.outputModuleId();

          eventSum += eventServer->getEventCount(EventServer::LONG_TERM_STATS,
                                                 EventServer::OUTPUT_STATS,
                                                 outputModuleId, now);
          eventRateSum += eventServer->getEventRate(EventServer::LONG_TERM_STATS,
                                                    EventServer::OUTPUT_STATS,
                                                    outputModuleId, now);
          dataRateSum += eventServer->getDataRate(EventServer::LONG_TERM_STATS,
                                                  EventServer::OUTPUT_STATS,
                                                  outputModuleId, now);

          *out << "<tr>" << std::endl;
          *out << "  <td align=\"center\">" << initView.outputModuleLabel()
               << "</td>" << std::endl;
          *out << "  <td align=\"center\">"
               << eventServer->getEventCount(EventServer::LONG_TERM_STATS,
                                             EventServer::OUTPUT_STATS,
                                             outputModuleId, now)
               << "</td>" << std::endl;
          *out << "  <td align=\"center\">"
               << eventServer->getEventRate(EventServer::LONG_TERM_STATS,
                                            EventServer::OUTPUT_STATS,
                                            outputModuleId, now)
               << "</td>" << std::endl;
          *out << "  <td align=\"center\">"
               << eventServer->getDataRate(EventServer::LONG_TERM_STATS,
                                           EventServer::OUTPUT_STATS,
                                           outputModuleId, now)
               << "</td>" << std::endl;
          *out << "  <td align=\"center\">"
               << eventServer->getDuration(EventServer::LONG_TERM_STATS,
                                           EventServer::OUTPUT_STATS,
                                           outputModuleId, now)
               << "</td>" << std::endl;
          *out << "</tr>" << std::endl;
        }

        // add a row with the totals
        if (initMsgCollection->size() > 1) {
          *out << "<tr>" << std::endl;
          *out << "  <td align=\"center\">Totals</td>" << std::endl;
          *out << "  <td align=\"center\">" << eventSum << "</td>" << std::endl;
          *out << "  <td align=\"center\">" << eventRateSum << "</td>" << std::endl;
          *out << "  <td align=\"center\">" << dataRateSum << "</td>" << std::endl;
          *out << "  <td align=\"center\">&nbsp;</td>" << std::endl;
          *out << "</tr>" << std::endl;
        }
        *out << "</table>" << std::endl;

        *out << "<h4>Timing:</h4>" << std::endl;
        *out << "<table border=\"1\" width=\"100%\">" << std::endl;
        *out << "<tr>" << std::endl;
        *out << "  <th>&nbsp;</th>" << std::endl;
        *out << "  <th>CPU Time<br/>(sec)</th>" << std::endl;
        *out << "  <th>CPU Time<br/>Percent</th>" << std::endl;
        *out << "  <th>Real Time<br/>(sec)</th>" << std::endl;
        *out << "  <th>Real Time<br/>Percent</th>" << std::endl;
        *out << "  <th>Duration (sec)</th>" << std::endl;
        *out << "</tr>" << std::endl;
        *out << "<tr>" << std::endl;
        *out << "  <td align=\"center\">Recent Results</td>" << std::endl;
        *out << "  <td align=\"center\">"
             << eventServer->getInternalTime(EventServer::SHORT_TERM_STATS,
                                             EventServer::CPUTIME,
                                             now)
             << "</td>" << std::endl;
        *out << "  <td align=\"center\">"
             << 100 * eventServer->getTimeFraction(EventServer::SHORT_TERM_STATS,
                                                   EventServer::CPUTIME,
                                                   now)
             << "</td>" << std::endl;
        *out << "  <td align=\"center\">"
             << eventServer->getInternalTime(EventServer::SHORT_TERM_STATS,
                                             EventServer::REALTIME,
                                             now)
             << "</td>" << std::endl;
        *out << "  <td align=\"center\">"
             << 100 * eventServer->getTimeFraction(EventServer::SHORT_TERM_STATS,
                                                   EventServer::REALTIME,
                                                   now)
             << "</td>" << std::endl;
        *out << "  <td align=\"center\">"
             << eventServer->getTotalTime(EventServer::SHORT_TERM_STATS,
                                          EventServer::REALTIME,
                                          now)
             << "</td>" << std::endl;
        *out << "</tr>" << std::endl;
        *out << "<tr>" << std::endl;
        *out << "  <td align=\"center\">Full Results</td>" << std::endl;
        *out << "  <td align=\"center\">"
             << eventServer->getInternalTime(EventServer::LONG_TERM_STATS,
                                             EventServer::CPUTIME,
                                             now)
             << "</td>" << std::endl;
        *out << "  <td align=\"center\">"
             << 100 * eventServer->getTimeFraction(EventServer::LONG_TERM_STATS,
                                                   EventServer::CPUTIME,
                                                   now)
             << "</td>" << std::endl;
        *out << "  <td align=\"center\">"
             << eventServer->getInternalTime(EventServer::LONG_TERM_STATS,
                                             EventServer::REALTIME,
                                             now)
             << "</td>" << std::endl;
        *out << "  <td align=\"center\">"
             << 100 * eventServer->getTimeFraction(EventServer::LONG_TERM_STATS,
                                                   EventServer::REALTIME,
                                                   now)
             << "</td>" << std::endl;
        *out << "  <td align=\"center\">"
             << eventServer->getTotalTime(EventServer::LONG_TERM_STATS,
                                          EventServer::REALTIME,
                                          now)
             << "</td>" << std::endl;
        *out << "</tr>" << std::endl;
        *out << "</table>" << std::endl;

        *out << "<h3>Consumers:</h3>" << std::endl;
        std::map< uint32, boost::shared_ptr<ConsumerPipe> > consumerTable = 
          eventServer->getConsumerTable();
        if (consumerTable.size() == 0)
        {
          *out << "No consumers are currently registered with "
               << "this Storage Manager instance.<br/>" << std::endl;
        }
        else
        {
          std::map< uint32, boost::shared_ptr<ConsumerPipe> >::const_iterator 
            consumerIter;

          // ************************************************************
          // * Consumer summary table
          // ************************************************************
          *out << "<h4>Summary:</h4>" << std::endl;
          *out << "<table border=\"1\" width=\"100%\">" << std::endl;
          *out << "<tr>" << std::endl;
          *out << "  <th>ID</th>" << std::endl;
          *out << "  <th>Name</th>" << std::endl;
          *out << "  <th>State</th>" << std::endl;
          *out << "  <th>Requested<br/>Rate</th>" << std::endl;
          *out << "  <th>Requested HLT<br/>Output Module</th>" << std::endl;
          *out << "  <th>Trigger<br/>Request</th>" << std::endl;
          *out << "</tr>" << std::endl;

          for (consumerIter = consumerTable.begin();
               consumerIter != consumerTable.end();
               ++consumerIter)
          {
            boost::shared_ptr<ConsumerPipe> consPtr = consumerIter->second;
            *out << "<tr>" << std::endl;
            *out << "  <td align=\"center\">" << consPtr->getConsumerId()
                 << "</td>" << std::endl;

            *out << "  <td align=\"center\">";
            if (consPtr->isProxyServer()) {
              *out << "Proxy Server";
            }
            else {
              *out << consPtr->getConsumerName();
            }
            *out << "</td>" << std::endl;

            *out << "  <td align=\"center\">";
            if (consPtr->isDisconnected()) {
              *out << "Disconnected";
            }
            else if (consPtr->isIdle()) {
              *out << "Idle";
            }
            else {
              *out << "Active";
            }
            *out << "</td>" << std::endl;

            *out << "  <td align=\"center\">" << consPtr->getRateRequest()
                 << " Hz</td>" << std::endl;
            if (consPtr->isProxyServer()) {
              *out << "  <td align=\"center\">&lt;all&gt;</td>" << std::endl;
            }
            else {
              std::string hltOut = consPtr->getHLTOutputSelection();
              if (hltOut.empty()) {
                *out << "  <td align=\"center\">&lt;none&gt;</td>" << std::endl;
              }
              else {
                *out << "  <td align=\"center\">" << hltOut
                     << "</td>" << std::endl;
              }
            }
            *out << "  <td align=\"center\">"
                 << InitMsgCollection::stringsToText(consPtr->getTriggerSelection(), 5)
                 << "</td>" << std::endl;

            *out << "</tr>" << std::endl;
          }
          *out << "</table>" << std::endl;

          // ************************************************************
          // * Recent results for queued events
          // ************************************************************
          *out << "<h4>Queued Events, Recent Results:</h4>" << std::endl;
          *out << "<table border=\"1\" width=\"100%\">" << std::endl;
          *out << "<tr>" << std::endl;
          *out << "  <th>ID</th>" << std::endl;
          *out << "  <th>Name</th>" << std::endl;
          *out << "  <th>Event Count</th>" << std::endl;
          *out << "  <th>Event Rate</th>" << std::endl;
          *out << "  <th>Data Rate</th>" << std::endl;
          *out << "  <th>Duration<br/>(sec)</th>" << std::endl;
          *out << "  <th>Average<br/>Queue Size</th>" << std::endl;
          *out << "</tr>" << std::endl;

          displayedConsumerCount = 0;
          eventSum = 0.0;
          eventRateSum = 0.0;
          dataRateSum = 0.0;
          for (consumerIter = consumerTable.begin();
               consumerIter != consumerTable.end();
               ++consumerIter)
          {
            boost::shared_ptr<ConsumerPipe> consPtr = consumerIter->second;
            if (consPtr->isDisconnected()) {continue;}

            ++displayedConsumerCount;
            eventSum += consPtr->getEventCount(ConsumerPipe::SHORT_TERM,
                                               ConsumerPipe::QUEUED_EVENTS,
                                               now);
            eventRateSum += consPtr->getEventRate(ConsumerPipe::SHORT_TERM,
                                                  ConsumerPipe::QUEUED_EVENTS,
                                                  now);
            dataRateSum += consPtr->getDataRate(ConsumerPipe::SHORT_TERM,
                                                ConsumerPipe::QUEUED_EVENTS,
                                                now);

            *out << "<tr>" << std::endl;
            *out << "  <td align=\"center\">" << consPtr->getConsumerId()
                 << "</td>" << std::endl;
            *out << "  <td align=\"center\">";
            if (consPtr->isProxyServer()) {
              *out << "Proxy Server";
            }
            else {
              *out << consPtr->getConsumerName();
            }
            *out << "</td>" << std::endl;

            *out << "  <td align=\"center\">"
                 << consPtr->getEventCount(ConsumerPipe::SHORT_TERM,
                                           ConsumerPipe::QUEUED_EVENTS,
                                           now)
                 << "</td>" << std::endl;
            *out << "  <td align=\"center\">"
                 << consPtr->getEventRate(ConsumerPipe::SHORT_TERM,
                                          ConsumerPipe::QUEUED_EVENTS,
                                          now)
                 << "</td>" << std::endl;
            *out << "  <td align=\"center\">"
                 << consPtr->getDataRate(ConsumerPipe::SHORT_TERM,
                                         ConsumerPipe::QUEUED_EVENTS,
                                         now)
                 << "</td>" << std::endl;
            *out << "  <td align=\"center\">"
                 << consPtr->getDuration(ConsumerPipe::SHORT_TERM,
                                         ConsumerPipe::QUEUED_EVENTS,
                                         now)
                 << "</td>" << std::endl;
            *out << "  <td align=\"center\">"
                 << consPtr->getAverageQueueSize(ConsumerPipe::SHORT_TERM,
                                                 ConsumerPipe::QUEUED_EVENTS,
                                                 now)
                 << "</td>" << std::endl;
            *out << "</tr>" << std::endl;
          }

          // add a row with the totals
          if (displayedConsumerCount > 1) {
            *out << "<tr>" << std::endl;
            *out << "  <td align=\"center\">&nbsp;</td>" << std::endl;
            *out << "  <td align=\"center\">Totals</td>" << std::endl;
            *out << "  <td align=\"center\">" << eventSum << "</td>" << std::endl;
            *out << "  <td align=\"center\">" << eventRateSum << "</td>" << std::endl;
            *out << "  <td align=\"center\">" << dataRateSum << "</td>" << std::endl;
            *out << "  <td align=\"center\">&nbsp;</td>" << std::endl;
            *out << "  <td align=\"center\">&nbsp;</td>" << std::endl;
            *out << "</tr>" << std::endl;
          }
          *out << "</table>" << std::endl;

          // ************************************************************
          // * Recent results for served events
          // ************************************************************
          *out << "<h4>Served Events, Recent Results:</h4>" << std::endl;
          *out << "<table border=\"1\" width=\"100%\">" << std::endl;
          *out << "<tr>" << std::endl;
          *out << "  <th>ID</th>" << std::endl;
          *out << "  <th>Name</th>" << std::endl;
          *out << "  <th>Event Count</th>" << std::endl;
          *out << "  <th>Event Rate</th>" << std::endl;
          *out << "  <th>Data Rate</th>" << std::endl;
          *out << "  <th>Duration (sec)</th>" << std::endl;
          *out << "</tr>" << std::endl;

          displayedConsumerCount = 0;
          eventSum = 0.0;
          eventRateSum = 0.0;
          dataRateSum = 0.0;
          for (consumerIter = consumerTable.begin();
               consumerIter != consumerTable.end();
               ++consumerIter)
          {
            boost::shared_ptr<ConsumerPipe> consPtr = consumerIter->second;
            if (consPtr->isDisconnected()) {continue;}

            ++displayedConsumerCount;
            eventSum += consPtr->getEventCount(ConsumerPipe::SHORT_TERM,
                                               ConsumerPipe::SERVED_EVENTS,
                                               now);
            eventRateSum += consPtr->getEventRate(ConsumerPipe::SHORT_TERM,
                                                  ConsumerPipe::SERVED_EVENTS,
                                                  now);
            dataRateSum += consPtr->getDataRate(ConsumerPipe::SHORT_TERM,
                                                ConsumerPipe::SERVED_EVENTS,
                                                now);

            *out << "<tr>" << std::endl;
            *out << "  <td align=\"center\">" << consPtr->getConsumerId()
                 << "</td>" << std::endl;
            *out << "  <td align=\"center\">";
            if (consPtr->isProxyServer()) {
              *out << "Proxy Server";
            }
            else {
              *out << consPtr->getConsumerName();
            }
            *out << "</td>" << std::endl;

            *out << "  <td align=\"center\">"
                 << consPtr->getEventCount(ConsumerPipe::SHORT_TERM,
                                           ConsumerPipe::SERVED_EVENTS,
                                           now)
                 << "</td>" << std::endl;
            *out << "  <td align=\"center\">"
                 << consPtr->getEventRate(ConsumerPipe::SHORT_TERM,
                                          ConsumerPipe::SERVED_EVENTS,
                                          now)
                 << "</td>" << std::endl;
            *out << "  <td align=\"center\">"
                 << consPtr->getDataRate(ConsumerPipe::SHORT_TERM,
                                         ConsumerPipe::SERVED_EVENTS,
                                         now)
                 << "</td>" << std::endl;
            *out << "  <td align=\"center\">"
                 << consPtr->getDuration(ConsumerPipe::SHORT_TERM,
                                         ConsumerPipe::SERVED_EVENTS,
                                         now)
                 << "</td>" << std::endl;
            *out << "</tr>" << std::endl;
          }

          // add a row with the totals
          if (displayedConsumerCount > 1) {
            *out << "<tr>" << std::endl;
            *out << "  <td align=\"center\">&nbsp;</td>" << std::endl;
            *out << "  <td align=\"center\">Totals</td>" << std::endl;
            *out << "  <td align=\"center\">" << eventSum << "</td>" << std::endl;
            *out << "  <td align=\"center\">" << eventRateSum << "</td>" << std::endl;
            *out << "  <td align=\"center\">" << dataRateSum << "</td>" << std::endl;
            *out << "  <td align=\"center\">&nbsp;</td>" << std::endl;
            *out << "</tr>" << std::endl;
          }
          *out << "</table>" << std::endl;

          // ************************************************************
          // * Full results for queued events
          // ************************************************************
          *out << "<h4>Queued Events, Full Results:</h4>" << std::endl;
          *out << "<table border=\"1\" width=\"100%\">" << std::endl;
          *out << "<tr>" << std::endl;
          *out << "  <th>ID</th>" << std::endl;
          *out << "  <th>Name</th>" << std::endl;
          *out << "  <th>Event Count</th>" << std::endl;
          *out << "  <th>Event Rate</th>" << std::endl;
          *out << "  <th>Data Rate</th>" << std::endl;
          *out << "  <th>Duration<br/>(sec)</th>" << std::endl;
          *out << "  <th>Average<br/>Queue Size</th>" << std::endl;
          *out << "</tr>" << std::endl;

          displayedConsumerCount = 0;
          eventSum = 0.0;
          eventRateSum = 0.0;
          dataRateSum = 0.0;
          for (consumerIter = consumerTable.begin();
               consumerIter != consumerTable.end();
               ++consumerIter)
          {
            boost::shared_ptr<ConsumerPipe> consPtr = consumerIter->second;
            if (consPtr->isDisconnected()) {continue;}

            ++displayedConsumerCount;
            eventSum += consPtr->getEventCount(ConsumerPipe::LONG_TERM,
                                               ConsumerPipe::QUEUED_EVENTS,
                                               now);
            eventRateSum += consPtr->getEventRate(ConsumerPipe::LONG_TERM,
                                                  ConsumerPipe::QUEUED_EVENTS,
                                                  now);
            dataRateSum += consPtr->getDataRate(ConsumerPipe::LONG_TERM,
                                                ConsumerPipe::QUEUED_EVENTS,
                                                now);

            *out << "<tr>" << std::endl;
            *out << "  <td align=\"center\">" << consPtr->getConsumerId()
                 << "</td>" << std::endl;
            *out << "  <td align=\"center\">";
            if (consPtr->isProxyServer()) {
              *out << "Proxy Server";
            }
            else {
              *out << consPtr->getConsumerName();
            }
            *out << "</td>" << std::endl;

            *out << "  <td align=\"center\">"
                 << consPtr->getEventCount(ConsumerPipe::LONG_TERM,
                                           ConsumerPipe::QUEUED_EVENTS,
                                           now)
                 << "</td>" << std::endl;
            *out << "  <td align=\"center\">"
                 << consPtr->getEventRate(ConsumerPipe::LONG_TERM,
                                          ConsumerPipe::QUEUED_EVENTS,
                                          now)
                 << "</td>" << std::endl;
            *out << "  <td align=\"center\">"
                 << consPtr->getDataRate(ConsumerPipe::LONG_TERM,
                                         ConsumerPipe::QUEUED_EVENTS,
                                         now)
                 << "</td>" << std::endl;
            *out << "  <td align=\"center\">"
                 << consPtr->getDuration(ConsumerPipe::LONG_TERM,
                                         ConsumerPipe::QUEUED_EVENTS,
                                         now)
                 << "</td>" << std::endl;
            *out << "  <td align=\"center\">"
                 << consPtr->getAverageQueueSize(ConsumerPipe::LONG_TERM,
                                                 ConsumerPipe::QUEUED_EVENTS,
                                                 now)
                 << "</td>" << std::endl;
            *out << "</tr>" << std::endl;
          }

          // add a row with the totals
          if (displayedConsumerCount > 1) {
            *out << "<tr>" << std::endl;
            *out << "  <td align=\"center\">&nbsp;</td>" << std::endl;
            *out << "  <td align=\"center\">Totals</td>" << std::endl;
            *out << "  <td align=\"center\">" << eventSum << "</td>" << std::endl;
            *out << "  <td align=\"center\">" << eventRateSum << "</td>" << std::endl;
            *out << "  <td align=\"center\">" << dataRateSum << "</td>" << std::endl;
            *out << "  <td align=\"center\">&nbsp;</td>" << std::endl;
            *out << "  <td align=\"center\">&nbsp;</td>" << std::endl;
            *out << "</tr>" << std::endl;
          }
          *out << "</table>" << std::endl;

          // ************************************************************
          // * Full results for served events
          // ************************************************************
          *out << "<h4>Served Events, Full Results:</h4>" << std::endl;
          *out << "<table border=\"1\" width=\"100%\">" << std::endl;
          *out << "<tr>" << std::endl;
          *out << "  <th>ID</th>" << std::endl;
          *out << "  <th>Name</th>" << std::endl;
          *out << "  <th>Event Count</th>" << std::endl;
          *out << "  <th>Event Rate</th>" << std::endl;
          *out << "  <th>Data Rate</th>" << std::endl;
          *out << "  <th>Duration (sec)</th>" << std::endl;
          *out << "</tr>" << std::endl;

          displayedConsumerCount = 0;
          eventSum = 0.0;
          eventRateSum = 0.0;
          dataRateSum = 0.0;
          for (consumerIter = consumerTable.begin();
               consumerIter != consumerTable.end();
               ++consumerIter)
          {
            boost::shared_ptr<ConsumerPipe> consPtr = consumerIter->second;
            if (consPtr->isDisconnected ()) {continue;}

            ++displayedConsumerCount;
            eventSum += consPtr->getEventCount(ConsumerPipe::LONG_TERM,
                                               ConsumerPipe::SERVED_EVENTS,
                                               now);
            eventRateSum += consPtr->getEventRate(ConsumerPipe::LONG_TERM,
                                                  ConsumerPipe::SERVED_EVENTS,
                                                  now);
            dataRateSum += consPtr->getDataRate(ConsumerPipe::LONG_TERM,
                                                ConsumerPipe::SERVED_EVENTS,
                                                now);

            *out << "<tr>" << std::endl;
            *out << "  <td align=\"center\">" << consPtr->getConsumerId()
                 << "</td>" << std::endl;
            *out << "  <td align=\"center\">";
            if (consPtr->isProxyServer()) {
              *out << "Proxy Server";
            }
            else {
              *out << consPtr->getConsumerName();
            }
            *out << "</td>" << std::endl;

            *out << "  <td align=\"center\">"
                 << consPtr->getEventCount(ConsumerPipe::LONG_TERM,
                                           ConsumerPipe::SERVED_EVENTS,
                                           now)
                 << "</td>" << std::endl;
            *out << "  <td align=\"center\">"
                 << consPtr->getEventRate(ConsumerPipe::LONG_TERM,
                                          ConsumerPipe::SERVED_EVENTS,
                                          now)
                 << "</td>" << std::endl;
            *out << "  <td align=\"center\">"
                 << consPtr->getDataRate(ConsumerPipe::LONG_TERM,
                                         ConsumerPipe::SERVED_EVENTS,
                                         now)
                 << "</td>" << std::endl;
            *out << "  <td align=\"center\">"
                 << consPtr->getDuration(ConsumerPipe::LONG_TERM,
                                         ConsumerPipe::SERVED_EVENTS,
                                         now)
                 << "</td>" << std::endl;
            *out << "</tr>" << std::endl;
          }

          // add a row with the totals
          if (displayedConsumerCount > 1) {
            *out << "<tr>" << std::endl;
            *out << "  <td align=\"center\">&nbsp;</td>" << std::endl;
            *out << "  <td align=\"center\">Totals</td>" << std::endl;
            *out << "  <td align=\"center\">" << eventSum << "</td>" << std::endl;
            *out << "  <td align=\"center\">" << eventRateSum << "</td>" << std::endl;
            *out << "  <td align=\"center\">" << dataRateSum << "</td>" << std::endl;
            *out << "  <td align=\"center\">&nbsp;</td>" << std::endl;
            *out << "</tr>" << std::endl;
          }
          *out << "</table>" << std::endl;
        }
      }
      else
      {
        *out << "<br/>Waiting for INIT messages from the filter units...<br/>"
             << std::endl;
      }
    }
    else
    {
      *out << "<br/>The system is unable to fetch the Event Server "
           << "instance or the Init Message Collection instance. "
           << "This is a (very) unexpected error and could "
           << "be caused by an uninitialized EventServer.<br/>"
           << std::endl;
    }

    if(sharedResourcesPtr_->_initMsgCollection.get() != NULL &&
       sharedResourcesPtr_->_initMsgCollection->size() > 0)
    {
      boost::shared_ptr<InitMsgCollection> initMsgCollection =
        sharedResourcesPtr_->_initMsgCollection;
      *out << "<h3>HLT Trigger Paths:</h3>" << std::endl;
      *out << "<table border=\"1\" width=\"100%\">" << std::endl;

      {
        InitMsgSharedPtr serializedProds = initMsgCollection->getLastElement();
        InitMsgView initView(&(*serializedProds)[0]);
        Strings triggerNameList;
        initView.hltTriggerNames(triggerNameList);

        *out << "<tr>" << std::endl;
        *out << "  <td align=\"left\" valign=\"top\">"
             << "Full Trigger List</td>" << std::endl;
        *out << "  <td align=\"left\" valign=\"top\">"
             << InitMsgCollection::stringsToText(triggerNameList, 0)
             << "</td>" << std::endl;
        *out << "</tr>" << std::endl;
      }

      for (int idx = 0; idx < initMsgCollection->size(); ++idx) {
        InitMsgSharedPtr serializedProds = initMsgCollection->getElementAt(idx);
        InitMsgView initView(&(*serializedProds)[0]);
        Strings triggerSelectionList;
        initView.hltTriggerSelections(triggerSelectionList);

        *out << "<tr>" << std::endl;
        *out << "  <td align=\"left\" valign=\"top\">"
             << initView.outputModuleLabel()
             << " Output Module</td>" << std::endl;
        *out << "  <td align=\"left\" valign=\"top\">"
             << InitMsgCollection::stringsToText(triggerSelectionList, 0)
             << "</td>" << std::endl;
        *out << "</tr>" << std::endl;
      }

      *out << "</table>" << std::endl;
    }
  }
  else
  {
    *out << "<br/>Event server statistics are only available when the "
         << "Storage Manager is in the Enabled state.<br/>" << std::endl;
  }

  *out << "<br/><hr/>" << std::endl;
  char timeString[64];
  time_t now = time(0);
  strftime(timeString, 60, "%d-%b-%Y %H:%M:%S %Z", localtime(&now));
  *out << "Last updated: " << timeString << std::endl;;
  *out << "</body>" << std::endl;
  *out << "</html>" << std::endl;
}

////////////////////////////// consumer registration web page ////////////////////////////
void StorageManager::consumerWebPage(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception)
{
  if(externallyVisibleState() == "Enabled")
  { // what is the right place for this?

  std::string consumerName = "None provided";
  std::string consumerPriority = "normal";
  std::string consumerRequest = "<>";
  std::string consumerHost = in->getenv("REMOTE_HOST");

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

  // resize the local buffer, if needed, to handle a minimal response message
  unsigned int responseSize = 200;
  if (mybuffer_.capacity() < responseSize) mybuffer_.resize(responseSize);

  // fetch the event server
  // (it and/or the job controller may not have been created yet)
  boost::shared_ptr<EventServer> eventServer =
    sharedResourcesPtr_->_oldEventServer;

  // if no event server, tell the consumer that we're not ready
  if (eventServer.get() == NULL)
  {
    // build the registration response into the message buffer
    ConsRegResponseBuilder respMsg(&mybuffer_[0], mybuffer_.capacity(),
                                   ConsRegResponseBuilder::ES_NOT_READY, 0);
    // debug message so that compiler thinks respMsg is used
    FDEBUG(20) << "Registration response size =  " <<
      respMsg.size() << std::endl;
  }
  else
  {
    // resize the local buffer, if needed, to handle a full response message
    int mapStringSize = eventServer->getSelectionTableStringSize();
    responseSize += (int) (2.5 * mapStringSize);
    if (mybuffer_.capacity() < responseSize) mybuffer_.resize(responseSize);

    // fetch the event selection request from the consumer request
    edm::ParameterSet requestParamSet(consumerRequest);

    // 26-Jan-2009, KAB: an ugly hack to get ParameterSet to serialize
    // the parameters that we need.  A better solution is in the works.
    try {
      double rate =
        requestParamSet.getUntrackedParameter<double>("maxEventRequestRate",
                                                      -999.0);
      if (rate == -999.0) {
        rate = requestParamSet.getParameter<double>("TrackedMaxRate");
        requestParamSet.addUntrackedParameter<double>("maxEventRequestRate",
                                                      rate);
      }
    }
    catch (...) {}
    try {
      std::string hltOMLabel =
        requestParamSet.getUntrackedParameter<std::string>("SelectHLTOutput",
                                                           "NoneFound");
      if (hltOMLabel == "NoneFound") {
        hltOMLabel =
          requestParamSet.getParameter<std::string>("TrackedHLTOutMod");
        requestParamSet.addUntrackedParameter<std::string>("SelectHLTOutput",
                                                           hltOMLabel);
      }
    }
    catch (...) {}
    try {
      edm::ParameterSet tmpPSet1 =
        requestParamSet.getUntrackedParameter<edm::ParameterSet>("SelectEvents",
                                                                 edm::ParameterSet());
      if (tmpPSet1.empty()) {
        Strings path_specs = 
          requestParamSet.getParameter<Strings>("TrackedEventSelection");
        if (! path_specs.empty()) {
          edm::ParameterSet tmpPSet2;
          tmpPSet2.addParameter<Strings>("SelectEvents", path_specs);
          requestParamSet.addUntrackedParameter<edm::ParameterSet>("SelectEvents",
                                                                   tmpPSet2);
        }
      }
    }
    catch (...) {}

    Strings selectionRequest =
      EventSelector::getEventSelectionVString(requestParamSet);
    Strings modifiedRequest =
      eventServer->updateTriggerSelectionForStreams(selectionRequest);

    // pull the rate request out of the consumer parameter set, too
    double maxEventRequestRate =
      requestParamSet.getUntrackedParameter<double>("maxEventRequestRate", 1.0);

    // pull the HLT output module selection out of the PSet
    // (default is empty string)
    std::string hltOMLabel =
      requestParamSet.getUntrackedParameter<std::string>("SelectHLTOutput",
                                                         std::string());

    EventServingParams esParams =
      sharedResourcesPtr_->_configuration->getEventServingParams();

    // create the local consumer interface and add it to the event server
    boost::shared_ptr<ConsumerPipe>
      consPtr(new ConsumerPipe(consumerName, consumerPriority,
                               (int)esParams._activeConsumerTimeout,
                               (int)esParams._idleConsumerTimeout,
                               modifiedRequest, maxEventRequestRate,
                               hltOMLabel,
                               consumerHost,
                               esParams._consumerQueueSize));
    eventServer->addConsumer(consPtr);
    // over-ride pushmode if not set in StorageManager
    if((consumerPriority.compare("PushMode") == 0) &&
       !esParams._pushmode2proxy)
        consPtr->setPushMode(false);

    // build the registration response into the message buffer
    ConsRegResponseBuilder respMsg(&mybuffer_[0], mybuffer_.capacity(),
                                   0, consPtr->getConsumerId());

    // add the stream selection table to the proxy server response
    if (consPtr->isProxyServer()) {
      respMsg.setStreamSelectionTable(eventServer->getStreamSelectionTable());
    }

    // debug message so that compiler thinks respMsg is used
    FDEBUG(20) << "Registration response size =  " <<
      respMsg.size() << std::endl;
  }

  // send the response
  ConsRegResponseView responseMessage(&mybuffer_[0]);
  unsigned int len = responseMessage.size();

  out->getHTTPResponseHeader().addHeader("Content-Type", "application/octet-stream");
  out->getHTTPResponseHeader().addHeader("Content-Transfer-Encoding", "binary");
  out->write((char*) &mybuffer_[0],len);

  } else { // is this the right thing to send?
   // In wrong state for this message - return zero length stream, should return Msg NOTREADY
   int len = 0;
   out->getHTTPResponseHeader().addHeader("Content-Type", "application/octet-stream");
   out->getHTTPResponseHeader().addHeader("Content-Transfer-Encoding", "binary");
   out->write((char*) &mybuffer_[0],len);
  }

}

//////////// *** get DQMevent data web page //////////////////////////////////////////////////////////
void StorageManager::DQMeventdataWebPage(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception)
{
  // default the message length to zero
  int len=0;

  // determine the consumer ID from the event request
  // message, if it is available.
  unsigned int consumerId = 0;
  std::string lengthString = in->getenv("CONTENT_LENGTH");
  unsigned int contentLength = std::atol(lengthString.c_str());
  if (contentLength > 0) 
  {
    auto_ptr< vector<char> > bufPtr(new vector<char>(contentLength));
    in->read(&(*bufPtr)[0], contentLength);
    OtherMessageView requestMessage(&(*bufPtr)[0]);
    if (requestMessage.code() == Header::DQMEVENT_REQUEST)
    {
      uint8 *bodyPtr = requestMessage.msgBody();
      consumerId = convert32(bodyPtr);
    }
  }
  
  // first test if StorageManager is in Enabled state and this is a valid request
  // there must also be DQM data available
  if(externallyVisibleState() == "Enabled" && consumerId != 0)
  {
    boost::shared_ptr<DQMEventServer> eventServer =
      sharedResourcesPtr_->_oldDQMEventServer;
    if (eventServer.get() != NULL)
    {
      boost::shared_ptr< std::vector<char> > bufPtr =
        eventServer->getDQMEvent(consumerId);
      if (bufPtr.get() != NULL)
      {
        DQMEventMsgView msgView(&(*bufPtr)[0]);

        // what if mybuffer_ is used in multiple threads? Can it happen?
        unsigned char* from = msgView.startAddress();
        unsigned int dsize = msgView.size();
        if(mybuffer_.capacity() < dsize) mybuffer_.resize(dsize);
        unsigned char* pos = (unsigned char*) &mybuffer_[0];

        copy(from,from+dsize,pos);
        len = dsize;
        FDEBUG(10) << "sending update at event " << msgView.eventNumberAtUpdate() << std::endl;
      }
    }
    
    // check if zero length is sent when there is no valid data
    // i.e. on getDQMEvent, can already send zero length if request is invalid
    out->getHTTPResponseHeader().addHeader("Content-Type", "application/octet-stream");
    out->getHTTPResponseHeader().addHeader("Content-Transfer-Encoding", "binary");
    out->write((char*) &mybuffer_[0],len);
  } // else send DONE as reponse (could be end of a run)
  else
  {
    // not an event request or not in enabled state, just send DONE message
    OtherMessageBuilder othermsg(&mybuffer_[0],Header::DONE);
    len = othermsg.size();
      
    out->getHTTPResponseHeader().addHeader("Content-Type", "application/octet-stream");
    out->getHTTPResponseHeader().addHeader("Content-Transfer-Encoding", "binary");
    out->write((char*) &mybuffer_[0],len);
  }
  
}

////////////////////////////// DQM consumer registration web page ////////////////////////////
void StorageManager::DQMconsumerWebPage(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception)
{
  if(externallyVisibleState() == "Enabled")
  { // We need to be in the enabled state

    std::string consumerName = "None provided";
    std::string consumerPriority = "normal";
    std::string consumerRequest = "*";
    std::string consumerHost = in->getenv("REMOTE_HOST");

    // read the consumer registration message from the http input stream
    std::string lengthString = in->getenv("CONTENT_LENGTH");
    unsigned int contentLength = std::atol(lengthString.c_str());
    if (contentLength > 0)
    {
      auto_ptr< vector<char> > bufPtr(new vector<char>(contentLength));
      in->read(&(*bufPtr)[0], contentLength);
      ConsRegRequestView requestMessage(&(*bufPtr)[0]);
      consumerName = requestMessage.getConsumerName();
      consumerPriority = requestMessage.getConsumerPriority();
      // for DQM consumers top folder name is stored in the "parameteSet"
      std::string reqFolder = requestMessage.getRequestParameterSet();
      if (reqFolder.size() >= 1) consumerRequest = reqFolder;
    }

    // create the buffer to hold the registration reply message
    const int BUFFER_SIZE = 100;
    char msgBuff[BUFFER_SIZE];

    // fetch the DQMevent server
    // (it and/or the job controller may not have been created yet
    //  if not in the enabled state)
    boost::shared_ptr<DQMEventServer> eventServer =
      sharedResourcesPtr_->_oldDQMEventServer;

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
      EventServingParams esParams =
        sharedResourcesPtr_->_configuration->getEventServingParams();

      // create the local consumer interface and add it to the event server
      boost::shared_ptr<DQMConsumerPipe>
        consPtr(new DQMConsumerPipe(consumerName, consumerPriority,
                                    (int)esParams._DQMactiveConsumerTimeout,
                                    (int)esParams._DQMidleConsumerTimeout,
                                    consumerRequest, consumerHost,
                                    esParams._DQMconsumerQueueSize));
      eventServer->addConsumer(consPtr);
      // over-ride pushmode if not set in StorageManager
      if((consumerPriority.compare("PushMode") == 0) &&
         !esParams._pushmode2proxy)
          consPtr->setPushMode(false);

      // initialize it straight away (should later pass in the folder name to
      // optionally change the selection on a register?
      consPtr->initializeSelection();

      // build the registration response into the message buffer
      ConsRegResponseBuilder respMsg(msgBuff, BUFFER_SIZE,
                                     0, consPtr->getConsumerId());
      // debug message so that compiler thinks respMsg is used
      FDEBUG(20) << "Registration response size =  " <<
        respMsg.size() << std::endl;
    }

    // send the response
    ConsRegResponseView responseMessage(msgBuff);
    unsigned int len = responseMessage.size();
    if(mybuffer_.capacity() < len) mybuffer_.resize(len);
    for (unsigned int i=0; i<len; ++i) mybuffer_[i]=msgBuff[i];

    out->getHTTPResponseHeader().addHeader("Content-Type", "application/octet-stream");
    out->getHTTPResponseHeader().addHeader("Content-Transfer-Encoding", "binary");
    out->write((char*) &mybuffer_[0],len);

  } else { // is this the right thing to send?
   // In wrong state for this message - return zero length stream, should return Msg NOTREADY
   int len = 0;
   out->getHTTPResponseHeader().addHeader("Content-Type", "application/octet-stream");
   out->getHTTPResponseHeader().addHeader("Content-Transfer-Encoding", "binary");
   out->write((char*) &mybuffer_[0],len);
  }

}

//------------------------------------------------------------------------------
// Everything that has to do with the flash list goes here
// 
// - setupFlashList()                  - setup variables and initialize them
// - actionPerformed(xdata::Event &e)  - update values in flash list
//------------------------------------------------------------------------------
void StorageManager::setupFlashList()
{
  //----------------------------------------------------------------------------
  // Setup the header variables
  //----------------------------------------------------------------------------
  class_    = getApplicationDescriptor()->getClassName();
  instance_ = getApplicationDescriptor()->getInstance();
  std::string url;
  url       = getApplicationDescriptor()->getContextDescriptor()->getURL();
  url      += "/";
  url      += getApplicationDescriptor()->getURN();
  url_      = url;

  //----------------------------------------------------------------------------
  // Create/Retrieve an infospace which can be monitored
  //----------------------------------------------------------------------------
  std::ostringstream oss;
  oss << "urn:xdaq-monitorable-" << class_.value_;
  toolbox::net::URN urn = this->createQualifiedInfoSpace(oss.str());
  xdata::InfoSpace *is = xdata::getInfoSpaceFactory()->get(urn.toString());

  //----------------------------------------------------------------------------
  // Publish monitor data in monitorable info space -- Head
  //----------------------------------------------------------------------------

  is->fireItemAvailable("class",                &class_);
  is->fireItemAvailable("instance",             &instance_);
  is->fireItemAvailable("url",                  &url_);
  // Body
  // should this be here also??
  is->fireItemAvailable("namesOfOutMod",      &namesOfOutMod_);
  is->fireItemAvailable("storedVolume",         &storedVolume_);
  is->fireItemAvailable("memoryUsed",           &memoryUsed_);
  //  is->fireItemAvailable("progressMarker",       &progressMarker_);
  is->fireItemAvailable("connectedRBs",         &connectedRBs_);

  //----------------------------------------------------------------------------
  // Attach listener to myCounter_ to detect retrieval event
  //----------------------------------------------------------------------------
  is->addItemRetrieveListener("class",                this);
  is->addItemRetrieveListener("instance",             this);
  is->addItemRetrieveListener("url",                  this);
  // Body
  // should this be here also??
  //is->addItemRetrieveListener("storedEvents",         this);
  is->addItemRetrieveListener("namesOfOutMod", this);
  is->addItemRetrieveListener("storedVolume",         this);
  is->addItemRetrieveListener("memoryUsed",           this);
  //  is->addItemRetrieveListener("progressMarker",       this);
  is->addItemRetrieveListener("connectedRBs",         this);
  //----------------------------------------------------------------------------
}


void StorageManager::actionPerformed(xdata::Event& e)  
{
  // 14-Oct-2008, KAB - skip all processing in this method, for now,
  // when the SM state is halted.  This will protect against the use
  // of un-initialized variables.
  if (externallyVisibleState()=="Halted") {return;}
  if (externallyVisibleState()=="halting") {return;}
  // paranoia - also return if serviceManager ptr is null.  Although, to do
  // this right, we would need a lock
  if (sharedResourcesPtr_->_serviceManager.get() == 0) {return;}

  if (e.type() == "ItemRetrieveEvent") {
    std::ostringstream oss;
    oss << "urn:xdaq-monitorable:" << class_.value_ << ":" << instance_.value_;
    xdata::InfoSpace *is = xdata::InfoSpace::get(oss.str());

    is->lock();
    std::string item = dynamic_cast<xdata::ItemRetrieveEvent&>(e).itemName();
    // Only update those locations which are not always up to date
    if      (item == "connectedRBs")
      connectedRBs_   = smrbsenders_.size();
    else if (item == "memoryUsed")
      memoryUsed_     = pool_->getMemoryUsage().getUsed();
    else if (item == "receivedEventsFromOutMod" || item == "namesOfOutMod") {
      receivedEventsFromOutMod_.clear();
      namesOfOutMod_.clear();

      boost::shared_ptr<InitMsgCollection> initMsgCollection =
        sharedResourcesPtr_->_initMsgCollection;
      idMap_iter oi(modId2ModOutMap_.begin()), oe(modId2ModOutMap_.end());
      for( ; oi != oe; ++oi) {
        std::string outputModuleLabel = oi->second;
        if (initMsgCollection.get() != NULL &&
            initMsgCollection->getOutputModuleName(oi->first) != "") {
          outputModuleLabel = initMsgCollection->getOutputModuleName(oi->first);
        }
        receivedEventsFromOutMod_.push_back(receivedEventsMap_[oi->second]);
        namesOfOutMod_.push_back(outputModuleLabel);
      }
    }
    is->unlock();
  }
}


std::string StorageManager::findStreamName(const std::string &in) const
{
  //cout << "in findStreamName with string " << in << endl;
  string::size_type t = in.find("storageManager");

  string::size_type b;
  if(t != string::npos)
    {
      //cout << " storageManager is at " << t << endl;
      b = in.rfind(".",t-2);
      if(b!=string::npos) 
	{
	  //cout << "looking for substring " << t-b-2 << "long" <<endl;
	  //cout << " stream name should be at " << b+1 << endl;
	  //cout << " will return name " << string(in.substr(b+1,t-b-2)) << endl;
	  return string(in.substr(b+1,t-b-2));
	}
      else
	cout << " stream name is lost " << endl;
    }
  else
    cout << " storageManager is not found " << endl;
  return in;
}


xoap::MessageReference StorageManager::configuring( xoap::MessageReference msg )
  throw( xoap::exception::Exception )
{
  try {
    LOG4CPLUS_INFO(getApplicationLogger(),"Start configuring ...");

    sharedResourcesPtr_->_commandQueue->enq_wait( stor::event_ptr( new stor::Configure() ) );

    configureAction();

    LOG4CPLUS_INFO(getApplicationLogger(),"Finished configuring!");
  }
  catch (cms::Exception& e) {
    reasonForFailedState_ = e.explainSelf();
    LOG4CPLUS_ERROR( getApplicationLogger(), reasonForFailedState_ );
    return msg;
  }
  catch (xcept::Exception &e) {
    reasonForFailedState_ = "configuring FAILED: " + (string)e.what();
    LOG4CPLUS_ERROR( getApplicationLogger(), reasonForFailedState_ );
    return msg;
  }
  catch (std::exception& e) {
    reasonForFailedState_  = e.what();
    LOG4CPLUS_ERROR( getApplicationLogger(), reasonForFailedState_ );
    return msg;
  }
  catch (...) {
    reasonForFailedState_  = "Unknown Exception while configuring";
    LOG4CPLUS_ERROR( getApplicationLogger(), reasonForFailedState_ );
    return msg;
  }

  return msg;
}


void StorageManager::configureAction()
{

  sharedResourcesPtr_->_configuration->updateAllParams();
  DiskWritingParams dwParams =
    sharedResourcesPtr_->_configuration->getDiskWritingParams();
  DQMProcessingParams dqmParams =
    sharedResourcesPtr_->_configuration->getDQMProcessingParams();
  EventServingParams esParams =
    sharedResourcesPtr_->_configuration->getEventServingParams();

  if(!edmplugin::PluginManager::isAvailable()) {
    edmplugin::PluginManager::configure(edmplugin::standard::config());
  }

  // check output locations and scripts before we continue
  checkDirectoryOK(dwParams._filePath);
  if((bool)dqmParams._archiveDQM) checkDirectoryOK(dqmParams._filePrefixDQM);

  sharedResourcesPtr_->_oldEventServer.
    reset(new EventServer(esParams._maxESEventRate, esParams._maxESDataRate,
                          esParams._esSelectedHLTOutputModule));
  sharedResourcesPtr_->_oldDQMEventServer.
    reset(new DQMEventServer(esParams._DQMmaxESEventRate));

  sharedResourcesPtr_->_serviceManager.reset(new ServiceManager(dwParams));
  sharedResourcesPtr_->_dqmServiceManager.reset(new DQMServiceManager());
  sharedResourcesPtr_->_dqmServiceManager->setParameters(dqmParams);
  sharedResourcesPtr_->_dqmServiceManager->
    setDQMEventServer(sharedResourcesPtr_->_oldDQMEventServer);

  sharedResourcesPtr_->_oldEventServer->
    setStreamSelectionTable(sharedResourcesPtr_->_serviceManager->
                            getStreamSelectionTable());
}


xoap::MessageReference StorageManager::enabling( xoap::MessageReference msg )
  throw( xoap::exception::Exception )
{
  if (sharedResourcesPtr_->_configuration->streamConfigurationHasChanged()) {
    try {
      LOG4CPLUS_INFO(getApplicationLogger(),"Start re-configuring ...");
      sharedResourcesPtr_->_commandQueue->enq_wait( stor::event_ptr( new stor::Reconfigure() ) );
      this->haltAction();
      this->configureAction();
      LOG4CPLUS_INFO(getApplicationLogger(),"Finished re-configuring!");
    }
    catch (cms::Exception& e) {
      reasonForFailedState_ = e.explainSelf();
      LOG4CPLUS_ERROR( getApplicationLogger(), reasonForFailedState_ );
      return msg;
    }
    catch (xcept::Exception &e) {
      reasonForFailedState_ = "re-configuring FAILED: " + (string)e.what();
      LOG4CPLUS_ERROR( getApplicationLogger(), reasonForFailedState_ );
      return msg;
    }
    catch (std::exception& e) {
      reasonForFailedState_  = e.what();
      LOG4CPLUS_ERROR( getApplicationLogger(), reasonForFailedState_ );
      return msg;
    }
    catch (...) {
      reasonForFailedState_  = "Unknown Exception while re-configuring";
      LOG4CPLUS_ERROR( getApplicationLogger(), reasonForFailedState_ );
      return msg;
    }
  }

  try {
    LOG4CPLUS_INFO(getApplicationLogger(),"Start enabling ...");

    sharedResourcesPtr_->_commandQueue->enq_wait( stor::event_ptr( new stor::Enable() ) );

    smrbsenders_.clear();
    
    storedVolume_ = 0;
    receivedEventsFromOutMod_.clear();
    namesOfOutMod_.clear();
    receivedEventsMap_.clear();
    avEventSizeMap_.clear();
    avCompressRatioMap_.clear();
    modId2ModOutMap_.clear();
    storedEventsMap_.clear();

    LOG4CPLUS_INFO(getApplicationLogger(),"Finished enabling!");
  }
  catch (xcept::Exception &e) {
    reasonForFailedState_ = "enabling FAILED: " + (string)e.what();
    LOG4CPLUS_ERROR( getApplicationLogger(), reasonForFailedState_ );
    return msg;
  }
  catch(...)
  {
    reasonForFailedState_  = "Unknown Exception while enabling";
    LOG4CPLUS_ERROR( getApplicationLogger(), reasonForFailedState_ );
    return msg;
  }
  startMonitoringWorkLoop();
  return msg;
}


xoap::MessageReference StorageManager::stopping( xoap::MessageReference msg )
  throw( xoap::exception::Exception )
{
  try {
    LOG4CPLUS_INFO(getApplicationLogger(),"Start stopping ...");
    sharedResourcesPtr_->_commandQueue->enq_wait( stor::event_ptr( new stor::Stop() ) );
    stopAction();

    LOG4CPLUS_INFO(getApplicationLogger(),"Finished stopping!");
  }
  catch (xcept::Exception &e) {
    reasonForFailedState_ = "stopping FAILED: " + (string)e.what();
    LOG4CPLUS_ERROR( getApplicationLogger(), reasonForFailedState_ );
    return msg;
  }
  catch(...)
  {
    reasonForFailedState_  = "Unknown Exception while stopping";
    LOG4CPLUS_ERROR( getApplicationLogger(), reasonForFailedState_ );
    return msg;
  }
  
  return msg;
}


xoap::MessageReference StorageManager::halting( xoap::MessageReference msg )
  throw( xoap::exception::Exception )
{
  try {
    LOG4CPLUS_INFO(getApplicationLogger(),"Start halting ...");

    sharedResourcesPtr_->_commandQueue->enq_wait( stor::event_ptr( new stor::Halt() ) );

    haltAction();
    
    LOG4CPLUS_INFO(getApplicationLogger(),"Finished halting!");
  }
  catch (xcept::Exception &e) {
    reasonForFailedState_ = "halting FAILED: " + (string)e.what();
    LOG4CPLUS_ERROR( getApplicationLogger(), reasonForFailedState_ );
    return msg;
  }
  catch(...)
  {
    reasonForFailedState_  = "Unknown Exception while halting";
    LOG4CPLUS_ERROR( getApplicationLogger(), reasonForFailedState_ );
    return msg;
  }
  
  return msg;
}

void StorageManager::stopAction()
{
  receivedEventsFromOutMod_.clear();
  namesOfOutMod_.clear();

  boost::shared_ptr<InitMsgCollection> initMsgCollection =
    sharedResourcesPtr_->_initMsgCollection;
  idMap_iter oi(modId2ModOutMap_.begin()), oe(modId2ModOutMap_.end());
  for( ; oi != oe; ++oi) {
      std::string outputModuleLabel = oi->second;
      if (initMsgCollection.get() != NULL &&
          initMsgCollection->getOutputModuleName(oi->first) != "") {
        outputModuleLabel = initMsgCollection->getOutputModuleName(oi->first);
      }
      receivedEventsFromOutMod_.push_back(receivedEventsMap_[oi->second]);
      namesOfOutMod_.push_back(outputModuleLabel);
  }

  // should clear the event server(s) last event/queue
  if (sharedResourcesPtr_->_oldEventServer.get() != NULL)
  {
    sharedResourcesPtr_->_oldEventServer->clearQueue();
  }
  if (sharedResourcesPtr_->_oldDQMEventServer.get() != NULL)
  {
    sharedResourcesPtr_->_oldDQMEventServer->clearQueue();
  }
}

void StorageManager::haltAction()
{
  stopAction();
}



////////////////////////////////////////////////////////////////////////////////
/*
xoap::MessageReference StorageManager::fsmCallback(xoap::MessageReference msg)
  throw (xoap::exception::Exception)
{
  return fsm_.commandCallback(msg);
}
*/

////////////////////////////////////////////////////////////////////////////////
void StorageManager::sendDiscardMessage(unsigned int    rbBufferID, 
					unsigned int    hltInstance,
					unsigned int    msgType,
					string          hltClassName)
{
  /*
  std::cout << "sendDiscardMessage ... " 
	    << rbBufferID     << "  "
	    << hltInstance    << "  "
	    << msgType        << "  "
	    << hltClassName   << std::endl;
  */
    
  set<xdaq::ApplicationDescriptor*> setOfRBs=
    getApplicationContext()->getDefaultZone()->
    getApplicationDescriptors(hltClassName.c_str());
  
  for (set<xdaq::ApplicationDescriptor*>::iterator 
	 it=setOfRBs.begin();it!=setOfRBs.end();++it)
    {
      if ((*it)->getInstance()==hltInstance)
	{
	  
	  stor::FUProxy* proxy =  new stor::FUProxy(getApplicationDescriptor(),
						    *it,
						    getApplicationContext(),
						    pool_);
	  if ( msgType == I2O_FU_DATA_DISCARD )
	    proxy -> sendDataDiscard(rbBufferID);	
	  else if ( msgType == I2O_FU_DQM_DISCARD )
	    proxy -> sendDQMDiscard(rbBufferID);
	  else assert("Unknown discard message type" == 0);
	  delete proxy;
	}
    }
}

void StorageManager::startMonitoringWorkLoop() throw (evf::Exception)
{
  DiskWritingParams dwParams =
    sharedResourcesPtr_->_configuration->getDiskWritingParams();
  try {
    wlMonitoring_=
      toolbox::task::getWorkLoopFactory()->
      getWorkLoop(dwParams._smInstanceString+"Monitoring", "waiting");
    if (!wlMonitoring_->isActive()) wlMonitoring_->activate();
    asMonitoring_ = toolbox::task::bind(this,&StorageManager::monitoring,
                                        dwParams._smInstanceString+"Monitoring");
    wlMonitoring_->submit(asMonitoring_);
  }
  catch (xcept::Exception& e) {
    string msg = "Failed to start workloop 'Monitoring'.";
    XCEPT_RETHROW(evf::Exception,msg,e);
  }
}


bool StorageManager::monitoring(toolbox::task::WorkLoop* wl)
{
  // @@EM if state is already "failed" then no reason to firefailed again 
  //      (in fact it will cause problems) so bail out !
  if(externallyVisibleState() == "Failed") return false;
  // @@EM Look for exceptions in the FragmentCollector thread, do a state transition if present
//   if(stor::getSMFC_exceptionStatus()) {
//     edm::LogError("StorageManager") << "Fatal BURP in FragmentCollector thread detected! \n"
//        << stor::getSMFC_reason4Exception();

//     reasonForFailedState_ = stor::getSMFC_reason4Exception();
//     LOG4CPLUS_ERROR( getApplicationLogger(), reasonForFailedState_ );
//     return false; // stop monitoring workloop after going to failed state
//   }

  stor::utils::sleep(10.0);
  if(sharedResourcesPtr_->_serviceManager.get() != NULL &&
     sharedResourcesPtr_->_initMsgCollection.get() != NULL &&
     sharedResourcesPtr_->_initMsgCollection->size() > 0) {
    boost::mutex::scoped_lock sl(halt_lock_);

    {
      // this is needed only if using flashlist infospace (not for the moment)
      std::ostringstream oss;
      oss << "urn:xdaq-monitorable:" << class_.value_ << ":" << instance_.value_;
      xdata::InfoSpace *is = xdata::InfoSpace::get(oss.str());  
      is->lock();

      boost::shared_ptr<stor::SMOnlyStats> stored_stats =
        sharedResourcesPtr_->_serviceManager->get_stats();
      store_samples_ = stored_stats->samples_;
      store_period4samples_ = stored_stats->period4samples_;
      store_instantBandwidth_ = stored_stats->instantBandwidth_;
      store_instantRate_ = stored_stats->instantRate_;
      store_instantLatency_ = stored_stats->instantLatency_;
      store_totalSamples_ = (unsigned long)stored_stats->totalSamples_;
      store_duration_ = stored_stats->duration_;
      store_meanBandwidth_ = stored_stats->meanBandwidth_;
      store_meanRate_ = stored_stats->meanRate_;
      store_meanLatency_ = stored_stats->meanLatency_;
      store_maxBandwidth_ = stored_stats->maxBandwidth_;
      store_minBandwidth_ = stored_stats->minBandwidth_;
      store_instantBandwidth2_ = stored_stats->instantBandwidth2_;
      store_instantRate2_ = stored_stats->instantRate2_;
      store_instantLatency2_ = stored_stats->instantLatency2_;
      store_totalSamples2_ = (unsigned long)stored_stats->totalSamples2_;
      store_duration2_ = stored_stats->duration2_;
      store_meanBandwidth2_ = stored_stats->meanBandwidth2_;
      store_meanRate2_ = stored_stats->meanRate2_;
      store_meanLatency2_ = stored_stats->meanLatency2_;
      store_maxBandwidth2_ = stored_stats->maxBandwidth2_;
      store_minBandwidth2_ = stored_stats->minBandwidth2_;
      store_receivedVolume_ = stored_stats->receivedVolume_;
      storedVolume_   = store_receivedVolume_;

      // end temporary solution
      is->unlock();
    }
    
      
  }
    
  return true;
}

/////////////////////////////////
//// Get current state name: ////
/////////////////////////////////
std::string StorageManager::externallyVisibleState() const
{
  if( !sharedResourcesPtr_ ) return "Halted";
  if( !sharedResourcesPtr_->_statisticsReporter ) return "Halted";
  return sharedResourcesPtr_->_statisticsReporter->externallyVisibleState();
}

////////////////////////////////////////////
//// Get run number from Configuration: ////
////////////////////////////////////////////
unsigned int StorageManager::getRunNumber() const
{
  if( !sharedResourcesPtr_ ) return 0;
  if( !sharedResourcesPtr_->_configuration ) return 0;
  return sharedResourcesPtr_->_configuration->getRunNumber();
}

//////////////////////////////////////////////////////////////////////////
// *** Provides factory method for the instantiation of SM applications //
//////////////////////////////////////////////////////////////////////////
// This macro is depreciated:
XDAQ_INSTANTIATE(StorageManager)

// One should use the XDAQ_INSTANTIATOR() in the header file
// and this one here. But this breaks the backward compatibility,
// as all xml configuration files would have to be changed to use
// 'stor::StorageManager' instead of 'StorageManager'.
// XDAQ_INSTANTIATOR_IMPL(stor::StorageManager)


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
