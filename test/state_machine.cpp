#include <iostream>
#include <string>
#include <vector>

#include "EventFilter/StorageManager/interface/StateMachine.h"

using namespace std;

int main()
{

  StateMachine machine;
  machine.initiate();

  // This should be replaced by a map from string to event

  vector<string> transitions;
  transitions.push_back( "Configure" );
  transitions.push_back( "Enable" );
  transitions.push_back( "Stop" );
  transitions.push_back( "Halt" );

  for( unsigned int j = 0; j < 3; ++j )
    {

      for( unsigned int i = 0; i < 4; ++i )
	{

	  string t = transitions[i];

	  if( j == 1 && i == 2 ) continue;

	  if( t == "Configure" )
	    {
	      machine.process_event( Configure() );
	    }
	  else if( t == "Enable" )
	    {
	      machine.process_event( Enable() );
	    }
	  else if( t == "Stop" )
	    {
	      machine.process_event( Stop() );
	    }
	  else if( t == "Halt" )
	    {
	      machine.process_event( Halt() );
	    }

	  machine.handleI2OEventMessage();

	}

    }

  return 0;

}
