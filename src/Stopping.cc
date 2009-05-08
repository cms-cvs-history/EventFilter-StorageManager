// $Id: Stopping.cc,v 1.1.2.1 2009/05/05 20:13:25 mommsen Exp $

#include "EventFilter/StorageManager/interface/CommandQueue.h"
#include "EventFilter/StorageManager/interface/SharedResources.h"
#include "EventFilter/StorageManager/interface/StateMachine.h"

#include <iostream>
#include <unistd.h>

using namespace std;
using namespace stor;

Stopping::Stopping( my_context c ): my_base(c)
{
  TransitionRecord tr( stateName(), true );
  outermost_context().updateHistory( tr );

  SharedResourcesPtr sharedResources =
    outermost_context().getSharedResources();

  // request that the streams that are currently configured in the disk
  // writer be destroyed (this has the side effect of closing files)
  sharedResources->_diskWriterResources->requestStreamDestruction();

  // request that the DQM event store is cleared
  // if FinishingDQM has succeeded, the store is already empty
  sharedResources->_dqmEventProcessorResources->requestStoreDestruction();
}

Stopping::~Stopping()
{
  TransitionRecord tr( stateName(), false );
  outermost_context().updateHistory( tr );
}

string Stopping::do_stateName() const
{
  return string( "Stopping" );
}

void Stopping::logHaltDoneRequest( const HaltDone& request )
{
  outermost_context().unconsumed_event( request );
}

void
Stopping::do_noFragmentToProcess() const
{
  if ( destructionIsDone() )
  {
    SharedResourcesPtr sharedResources =
      outermost_context().getSharedResources();
    event_ptr stMachEvent( new StopDone() );
    sharedResources->_commandQueue->enq_wait( stMachEvent );
  }
}

bool
Stopping::destructionIsDone() const
{
  SharedResourcesPtr sharedResources =
    outermost_context().getSharedResources();

  // check if the requests are still being processed
  if ( sharedResources->_diskWriterResources->streamChangeOngoing() ) return false;

  // wait for the requests to be fulfilled
  sharedResources->_dqmEventProcessorResources->waitForStoreDestruction();

  return true;
}


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
