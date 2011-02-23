// $Id: DQMEventProcessor.cc,v 1.12.2.2 2011/02/03 14:16:28 mommsen Exp $
/// @file: DQMEventProcessor.cc

#include "toolbox/task/WorkLoopFactory.h"
#include "xcept/tools.h"

#include "EventFilter/StorageManager/interface/Exception.h"
#include "EventFilter/StorageManager/interface/DQMEventProcessor.h"
#include "EventFilter/StorageManager/interface/DQMEventProcessorResources.h"
#include "EventFilter/StorageManager/interface/DQMEventQueueCollection.h"
#include "EventFilter/StorageManager/interface/QueueID.h"
#include "EventFilter/StorageManager/interface/StatisticsReporter.h"

using namespace stor;


DQMEventProcessor::DQMEventProcessor(xdaq::Application *app, SharedResourcesPtr sr) :
_app(app),
_sharedResources(sr),
_actionIsActive(true),
_dqmEventStore(sr)
{
  WorkerThreadParams workerParams =
    _sharedResources->_configuration->getWorkerThreadParams();
  _timeout = workerParams._DQMEPdeqWaitTime;
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
    XCEPT_DECLARE_NESTED( stor::exception::DQMEventProcessing,
                          sentinelException, errorMsg, e );
    _sharedResources->moveToFailedState(sentinelException);
  }
  catch(std::exception &e)
  {
    errorMsg += e.what();
    XCEPT_DECLARE( stor::exception::DQMEventProcessing,
                   sentinelException, errorMsg );
    _sharedResources->moveToFailedState(sentinelException);
  }
  catch(...)
  {
    errorMsg += "Unknown exception";
    XCEPT_DECLARE( stor::exception::DQMEventProcessing,
                   sentinelException, errorMsg );
    _sharedResources->moveToFailedState(sentinelException);
  }

  return _actionIsActive;
}


void DQMEventProcessor::processNextDQMEvent()
{
  DQMEventQueue::value_type dqmEvent;
  DQMEventQueuePtr eq = _sharedResources->_dqmEventQueue;
  utils::time_point_t startTime = utils::getCurrentTime();
  if (eq->deq_timed_wait(dqmEvent, _timeout))
  {
    utils::duration_t elapsedTime = utils::getCurrentTime() - startTime;
    _sharedResources->_statisticsReporter->getThroughputMonitorCollection().addDQMEventProcessorIdleSample(elapsedTime);
    _sharedResources->_statisticsReporter->getThroughputMonitorCollection().addPoppedDQMEventSample(dqmEvent.first.memoryUsed());
    // dqmEvent.second would give us the number of dropped updates

    _dqmEventStore.addDQMEvent(dqmEvent.first);
  }
  else
  {
    utils::duration_t elapsedTime = utils::getCurrentTime() - startTime;
    _sharedResources->_statisticsReporter->getThroughputMonitorCollection().addDQMEventProcessorIdleSample(elapsedTime);
  }

  processCompletedTopLevelFolders();

  DQMEventProcessorResources::Requests requests;
  DQMProcessingParams dqmParams;
  boost::posix_time::time_duration newTimeoutValue;
  if (_sharedResources->_dqmEventProcessorResources->
    getRequests(requests, dqmParams, newTimeoutValue))
  {
    if (requests.configuration)
    {
      _timeout = newTimeoutValue;
      _dqmEventStore.setParameters(dqmParams);
    }
    if (requests.endOfRun)
    {
      endOfRun();
    }
    if (requests.storeDestruction)
    {
      _dqmEventStore.clear();
    }
    _sharedResources->_dqmEventProcessorResources->requestsDone();
  }
}

void DQMEventProcessor::endOfRun()
{
  _dqmEventStore.purge();
  processCompletedTopLevelFolders();
}


void DQMEventProcessor::processCompletedTopLevelFolders()
{
  DQMTopLevelFolder::Record topLevelFolderRecord;
  while ( _dqmEventStore.getCompletedTopLevelFolderIfAvailable(topLevelFolderRecord) )
  {
    _sharedResources->
      _dqmEventQueueCollection->addEvent(topLevelFolderRecord);
  }
}


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
