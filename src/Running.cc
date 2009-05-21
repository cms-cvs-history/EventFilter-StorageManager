// $Id: Enabled.cc,v 1.1.2.36 2009/05/05 10:40:39 mommsen Exp $

#include "EventFilter/StorageManager/interface/ErrorStreamConfigurationInfo.h"
#include "EventFilter/StorageManager/interface/EventStreamConfigurationInfo.h"
#include "EventFilter/StorageManager/interface/EventDistributor.h"
#include "EventFilter/StorageManager/interface/FragmentStore.h"
#include "EventFilter/StorageManager/interface/SharedResources.h"
#include "EventFilter/StorageManager/interface/StateMachine.h"

#include <iostream>
#include <unistd.h>

using namespace std;
using namespace stor;

Running::Running( my_context c ): my_base(c)
{
  TransitionRecord tr( stateName(), true );
  outermost_context().updateHistory( tr );

  SharedResourcesPtr sharedResources =
    outermost_context().getSharedResources();

  // Configure event distributor
  EventDistributor* ed = outermost_context().getEventDistributor();
  EvtStrConfigListPtr evtCfgList = sharedResources->_configuration->
    getCurrentEventStreamConfig();
  ErrStrConfigListPtr errCfgList = sharedResources->_configuration->
    getCurrentErrorStreamConfig();
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

Running::~Running()
{
  TransitionRecord tr( stateName(), false );
  outermost_context().updateHistory( tr );

  SharedResourcesPtr sharedResources =
    outermost_context().getSharedResources();

  // Disable consumer registration:
  sharedResources->_registrationCollection->disableConsumerRegistration();

  // Clear consumer queues
  sharedResources->_eventConsumerQueueCollection->clearQueues();
  sharedResources->_dqmEventConsumerQueueCollection->clearQueues();

  // Clear the queues
  sharedResources->_fragmentQueue->clear();
  sharedResources->_streamQueue->clear();
  sharedResources->_dqmEventQueue->clear();
  sharedResources->_registrationQueue->clear();

  // Clear any fragments left in the fragment store
  outermost_context().getFragmentStore()->clear();

}

string Running::do_stateName() const
{
  return string( "Running" );
}

void Running::logStopDoneRequest( const StopDone& request )
{
  outermost_context().unconsumed_event( request );
}

void Running::logHaltDoneRequest( const HaltDone& request )
{
  outermost_context().unconsumed_event( request );
}


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -