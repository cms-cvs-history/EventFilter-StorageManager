// $Id: DQMEventProcessor.cc,v 1.1.2.6 2009/04/17 10:42:49 mommsen Exp $

#include "toolbox/task/WorkLoopFactory.h"
#include "xcept/tools.h"

#include "EventFilter/StorageManager/interface/Exception.h"
#include "EventFilter/StorageManager/interface/DQMEventProcessor.h"

using namespace stor;


DQMEventProcessor::DQMEventProcessor(xdaq::Application *app, SharedResourcesPtr sr) :
_app(app),
_sharedResources(sr),
_actionIsActive(true),
_dqmEventStore(sr->_configuration->getDQMProcessingParams())
{
  WorkerThreadParams workerParams =
    _sharedResources->_configuration->getWorkerThreadParams();
  _timeout = (unsigned int) workerParams._DQMEPdeqWaitTime;
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
  processNextCompletedDQMEventRecord();
}


void DQMEventProcessor::processNextCompletedDQMEventRecord()
{
  DQMEventRecord dqmRecord;
  if ( _dqmEventStore.getCompletedDQMEventRecordIfAvailable(dqmRecord) )
  {
    _sharedResources->
      _dqmEventConsumerQueueCollection->addEvent(dqmRecord);
  }
}


QueueID DQMEventProcessor::registerDQMEventConsumer
(
  DQMEventConsumerRegistrationInfo const& ri
)
{
  return _sharedResources->
    _dqmEventConsumerQueueCollection->createQueue(ri.consumerID(),
                                                  ri.queuePolicy(),
                                                  ri.maxQueueSize());
}


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
