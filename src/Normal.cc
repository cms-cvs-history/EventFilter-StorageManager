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

void Normal::logHaltRequest( const Halt& request )
{
  do_logInvalidEvent( "Halt", outermost_context().getCurrentStateName() );
}

void Normal::logConfigureRequest( const Configure& request )
{
  do_logInvalidEvent( "Configure", outermost_context().getCurrentStateName() );
}

void Normal::logReconfigureRequest( const Reconfigure& request )
{
  do_logInvalidEvent( "Reconfigure", outermost_context().getCurrentStateName() );
}

void Normal::logEnableRequest( const Enable& request )
{
  do_logInvalidEvent( "Enable", outermost_context().getCurrentStateName() );
}

void Normal::logStopRequest( const Stop& request )
{
  do_logInvalidEvent( "Stop", outermost_context().getCurrentStateName() );
}

void Normal::logStopDoneRequest( const StopDone& request )
{
  do_logInvalidEvent( "StopDone", outermost_context().getCurrentStateName() );
}

void Normal::logEmergencyStopRequest( const EmergencyStop& request )
{
  do_logInvalidEvent( "EmergencyStop", outermost_context().getCurrentStateName() );
}
