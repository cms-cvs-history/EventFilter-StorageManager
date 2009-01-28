#include <iostream>
#include <string>
#include <vector>

#include "EventFilter/StorageManager/interface/StateMachine.h"

using namespace std;
using namespace stor;

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

	  std::cout << "### Requesting " << t << " transition." << std::endl;

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

          // Test if we need to send a StopDone event.  In the real
          // system this will be done by a thread that waits until
          // the queues are empty and then posts this event.
//           if( machine.getCurrentStateName() == "DrainingQueues" )
//           {
//               machine.process_event( StopDone() );
//           }

	  // In the current design, the StateMachine object does not
	  // process I2O messages.

	  //	  machine.handleI2OEventMessage();


          sleep(2);
	}

    }

  return 0;

}
