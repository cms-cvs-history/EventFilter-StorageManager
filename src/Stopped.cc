#include "EventFilter/StorageManager/interface/StateMachine.h"

#include <iostream>

using namespace std;
using namespace stor;

Stopped::Stopped( my_context c ): my_base(c)
{
  TransitionRecord tr( stateName(), true );
  outermost_context().updateHistory( tr );
  outermost_context().setExternallyVisibleState( "Ready" );
  outermost_context().getNotifier()->reportNewState( "Ready" );
}

Stopped::~Stopped()
{
  TransitionRecord tr( stateName(), false );
  outermost_context().updateHistory( tr );
}

string Stopped::do_stateName() const
{
  return string( "Stopped" );
}

// void Stopped::handleI2OEventMessage() const
// {
//   cerr << "Error: " << stateName() << " state cannot handle I2O messages" << endl;
// }

/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
