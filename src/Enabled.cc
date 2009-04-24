// $Id: Enabled.cc,v 1.1.2.34 2009/04/23 19:19:49 mommsen Exp $

#include "EventFilter/StorageManager/interface/EventDistributor.h"
#include "EventFilter/StorageManager/interface/FragmentStore.h"
#include "EventFilter/StorageManager/interface/StateMachine.h"
#include "EventFilter/StorageManager/interface/SharedResources.h"

#include <iostream>

using namespace std;
using namespace stor;

Enabled::Enabled( my_context c ): my_base(c)
{
  TransitionRecord tr( stateName(), true );
  outermost_context().updateHistory( tr );

  SharedResourcesPtr sharedResources =
    outermost_context().getSharedResources();

  // reset all statistics (needs to be done first)
  sharedResources->_statisticsReporter->reset();

  // clear the INIT message collection at begin run
  sharedResources->_initMsgCollection->clear();

  // update the run-based configuration parameters
  sharedResources->_configuration->updateRunParams();

  // disk writer and (dqm) event distributors begin-run processing
  EvtStrConfigList evtCfgList = sharedResources->_configuration->
    getCurrentEventStreamConfig();
  ErrStrConfigList errCfgList = sharedResources->_configuration->
    getCurrentErrorStreamConfig();

  WorkerThreadParams workerParams =
    sharedResources->_configuration->getWorkerThreadParams();
  sharedResources->_diskWriterResources->
    requestStreamConfiguration(&evtCfgList, &errCfgList,
                               workerParams._DWdeqWaitTime);
  sharedResources->_dqmEventProcessorResources->
    requestConfiguration(
      sharedResources->_configuration->getDQMProcessingParams(),
      workerParams._DQMEPdeqWaitTime);

  sharedResources->_diskWriterResources->waitForStreamConfiguration();
  sharedResources->_dqmEventProcessorResources->waitForConfiguration();

  EventDistributor* ed = outermost_context().getEventDistributor();
  ed->registerEventStreams(evtCfgList);
  ed->registerErrorStreams(errCfgList);

  // Clear old consumer registrations:
  sharedResources->_registrationCollection->clearRegistrations();
  ed->clearConsumers();
  sharedResources->_eventConsumerQueueCollection->removeQueues();
  sharedResources->_dqmEventConsumerQueueCollection->removeQueues();

  // Enable consumer registration:
  sharedResources->_registrationCollection->enableConsumerRegistration();

}

Enabled::~Enabled()
{
  TransitionRecord tr( stateName(), false );
  outermost_context().updateHistory( tr );

  SharedResourcesPtr sharedResources =
    outermost_context().getSharedResources();

  // Disable consumer registration:
  sharedResources->_registrationCollection->disableConsumerRegistration();

  // Clear any fragments left in the fragment store
  outermost_context().getFragmentStore()->clear();

  // DQM end-run processing
  if ( sharedResources->_dqmServiceManager.get() != 0 )
    {
      sharedResources->_dqmServiceManager->stop();
    }

  // request that the streams that are currently configured in the disk
  // writer be destroyed (this has the side effect of closing files)
  sharedResources->_diskWriterResources->requestStreamDestruction();

  // request that the DQM event store is cleared
  // if draining queues has succeeded, the store is already empty
  sharedResources->_dqmEventProcessorResources->requestStoreDestruction();

  // wait for the requests to be fulfilled
  sharedResources->_diskWriterResources->waitForStreamDestruction();
  sharedResources->_dqmEventProcessorResources->waitForStoreDestruction();

  // clear the stream selections in the event distributor
  outermost_context().getEventDistributor()->clearStreams();

}

string Enabled::do_stateName() const
{
  return string( "Enabled" );
}

void Enabled::logReconfigureRequest( const Reconfigure& request )
{
  outermost_context().unconsumed_event( request );
}

/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
