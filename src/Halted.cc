#include "EventFilter/StorageManager/interface/Notifier.h"
#include "EventFilter/StorageManager/interface/StateMachine.h"

#include <iostream>

using namespace std;
using namespace stor;

Halted::Halted( my_context c ): my_base(c)
{
  TransitionRecord tr( stateName(), true );
  outermost_context().updateHistory( tr );
  outermost_context().declareInitialized();
}

Halted::~Halted()
{
  TransitionRecord tr( stateName(), false );
  outermost_context().updateHistory( tr );
  outermost_context().setExternallyVisibleState( "Halted" );
  outermost_context().getNotifier()->reportNewState( "Halted" );
}

string Halted::do_stateName() const
{
  return string( "Halted" );
}

// void Halted::handleI2OEventMessage() const
// {
//   cerr << "Error: " << stateName() << " state cannot handle I2O messages" << endl;
// }

/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
