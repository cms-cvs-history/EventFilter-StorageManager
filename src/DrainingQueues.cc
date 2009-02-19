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

  _doDraining = false;
  post_event( StopDone() );
}

DrainingQueues::~DrainingQueues()
{
  TransitionRecord tr( stateName(), false );
  outermost_context().updateHistory( tr );
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
DrainingQueues::do_noFragmentToProcess( EventDistributor& ed,
                                        FragmentStore& fs ) const
{
  I2OChain staleEvent;
  bool gotStaleEvent = true;  

  while ( gotStaleEvent && _doDraining )
  {
    if ( ed.full() )
    {
      // The event distributor cannot accept any new events.
      // Wait a bit then start over
      ::usleep(1000); // TODO: make the timeout configurable
    }
    else
    {
      gotStaleEvent = fs.getStaleEvent(staleEvent, 0);
      if ( gotStaleEvent )
      {
        ed.addEventToRelevantQueues(staleEvent);
      }
    }
  }
  // Make sure we clear the fragment store if we were interrupted
  fs.clear();
}


void DrainingQueues::emergencyStop(const EmergencyStop &event)
{
  _doDraining = false;
}

/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
