#include "EventFilter/StorageManager/interface/FragmentStore.h"
#include "EventFilter/StorageManager/interface/StateMachine.h"

#include <iostream>

using namespace std;
using namespace stor;

Enabled::Enabled( my_context c ): my_base(c)
{
  TransitionRecord tr( stateName(), true );
  outermost_context().updateHistory( tr );
}

Enabled::~Enabled()
{
  TransitionRecord tr( stateName(), false );
  outermost_context().updateHistory( tr );

  // Clear any fragments left in the fragment store
  outermost_context().getFragmentStore()->clear();
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
