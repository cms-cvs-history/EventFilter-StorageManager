#include "EventFilter/StorageManager/interface/StateMachine.h"

#include <iostream>

using namespace std;
using namespace stor;

Normal::Normal( my_context c ): my_base(c)
{
  cout << "Entering " << stateName() << " state" << endl;
  TransitionRecord tr( stateName(), true );
  outermost_context().updateHistory( tr );
}

Normal::~Normal()
{
  cout << "Exiting " << stateName() << " state" << endl;
  TransitionRecord tr( stateName(), false );
  outermost_context().updateHistory( tr );
}

string Normal::do_stateName() const
{
  return string( "Normal" );
}
