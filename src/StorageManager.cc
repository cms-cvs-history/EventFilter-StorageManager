// $Id: StorageManager.cc,v 1.92.4.120 2009/05/15 19:50:31 biery Exp $

#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>

#include "EventFilter/StorageManager/interface/StorageManager.h"
#include "EventFilter/StorageManager/interface/ConsumerPipe.h"
#include "EventFilter/StorageManager/interface/FUProxy.h"
#include "EventFilter/StorageManager/interface/FragmentMonitorCollection.h"
#include "EventFilter/StorageManager/interface/StateMachine.h"
#include "EventFilter/StorageManager/interface/EnquingPolicyTag.h"
#include "EventFilter/StorageManager/interface/ConsumerUtils.h"
#include "EventFilter/StorageManager/interface/Exception.h"

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


StorageManager::StorageManager(xdaq::ApplicationStub * s)
  throw (xdaq::exception::Exception) :
  xdaq::Application(s),
  reasonForFailedState_(),
  mybuffer_(7000000),
  _webPageHelper( getApplicationDescriptor(),
    "$Id: StorageManager.cc,v 1.92.4.120 2009/05/15 19:50:31 biery Exp $ $Name: refdev01_scratch_branch $")
{  
  LOG4CPLUS_INFO(this->getApplicationLogger(),"Making StorageManager");

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
  xgi::bind(this,&StorageManager::defaultWebPage,           "Default");
  xgi::bind(this,&StorageManager::storedDataWebPage,        "storedData");
  xgi::bind(this,&StorageManager::css,                      "styles.css");
  xgi::bind(this,&StorageManager::rbsenderWebPage,          "rbsenderlist");
  xgi::bind(this,&StorageManager::rbsenderDetailWebPage,    "rbsenderdetail");
  xgi::bind(this,&StorageManager::fileStatisticsWebPage,    "fileStatistics");
  xgi::bind(this,&StorageManager::dqmEventStatisticsWebPage,"dqmEventStatistics");

  // Consumer bindings:
  xgi::bind( this, &StorageManager::processConsumerRegistrationRequest, "registerConsumer" );
  xgi::bind( this, &StorageManager::processConsumerHeaderRequest, "getregdata" );
  xgi::bind( this, &StorageManager::processConsumerEventRequest, "geteventdata" );

  xgi::bind(this,&StorageManager::consumerListWebPage,  "consumerList");

  xgi::bind(this,&StorageManager::processDQMConsumerRegistrationRequest, "registerDQMConsumer");
  xgi::bind(this,&StorageManager::processDQMConsumerEventRequest, "getDQMeventdata");

  // Consumer statistics page bingding:
  xgi::bind( this, &StorageManager::consumerStatisticsPage,
             "consumerStatistics" );

  // need the line below so that deserializeRegistry can run
  // in order to compare two registries (cannot compare byte-for-byte) (if we keep this)
  // need line below anyway in case we deserialize DQMEvents for collation
  edm::RootAutoLibraryLoader::enable();

  // set application icon for hyperdaq
  getApplicationDescriptor()->setAttribute("icon", "/evf/images/smicon.jpg");

  _sharedResources.reset(new SharedResources());

  xdata::InfoSpace *ispace = getApplicationInfoSpace();
  unsigned long instance = getApplicationDescriptor()->getInstance();
  _sharedResources->_configuration.reset(new Configuration(ispace, instance));

  QueueConfigurationParams queueParams =
    _sharedResources->_configuration->getQueueConfigurationParams();
  _sharedResources->_commandQueue.
    reset(new CommandQueue(queueParams._commandQueueSize));
  _sharedResources->_fragmentQueue.
    reset(new FragmentQueue(queueParams._fragmentQueueSize));
  _sharedResources->_registrationQueue.
    reset(new RegistrationQueue(queueParams._registrationQueueSize));
  _sharedResources->_streamQueue.
    reset(new StreamQueue(queueParams._streamQueueSize));
  _sharedResources->_dqmEventQueue.
    reset(new DQMEventQueue(queueParams._dqmEventQueueSize));

  _sharedResources->_statisticsReporter.reset(new StatisticsReporter(this));
  _sharedResources->_initMsgCollection.reset(new InitMsgCollection());
  _sharedResources->_diskWriterResources.reset(new DiskWriterResources());
  _sharedResources->_dqmEventProcessorResources.reset(new DQMEventProcessorResources());

  _sharedResources->
    _discardManager.reset(new DiscardManager(getApplicationContext(),
                                             getApplicationDescriptor()));

  _sharedResources->_registrationCollection.reset( new RegistrationCollection() );
  boost::shared_ptr<ConsumerMonitorCollection>
    cmcptr( _sharedResources->_statisticsReporter->getEventConsumerMonitorCollection() );
  _sharedResources->_eventConsumerQueueCollection.reset( new EventQueueCollection( cmcptr ) );
  cmcptr = _sharedResources->_statisticsReporter->getDQMConsumerMonitorCollection();
  _sharedResources->_dqmEventConsumerQueueCollection.reset( new DQMEventQueueCollection( cmcptr ) );

  // Main worker threads
  _fragmentProcessor = new FragmentProcessor( this, _sharedResources );
  _diskWriter = new DiskWriter(this, _sharedResources);
  _dqmEventProcessor = new DQMEventProcessor(this, _sharedResources);

  // Start the workloops
  try
  {
    _sharedResources->_statisticsReporter->startWorkLoop("theStatisticsReporter");
    _fragmentProcessor->startWorkLoop("theFragmentProcessor");
    _diskWriter->startWorkLoop("theDiskWriter");
    _dqmEventProcessor->startWorkLoop("theDQMEventProcessor");
  }
  catch(xcept::Exception &e)
  {
    LOG4CPLUS_FATAL(getApplicationLogger(),
      e.what() << xcept::stdformat_exception_history(e));

    #ifndef STOR_BYPASS_SENTINEL
    notifyQualified("fatal", e);
    #endif

    _sharedResources->moveToFailedState();
  }
  catch(std::exception &e)
  {
    LOG4CPLUS_FATAL(getApplicationLogger(),
      e.what());
    
    #ifndef STOR_BYPASS_SENTINEL
    XCEPT_DECLARE(stor::exception::Exception,
      sentinelException, e.what());
    notifyQualified("fatal", sentinelException);
    #endif

    _sharedResources->moveToFailedState();
  }
  catch(...)
  {
    std::string errorMsg = "Unknown exception when starting the workloops.";
    LOG4CPLUS_FATAL(getApplicationLogger(),
      errorMsg);
    
    #ifndef STOR_BYPASS_SENTINEL
    XCEPT_DECLARE(stor::exception::Exception,
      sentinelException, errorMsg);
    notifyQualified("fatal", sentinelException);
    #endif

    _sharedResources->moveToFailedState();
  }
}

StorageManager::~StorageManager()
{
  delete _fragmentProcessor;
  delete _diskWriter;
  delete _dqmEventProcessor;
}

xoap::MessageReference
StorageManager::ParameterGet(xoap::MessageReference message)
  throw (xoap::exception::Exception)
{
  return Application::ParameterGet(message);
}


////////// *** I2O frame call back functions /////////////////////////////////////////////
void StorageManager::receiveRegistryMessage(toolbox::mem::Reference *ref)
{
  I2OChain i2oChain(ref);

  // Set the I2O message pool pointer. Only done for init messages.
  ResourceMonitorCollection& resourceMonCollection =
    _sharedResources->_statisticsReporter->getResourceMonitorCollection();
  resourceMonCollection.setMemoryPoolPointer( ref->getBuffer()->getPool() );

  FragmentMonitorCollection& fragMonCollection =
    _sharedResources->_statisticsReporter->getFragmentMonitorCollection();
  fragMonCollection.getAllFragmentSizeMQ().addSample( 
    static_cast<double>( i2oChain.totalDataSize() ) / 0x100000
  );

  _sharedResources->_fragmentQueue->enq_wait(i2oChain);


  ////////////////////
  // old code below can be removed once new event server is ready
  ////////////////////

  I2O_MESSAGE_FRAME         *stdMsg  = (I2O_MESSAGE_FRAME*) ref->getDataLocation();
  I2O_SM_PREAMBLE_MESSAGE_FRAME *msg = (I2O_SM_PREAMBLE_MESSAGE_FRAME*) stdMsg;

  FDEBUG(10) << "StorageManager: Received registry message from HLT " << msg->hltURL
             << " application " << msg->hltClassName << " id " << msg->hltLocalId
             << " instance " << msg->hltInstance << " tid " << msg->hltTid
             << " rbBufferID " << msg->rbBufferID << " outModID " << msg->outModID
             << " fuProcID " << msg->fuProcID  << " fuGUID 0x" << std::hex
             << msg->fuGUID << std::dec << std::endl;
  FDEBUG(10) << "StorageManager: registry size " << msg->dataSize << "\n";

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
}

void StorageManager::receiveDataMessage(toolbox::mem::Reference *ref)
{
  I2OChain i2oChain(ref);

  FragmentMonitorCollection& fragMonCollection =
    _sharedResources->_statisticsReporter->getFragmentMonitorCollection();
  fragMonCollection.addEventFragmentSample( i2oChain.totalDataSize() );

  _sharedResources->_fragmentQueue->enq_wait(i2oChain);


  ////////////////////
  // old code below can be removed once new event server is ready
  ////////////////////

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
}

void StorageManager::receiveErrorDataMessage(toolbox::mem::Reference *ref)
{
  I2OChain i2oChain(ref);

  FragmentMonitorCollection& fragMonCollection =
    _sharedResources->_statisticsReporter->getFragmentMonitorCollection();
  fragMonCollection.addEventFragmentSample( i2oChain.totalDataSize() );

  _sharedResources->_fragmentQueue->enq_wait(i2oChain);


  ////////////////////
  // old code below can be removed once new event server is ready
  ////////////////////

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
}

void StorageManager::receiveDQMMessage(toolbox::mem::Reference *ref)
{
  I2OChain i2oChain(ref);

  FragmentMonitorCollection& fragMonCollection =
    _sharedResources->_statisticsReporter->getFragmentMonitorCollection();
  fragMonCollection.addDQMEventFragmentSample( i2oChain.totalDataSize() );

  _sharedResources->_fragmentQueue->enq_wait(i2oChain);
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
      _sharedResources
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
      _sharedResources
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

///////////////////////////////////
//// Consumer statistics page: ////
///////////////////////////////////
void StorageManager::consumerStatisticsPage( xgi::Input* in,
                                             xgi::Output* out )
  throw( xgi::exception::Exception )
{

  std::string err_msg =
    "Failed to create consumer statistics page";

  try
  {
    _webPageHelper.consumerStatistics( out,
                                       _sharedResources );
  }
  catch( std::exception &e )
  {
    err_msg += ": ";
    err_msg += e.what();
    LOG4CPLUS_ERROR( getApplicationLogger(), err_msg );
    XCEPT_RAISE( xgi::exception::Exception, err_msg );
  }
  catch(...)
  {
    err_msg += ": Unknown exception";
    LOG4CPLUS_ERROR( getApplicationLogger(), err_msg );
    XCEPT_RAISE( xgi::exception::Exception, err_msg );
  }

}


void StorageManager::rbsenderWebPage(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception)
{
  std::string errorMsg = "Failed to create the data sender webpage";

  try
  {
    _webPageHelper.resourceBrokerOverview(out, _sharedResources);
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


void StorageManager::rbsenderDetailWebPage(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception)
{
  std::string errorMsg = "Failed to create the data sender webpage";

  try
  {
    long long localRBID = 0;
    cgicc::Cgicc cgiWrapper(in);
    cgicc::const_form_iterator updateRef = cgiWrapper.getElement("id");
    if (updateRef != cgiWrapper.getElements().end())
    {
      std::string idString = updateRef->getValue();
      localRBID = boost::lexical_cast<long long>(idString);
    }

    _webPageHelper.resourceBrokerDetail(out, _sharedResources, localRBID);
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


void StorageManager::fileStatisticsWebPage(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception)
{
  std::string errorMsg = "Failed to create the file statistics webpage";

  try
  {
    _webPageHelper.filesWebPage(
      out,
      _sharedResources
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


void StorageManager::dqmEventStatisticsWebPage(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception)
{
  std::string errorMsg = "Failed to create the DQM event statistics webpage";

  try
  {
    _webPageHelper.dqmEventWebPage(
      out,
      _sharedResources
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


void StorageManager::consumerListWebPage(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception)
{
// Remi Mommsen, May 5, 2009:
// It is not clear at the moment if this XML monitoring is still useful
// for the web-based monitoring. Need to be clarified with William Badgett <badgett@fnal.gov>

//   char buffer[65536];

//   out->getHTTPResponseHeader().addHeader("Content-Type", "application/xml");
//   sprintf(buffer,
//           "<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>\n<Monitor>\n");
//   out->write(buffer,strlen(buffer));

//   if(externallyVisibleState() == "Enabled")
//   {
//     sprintf(buffer, "<ConsumerList>\n");
//     out->write(buffer,strlen(buffer));

//     boost::shared_ptr<EventServer> eventServer =
//       _sharedResources->_oldEventServer;
//     if (eventServer.get() != NULL)
//     {
//       std::map< uint32, boost::shared_ptr<ConsumerPipe> > consumerTable = 
//         eventServer->getConsumerTable();
//       std::map< uint32, boost::shared_ptr<ConsumerPipe> >::const_iterator 
//         consumerIter;
//       for (consumerIter = consumerTable.begin();
//            consumerIter != consumerTable.end();
//            ++consumerIter)
//       {
//         boost::shared_ptr<ConsumerPipe> consumerPipe = consumerIter->second;
//         sprintf(buffer, "<Consumer>\n");
//         out->write(buffer,strlen(buffer));

//         if (consumerPipe->isProxyServer()) {
//           sprintf(buffer, "<Name>Proxy Server</Name>\n");
//         }
//         else {
//           sprintf(buffer, "<Name>%s</Name>\n",
//                   consumerPipe->getConsumerName().c_str());
//         }
//         out->write(buffer,strlen(buffer));

//         sprintf(buffer, "<ID>%d</ID>\n", consumerPipe->getConsumerId());
//         out->write(buffer,strlen(buffer));

//         sprintf(buffer, "<Time>%d</Time>\n", 
//                 (int)consumerPipe->getLastEventRequestTime());
//         out->write(buffer,strlen(buffer));

//         sprintf(buffer, "<Host>%s</Host>\n", 
//                 consumerPipe->getHostName().c_str());
//         out->write(buffer,strlen(buffer));

//         sprintf(buffer, "<Events>%d</Events>\n", consumerPipe->getEvents());
//         out->write(buffer,strlen(buffer));

//         sprintf(buffer, "<Failed>%d</Failed>\n", 
//                 consumerPipe->getPushEventFailures());
//         out->write(buffer,strlen(buffer));

//         sprintf(buffer, "<Idle>%d</Idle>\n", consumerPipe->isIdle());
//         out->write(buffer,strlen(buffer));

//         sprintf(buffer, "<Disconnected>%d</Disconnected>\n", 
//                 consumerPipe->isDisconnected());
//         out->write(buffer,strlen(buffer));

//         sprintf(buffer, "<Ready>%d</Ready>\n", consumerPipe->isReadyForEvent());
//         out->write(buffer,strlen(buffer));

//         sprintf(buffer, "</Consumer>\n");
//         out->write(buffer,strlen(buffer));
//       }
//     }
//     boost::shared_ptr<DQMEventServer> dqmServer =
//       _sharedResources->_oldDQMEventServer;
//     if (dqmServer.get() != NULL)
//     {
//       std::map< uint32, boost::shared_ptr<DQMConsumerPipe> > dqmTable = 
//         dqmServer->getConsumerTable();
//       std::map< uint32, boost::shared_ptr<DQMConsumerPipe> >::const_iterator 
//         dqmIter;
//       for (dqmIter = dqmTable.begin();
//            dqmIter != dqmTable.end();
//            ++dqmIter)
//       {
//         boost::shared_ptr<DQMConsumerPipe> dqmPipe = dqmIter->second;
//         sprintf(buffer, "<DQMConsumer>\n");
//         out->write(buffer,strlen(buffer));

//         if (dqmPipe->isProxyServer()) {
//           sprintf(buffer, "<Name>Proxy Server</Name>\n");
//         }
//         else {
//           sprintf(buffer, "<Name>%s</Name>\n",
//                   dqmPipe->getConsumerName().c_str());
//         }
//         out->write(buffer,strlen(buffer));

//         sprintf(buffer, "<ID>%d</ID>\n", dqmPipe->getConsumerId());
//         out->write(buffer,strlen(buffer));

//         sprintf(buffer, "<Time>%d</Time>\n", 
//                 (int)dqmPipe->getLastEventRequestTime());
//         out->write(buffer,strlen(buffer));

//         sprintf(buffer, "<Host>%s</Host>\n", 
//                 dqmPipe->getHostName().c_str());
//         out->write(buffer,strlen(buffer));

//         sprintf(buffer, "<Events>%d</Events>\n", dqmPipe->getEvents());
//         out->write(buffer,strlen(buffer));

//         sprintf(buffer, "<Failed>%d</Failed>\n", 
//                 dqmPipe->getPushEventFailures());
//         out->write(buffer,strlen(buffer));

//         sprintf(buffer, "<Idle>%d</Idle>\n", dqmPipe->isIdle());
//         out->write(buffer,strlen(buffer));

//         sprintf(buffer, "<Disconnected>%d</Disconnected>\n", 
//                 dqmPipe->isDisconnected());
//         out->write(buffer,strlen(buffer));

//         sprintf(buffer, "<Ready>%d</Ready>\n", dqmPipe->isReadyForEvent());
//         out->write(buffer,strlen(buffer));

//         sprintf(buffer, "</DQMConsumer>\n");
//         out->write(buffer,strlen(buffer));
//       }
//     }
//     sprintf(buffer, "</ConsumerList>\n");
//     out->write(buffer,strlen(buffer));
//   }
//   sprintf(buffer, "</Monitor>");
//   out->write(buffer,strlen(buffer));
}


xoap::MessageReference StorageManager::configuring( xoap::MessageReference msg )
  throw( xoap::exception::Exception )
{
  try {
    LOG4CPLUS_INFO(getApplicationLogger(),"Start configuring ...");

    _sharedResources->_commandQueue->enq_wait( stor::event_ptr( new stor::Configure() ) );

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

  _sharedResources->_configuration->updateAllParams();
  DiskWritingParams dwParams =
    _sharedResources->_configuration->getDiskWritingParams();
  DQMProcessingParams dqmParams =
    _sharedResources->_configuration->getDQMProcessingParams();
  EventServingParams esParams =
    _sharedResources->_configuration->getEventServingParams();

  if(!edmplugin::PluginManager::isAvailable()) {
    edmplugin::PluginManager::configure(edmplugin::standard::config());
  }

  _sharedResources->_oldEventServer.
    reset(new EventServer(esParams._maxESEventRate, esParams._maxESDataRate,
                          esParams._esSelectedHLTOutputModule));
}


xoap::MessageReference StorageManager::enabling( xoap::MessageReference msg )
  throw( xoap::exception::Exception )
{
  if (_sharedResources->_configuration->streamConfigurationHasChanged()) {
    try {
      LOG4CPLUS_INFO(getApplicationLogger(),"Start re-configuring ...");
      _sharedResources->_commandQueue->enq_wait( stor::event_ptr( new stor::Reconfigure() ) );
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

    _sharedResources->_commandQueue->enq_wait( stor::event_ptr( new stor::Enable() ) );

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
  return msg;
}


xoap::MessageReference StorageManager::stopping( xoap::MessageReference msg )
  throw( xoap::exception::Exception )
{
  try {
    LOG4CPLUS_INFO(getApplicationLogger(),"Start stopping ...");
    _sharedResources->_commandQueue->enq_wait( stor::event_ptr( new stor::Stop() ) );
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

    _sharedResources->_commandQueue->enq_wait( stor::event_ptr( new stor::Halt() ) );

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
  // should clear the event server(s) last event/queue
  if (_sharedResources->_oldEventServer.get() != NULL)
  {
    _sharedResources->_oldEventServer->clearQueue();
  }
}

void StorageManager::haltAction()
{
  stopAction();
}


/////////////////////////////////
//// Get current state name: ////
/////////////////////////////////
std::string StorageManager::externallyVisibleState() const
{
  if( !_sharedResources ) return "Halted";
  if( !_sharedResources->_statisticsReporter ) return "Halted";
  return _sharedResources->_statisticsReporter->
    getStateMachineMonitorCollection().externallyVisibleState();
}

////////////////////////////////////////////
//// Get run number from Configuration: ////
////////////////////////////////////////////
unsigned int StorageManager::getRunNumber() const
{
  if( !_sharedResources ) return 0;
  if( !_sharedResources->_configuration ) return 0;
  return _sharedResources->_configuration->getRunNumber();
}

/////////////////////////////////////////////
//// New consumer registration callback: ////
/////////////////////////////////////////////
void
StorageManager::processConsumerRegistrationRequest( xgi::Input* in, xgi::Output* out )
  throw( xgi::exception::Exception )
{

  // Get consumer ID if registration is allowed:
  ConsumerID cid = _sharedResources->_registrationCollection->getConsumerID();
  if( !cid.isValid() )
    {
      writeNotReady( out );
      return;
    }

  const utils::duration_t secs2stale =
    _sharedResources->_configuration->getEventServingParams()._activeConsumerTimeout;
  const enquing_policy::PolicyTag policy = enquing_policy::DiscardOld;
  const size_t qsize =
    _sharedResources->_configuration->getEventServingParams()._consumerQueueSize;

  // Create registration info and set consumer ID:
  stor::ConsRegPtr reginfo;
  std::string errorMsg = "Error parsing an event consumer registration request";
  try
    {
      reginfo = parseEventConsumerRegistration( in, qsize, policy, secs2stale );
    }
  catch ( edm::Exception& excpt )
    {
      errorMsg.append( ": " );
      errorMsg.append( excpt.what() );

      LOG4CPLUS_ERROR(this->getApplicationLogger(), errorMsg);

      #ifndef STOR_BYPASS_SENTINEL
      XCEPT_DECLARE(stor::exception::ConsumerRegistration,
      sentinelException, errorMsg);
      notifyQualified("error", sentinelException);
      #endif

      writeErrorString( out, errorMsg );
      return;
    }
  catch ( xcept::Exception& excpt )
    {
      LOG4CPLUS_ERROR(this->getApplicationLogger(),
        errorMsg << xcept::stdformat_exception_history(excpt));

      #ifndef STOR_BYPASS_SENTINEL
      XCEPT_DECLARE_NESTED(stor::exception::ConsumerRegistration,
      sentinelException, errorMsg, excpt);
      notifyQualified("error", sentinelException);
      #endif

      writeErrorString( out, errorMsg );
      return;
    }
  catch ( ... )
    {
      errorMsg.append( ": unknown exception" );

      LOG4CPLUS_ERROR(this->getApplicationLogger(), errorMsg);

      #ifndef STOR_BYPASS_SENTINEL
      XCEPT_DECLARE(stor::exception::ConsumerRegistration,
      sentinelException, errorMsg);
      notifyQualified("error", sentinelException);
      #endif

      writeErrorString( out, errorMsg );
      return;
    }
  reginfo->setConsumerID( cid );

  // Create queue and set queue ID:
  QueueID qid =
    _sharedResources->_eventConsumerQueueCollection->createQueue( cid,
                                                                  policy,
                                                                  qsize,
                                                                  secs2stale );
  if( !qid.isValid() )
    {
      writeNotReady( out );
      return;
    }

  reginfo->setQueueID( qid );

  // Register consumer with InitMsgCollection:
  bool reg_ok =
    _sharedResources->_initMsgCollection->registerConsumer( cid,
                                                            reginfo->selHLTOut() );
  if( !reg_ok )
    {
      writeNotReady( out );
      return;
    }

  // Add registration to collection:
  bool add_ok = 
    _sharedResources->_registrationCollection->addRegistrationInfo( cid,
                                                                    reginfo );
  if( !add_ok )
    {
      writeNotReady( out );
      return;
    }

  // Put registration on the queue:
  _sharedResources->_registrationQueue->enq_wait( reginfo );

  // Reply to consumer:
  writeConsumerRegistration( out, cid );

}

///////////////////////////////////////
//// New consumer header callback: ////
///////////////////////////////////////
void
StorageManager::processConsumerHeaderRequest( xgi::Input* in, xgi::Output* out )
  throw( xgi::exception::Exception )
{

  ConsumerID cid = getConsumerID( in );
  if( !cid.isValid() )
    {
      writeEmptyBuffer( out );
      return;
    }

  // 20-Apr-2009, KAB - treat the proxy server like any other consumer. If
  // and when we need to support multiple HLT output modules with the proxy
  // server, then we can go back to sending the full InitMsgCollection.
  InitMsgSharedPtr payload =
    _sharedResources->_initMsgCollection->getElementForConsumer( cid );

  if( payload.get() == NULL )
    {
      writeEmptyBuffer( out );
      return;
    }

  writeConsumerHeader( out, payload );

}

//////////////////////////////////////
//// New consumer event callback: ////
//////////////////////////////////////
void
StorageManager::processConsumerEventRequest( xgi::Input* in, xgi::Output* out )
  throw( xgi::exception::Exception )
{

  ConsumerID cid = getConsumerID( in );
  if( !cid.isValid() )
    {
      writeEmptyBuffer( out );
      return;
    }

  if ( !_sharedResources->_registrationCollection->registrationIsAllowed() )
    {
      writeDone( out );
      return;
    }

  I2OChain evt =
    _sharedResources->_eventConsumerQueueCollection->popEvent( cid );
  if( evt.faulty() )
    {
      writeEmptyBuffer( out );
      return;
    }

  writeConsumerEvent( out, evt );
}

/////////////////////////////////////////////////
//// New DQM consumer registration callback: ////
/////////////////////////////////////////////////
void
StorageManager::processDQMConsumerRegistrationRequest( xgi::Input* in, xgi::Output* out )
  throw( xgi::exception::Exception )
{

  // Get consumer ID if registration is allowed:
  ConsumerID cid = _sharedResources->_registrationCollection->getConsumerID();
  if( !cid.isValid() )
    {
      writeNotReady( out );
      return;
    }

  const utils::duration_t secs2stale =
    _sharedResources->_configuration->getEventServingParams()._DQMactiveConsumerTimeout;
  const enquing_policy::PolicyTag policy = enquing_policy::DiscardOld;
  const size_t qsize =
    _sharedResources->_configuration->getEventServingParams()._consumerQueueSize;

  // Create registration info and set consumer ID:
  stor::DQMEventConsRegPtr dqmreginfo;
  std::string errorMsg = "Error parsing a DQM event consumer registration request";
  try
    {
      dqmreginfo = parseDQMEventConsumerRegistration( in, qsize, policy, secs2stale );
    }
  catch ( edm::Exception& excpt )
    {
      errorMsg.append( ": " );
      errorMsg.append( excpt.what() );

      LOG4CPLUS_ERROR(this->getApplicationLogger(), errorMsg);

      #ifndef STOR_BYPASS_SENTINEL
      XCEPT_DECLARE(stor::exception::DQMConsumerRegistration,
      sentinelException, errorMsg);
      notifyQualified("error", sentinelException);
      #endif

      writeErrorString( out, errorMsg );
      return;
    }
  catch ( xcept::Exception& excpt )
    {
      LOG4CPLUS_ERROR(this->getApplicationLogger(),
        errorMsg << xcept::stdformat_exception_history(excpt));

      #ifndef STOR_BYPASS_SENTINEL
      XCEPT_DECLARE_NESTED(stor::exception::DQMConsumerRegistration,
      sentinelException, errorMsg, excpt);
      notifyQualified("error", sentinelException);
      #endif

      writeErrorString( out, errorMsg );
      return;
    }
  catch ( ... )
    {
      errorMsg.append( ": unknown exception" );

      LOG4CPLUS_ERROR(this->getApplicationLogger(), errorMsg);

      #ifndef STOR_BYPASS_SENTINEL
      XCEPT_DECLARE(stor::exception::DQMConsumerRegistration,
      sentinelException, errorMsg);
      notifyQualified("error", sentinelException);
      #endif

      writeErrorString( out, errorMsg );
      return;
    }
  dqmreginfo->setConsumerID( cid );

  // Create queue and set queue ID:
  QueueID qid =
    _sharedResources->_dqmEventConsumerQueueCollection->createQueue( cid,
                                                                     policy,
                                                                     qsize,
                                                                     secs2stale );
  if( !qid.isValid() )
    {
      writeNotReady( out );
      return;
    }

  dqmreginfo->setQueueID( qid );

  // Add registration to collection:
  bool add_ok = 
    _sharedResources->_registrationCollection->addRegistrationInfo( cid,
                                                                    dqmreginfo );
  if( !add_ok )
    {
      writeNotReady( out );
      return;
    }

  // Put registration on the queue:
  _sharedResources->_registrationQueue->enq_wait( dqmreginfo );

  // Reply to consumer:
  writeConsumerRegistration( out, cid );
}

//////////////////////////////////////////
//// New DQM consumer event callback: ////
//////////////////////////////////////////
void
StorageManager::processDQMConsumerEventRequest( xgi::Input* in, xgi::Output* out )
  throw( xgi::exception::Exception )
{

  ConsumerID cid = getConsumerID( in );
  if( !cid.isValid() )
    {
      writeEmptyBuffer( out );
      return;
    }

  if ( !_sharedResources->_registrationCollection->registrationIsAllowed() )
    {
      writeDone( out );
      return;
    }

  DQMEventRecord::GroupRecord dqmGroupRecord =
    _sharedResources->_dqmEventConsumerQueueCollection->popEvent( cid );

  if ( !dqmGroupRecord.empty() )
    writeDQMConsumerEvent( out, dqmGroupRecord.getDQMEventMsgView() );
  else
    writeEmptyBuffer( out );
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
