// $Id: Enabled.cc,v 1.1.2.36 2009/05/05 10:40:39 mommsen Exp $

#include "EventFilter/StorageManager/interface/CommandQueue.h"
#include "EventFilter/StorageManager/interface/SharedResources.h"
#include "EventFilter/StorageManager/interface/StateMachine.h"

#include <iostream>
#include <unistd.h>

using namespace std;
using namespace stor;

FinishingDQM::FinishingDQM( my_context c ): my_base(c)
{
  TransitionRecord tr( stateName(), true );
  outermost_context().updateHistory( tr );

  SharedResourcesPtr sharedResources =
    outermost_context().getSharedResources();

  // request end-of-run processing in DQMEventProcessor
  sharedResources->_dqmEventProcessorResources->requestEndOfRun();
}

FinishingDQM::~FinishingDQM()
{
  TransitionRecord tr( stateName(), false );
  outermost_context().updateHistory( tr );
}

string FinishingDQM::do_stateName() const
{
  return string( "FinishingDQM" );
}

void FinishingDQM::logStopRequest( const Stop& request )
{
  outermost_context().unconsumed_event( request );
}

void FinishingDQM::logQueuesEmptyRequest( const QueuesEmpty& request )
{
  outermost_context().unconsumed_event( request );
}

void
FinishingDQM::do_noFragmentToProcess() const
{
  if ( endOfRunProcessingIsDone() )
  {
    SharedResourcesPtr sharedResources =
      outermost_context().getSharedResources();
    event_ptr stMachEvent( new EndRun() );
    sharedResources->_commandQueue->enq_wait( stMachEvent );
  }
}

bool
FinishingDQM::endOfRunProcessingIsDone() const
{
  SharedResourcesPtr sharedResources =
    outermost_context().getSharedResources();

  if ( sharedResources->_dqmEventProcessorResources->isEndOfRunDone() ) 
    return true;
  else
    return false;
}

/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
