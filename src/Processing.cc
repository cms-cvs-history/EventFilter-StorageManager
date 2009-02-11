#include "EventFilter/StorageManager/interface/StateMachine.h"

#include <iostream>

using namespace std;
using namespace stor;

unsigned int Processing::_counter = 0;

Processing::Processing( my_context c ): my_base(c)
{

  ++_counter;
  if( _counter > 4 )
    {
      cerr << "Error: " << stateName() << " state created too many times" << endl;
      post_event( Fail() );
      return;
    }

  cout << "Entering " << stateName() << " state, counter = " << _counter << endl;
  TransitionRecord tr( stateName(), true );
  outermost_context().updateHistory( tr );

}

Processing::~Processing()
{
  cout << "Exiting " << stateName() << " state" << endl;
  TransitionRecord tr( stateName(), false );
  outermost_context().updateHistory( tr );
}

string Processing::do_stateName() const
{
  return string( "Processing" );
}

void Processing::logStopDoneRequest( const StopDone& request )
{
  do_logInvalidEvent( "StopDone", outermost_context().getCurrentStateName() );
}

void
Processing::do_processI2OFragment( I2OChain& frag,
				   EventDistributor& ed,
				   FragmentStore& fs ) const
{
  cout << "Processing a fragment\n";
}
