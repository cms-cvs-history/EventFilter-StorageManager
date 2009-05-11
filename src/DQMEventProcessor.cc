// $Id: DQMEventProcessor.cc,v 1.1.2.12 2009/05/04 12:36:17 mommsen Exp $

#include "toolbox/task/WorkLoopFactory.h"
#include "xcept/tools.h"

#include "EventFilter/StorageManager/interface/Exception.h"
#include "EventFilter/StorageManager/interface/DQMEventProcessor.h"

using namespace stor;


DQMEventProcessor::DQMEventProcessor(xdaq::Application *app, SharedResourcesPtr sr) :
_app(app),
_sharedResources(sr),
_actionIsActive(true),
_dqmEventStore( sr->_statisticsReporter->getDQMEventMonitorCollection() )
{
  WorkerThreadParams workerParams =
    _sharedResources->_configuration->getWorkerThreadParams();
  _timeout = static_cast<unsigned int>(workerParams._DQMEPdeqWaitTime);
}


DQMEventProcessor::~DQMEventProcessor()
{
  // Stop the activity
  _actionIsActive = false;

  // Cancel the workloop (will wait until the action has finished)
  _processWL->cancel();
}


void DQMEventProcessor::startWorkLoop(std::string workloopName)
{
  try
  {
    std::string identifier = utils::getIdentifier(_app->getApplicationDescriptor());
    
    _processWL = toolbox::task::getWorkLoopFactory()->
      getWorkLoop( identifier + workloopName, "waiting" );
    
    if ( ! _processWL->isActive() )
    {
      toolbox::task::ActionSignature* processAction = 
        toolbox::task::bind(this, &DQMEventProcessor::processDQMEvents,
          identifier + "ProcessNextDQMEvent");
      _processWL->submit(processAction);
      
      _processWL->activate();
    }
  }
  catch (xcept::Exception& e)
  {
    std::string msg = "Failed to start workloop 'DQMEventProcessor' with 'processNextDQMEvent'.";
    XCEPT_RETHROW(stor::exception::DQMEventProcessing, msg, e);
  }
}


bool DQMEventProcessor::processDQMEvents(toolbox::task::WorkLoop*)
{
  std::string errorMsg = "Failed to process a DQM event: ";
  
  try
  {
    processNextDQMEvent();
  }
  catch(xcept::Exception &e)
  {
    LOG4CPLUS_FATAL(_app->getApplicationLogger(),
      errorMsg << xcept::stdformat_exception_history(e));

    XCEPT_DECLARE_NESTED(stor::exception::DQMEventProcessing,
      sentinelException, errorMsg, e);
    _app->notifyQualified("fatal", sentinelException);
    _sharedResources->moveToFailedState();
  }
  catch(std::exception &e)
  {
    errorMsg += e.what();

    LOG4CPLUS_FATAL(_app->getApplicationLogger(),
      errorMsg);
    
    XCEPT_DECLARE(stor::exception::DQMEventProcessing,
      sentinelException, errorMsg);
    _app->notifyQualified("fatal", sentinelException);
    _sharedResources->moveToFailedState();
  }
  catch(...)
  {
    errorMsg += "Unknown exception";

    LOG4CPLUS_FATAL(_app->getApplicationLogger(),
      errorMsg);
    
    XCEPT_DECLARE(stor::exception::DQMEventProcessing,
      sentinelException, errorMsg);
    _app->notifyQualified("fatal", sentinelException);
    _sharedResources->moveToFailedState();
  }

  return _actionIsActive;
}


void DQMEventProcessor::processNextDQMEvent()
{
  I2OChain dqmEvent;
  boost::shared_ptr<DQMEventQueue> eq = _sharedResources->_dqmEventQueue;
  if (eq->deq_timed_wait(dqmEvent, _timeout))
  {
    _dqmEventStore.addDQMEvent(dqmEvent);
  }

  processCompleteDQMEventRecords();

  DQMProcessingParams dqmParams;
  double newTimeoutValue;
  if (_sharedResources->_dqmEventProcessorResources->
    configurationRequested(dqmParams, newTimeoutValue))
  {
    _timeout = static_cast<unsigned int>(newTimeoutValue);
    _dqmEventStore.setParameters(dqmParams);
    checkDirectories(dqmParams);
    _sharedResources->_dqmEventProcessorResources->configurationDone();
  }

  if (_sharedResources->_dqmEventProcessorResources->
      endOfRunRequested())
  {
    endOfRun();
    _sharedResources->_dqmEventProcessorResources->endOfRunDone();
  }

  if (_sharedResources->_dqmEventProcessorResources->
      storeDestructionRequested())
  {
    _dqmEventStore.clear();
    _sharedResources->_dqmEventProcessorResources->storeDestructionDone();
  }
}

void DQMEventProcessor::endOfRun()
{
  _dqmEventStore.writeAndPurgeAllDQMInstances();
  processCompleteDQMEventRecords();
}


void DQMEventProcessor::processCompleteDQMEventRecords()
{
  DQMEventRecord::GroupRecord dqmRecordEntry;
  while ( _dqmEventStore.getCompletedDQMGroupRecordIfAvailable(dqmRecordEntry) )
  {
    _sharedResources->
      _dqmEventConsumerQueueCollection->addEvent(dqmRecordEntry);
  }
}


void DQMEventProcessor::checkDirectories(DQMProcessingParams const& dqmParams) const
{
  if ( dqmParams._archiveDQM )
  {
    utils::checkDirectory(dqmParams._filePrefixDQM);
  }
}


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
