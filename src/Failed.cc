#include "EventFilter/StorageManager/interface/StateMachine.h"

#include <iostream>

using namespace std;
using namespace stor;

Failed::Failed( my_context c ): my_base(c)
{
  cout << "Entering " << stateName() << " state" << endl;
  TransitionRecord tr( stateName(), true );
  outermost_context().updateHistory( tr );
}

Failed::~Failed()
{
  cout << "Exiting " << stateName() << " state" << endl;
  TransitionRecord tr( stateName(), false );
  outermost_context().updateHistory( tr );
}

string Failed::do_stateName() const
{
  return string( "Failed" );
}

void Failed::logFailRequest( const Fail& request )
{
  do_logInvalidEvent( "Fail", outermost_context().getCurrentStateName() );
}

void Failed::logHaltRequest( const Halt& request )
{
  do_logInvalidEvent( "Halt", outermost_context().getCurrentStateName() );
}

void Failed::logConfigureRequest( const Configure& request )
{
  do_logInvalidEvent( "Configure", outermost_context().getCurrentStateName() );
}

void Failed::logReconfigureRequest( const Reconfigure& request )
{
  do_logInvalidEvent( "Reconfigure", outermost_context().getCurrentStateName() );
}

void Failed::logEnableRequest( const Enable& request )
{
  do_logInvalidEvent( "Enable", outermost_context().getCurrentStateName() );
}

void Failed::logStopRequest( const Stop& request )
{
  do_logInvalidEvent( "Stop", outermost_context().getCurrentStateName() );
}

void Failed::logStopDoneRequest( const StopDone& request )
{
  do_logInvalidEvent( "StopDone", outermost_context().getCurrentStateName() );
}

void Failed::logEmergencyStopRequest( const EmergencyStop& request )
{
  do_logInvalidEvent( "EmergencyStop", outermost_context().getCurrentStateName() );
}
