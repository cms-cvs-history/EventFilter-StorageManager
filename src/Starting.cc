// $Id: Enabled.cc,v 1.1.2.36 2009/05/05 10:40:39 mommsen Exp $

#include "EventFilter/StorageManager/interface/CommandQueue.h"
#include "EventFilter/StorageManager/interface/Configuration.h"
#include "EventFilter/StorageManager/interface/ErrorStreamConfigurationInfo.h"
#include "EventFilter/StorageManager/interface/EventStreamConfigurationInfo.h"
#include "EventFilter/StorageManager/interface/SharedResources.h"
#include "EventFilter/StorageManager/interface/StateMachine.h"

#include <iostream>
#include <unistd.h>

using namespace std;
using namespace stor;

Starting::Starting( my_context c ): my_base(c)
{
  TransitionRecord tr( stateName(), true );
  outermost_context().updateHistory( tr );

  SharedResourcesPtr sharedResources =
    outermost_context().getSharedResources();

  // Request DiskWriter to configure streams
  EvtStrConfigListPtr evtCfgList = sharedResources->_configuration->
    getCurrentEventStreamConfig();
  ErrStrConfigListPtr errCfgList = sharedResources->_configuration->
    getCurrentErrorStreamConfig();

  WorkerThreadParams workerParams =
    sharedResources->_configuration->getWorkerThreadParams();
  sharedResources->_diskWriterResources->
    requestStreamConfiguration(evtCfgList, errCfgList,
                               workerParams._DWdeqWaitTime);

  // Request configuration of DQMEventProcessor
  sharedResources->_dqmEventProcessorResources->
    requestConfiguration(
      sharedResources->_configuration->getDQMProcessingParams(),
      workerParams._DQMEPdeqWaitTime);
}

Starting::~Starting()
{
  TransitionRecord tr( stateName(), false );
  outermost_context().updateHistory( tr );
}

string Starting::do_stateName() const
{
  return string( "Starting" );
}

void Starting::logStopDoneRequest( const StopDone& request )
{
  outermost_context().unconsumed_event( request );
}

void Starting::logHaltDoneRequest( const HaltDone& request )
{
  outermost_context().unconsumed_event( request );
}

void
Starting::do_noFragmentToProcess() const
{
  if ( workerThreadsConfigured() )
  {
    SharedResourcesPtr sharedResources =
      outermost_context().getSharedResources();
    event_ptr stMachEvent( new StartRun() );
    sharedResources->_commandQueue->enq_wait( stMachEvent );
  }
}

bool
Starting::workerThreadsConfigured() const
{
  SharedResourcesPtr sharedResources =
    outermost_context().getSharedResources();

  sharedResources->_diskWriterResources->waitForStreamConfiguration();
  sharedResources->_dqmEventProcessorResources->waitForConfiguration();

  return true; 
}


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
