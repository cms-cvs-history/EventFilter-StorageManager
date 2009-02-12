#include "EventFilter/StorageManager/interface/StateMachine.h"

#include <iostream>

using namespace std;
using namespace stor;

Enabled::Enabled( my_context c ): my_base(c)
{
  cout << "Entering " << stateName() << " state" << endl;
  TransitionRecord tr( stateName(), true );
  outermost_context().updateHistory( tr );
}

Enabled::~Enabled()
{
  cout << "Clearing queues, closing files, and exiting " << stateName() << " state" << endl;
  TransitionRecord tr( stateName(), false );
  outermost_context().updateHistory( tr );
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
