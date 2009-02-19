#include "EventFilter/StorageManager/interface/EventDistributor.h"
#include "EventFilter/StorageManager/interface/FragmentStore.h"
#include "EventFilter/StorageManager/interface/I2OChain.h"
#include "EventFilter/StorageManager/interface/StateMachine.h"

#include <iostream>
#include <unistd.h>

using namespace std;
using namespace stor;

DrainingQueues::DrainingQueues( my_context c ): my_base(c),
_doDraining(true)
{
  TransitionRecord tr( stateName(), true );
  outermost_context().updateHistory( tr );

  post_event( StopDone() );
}

DrainingQueues::~DrainingQueues()
{
  TransitionRecord tr( stateName(), false );
  outermost_context().updateHistory( tr );

  _doDraining = false;
}

string DrainingQueues::do_stateName() const
{
  return string( "DrainingQueues" );
}

// void DrainingQueues::handleI2OEventMessage() const
// {
//   cerr << "Error: " << stateName() << " state cannot handle I2O messages" << endl;
// }


void
DrainingQueues::do_noFragmentToProcess() const
{
  I2OChain staleEvent;
  bool gotStaleEvent = true;  

  EventDistributor *ed = outermost_context().getEventDistributor();

  while ( gotStaleEvent && _doDraining && !ed->full() )
  {
    gotStaleEvent = 
      outermost_context().getFragmentStore()->getStaleEvent(staleEvent, 0);
    if ( gotStaleEvent )
    {
      ed->addEventToRelevantQueues(staleEvent);
    }
  }
}


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
