#include <iostream>

#include <boost/statechart/event_base.hpp>
#include <boost/shared_ptr.hpp>

#include "EventFilter/StorageManager/interface/StateMachine.h"

using namespace std;
using namespace boost::statechart;
using namespace stor;

using boost::shared_ptr;

void checkState( const StateMachine& machine,
		 const std::string& expected )
{
  const string actual = machine.getCurrentState().stateName();
  if( actual != expected )
    {
      cerr << "Expecting " << expected << ", got " << actual << endl;
      machine.dumpHistory( cerr );
      exit(1);
    }
}

int main()
{

  StateMachine m( 0, 0, 0 );

  // Make sure it starts up in Halted:
  m.initiate();
  checkState( m, "Halted" );

  // Configure:
  m.process_event( Configure() );
  checkState( m, "Stopped" );

  // Bring it up:
  //m.process_event( Enable() );
  //checkState( m, "Processing" );

  // Stop and reconfigure:
  m.clearHistory();
  //m.process_event( Stop() );
  //checkState( m, "Stopped" );
  m.process_event( Reconfigure() );
  checkState( m, "Stopped" );
  m.dumpHistory( cout );

  // Try invalid transitions:
  m.clearHistory();
  m.process_event( Fail() );
  checkState( m, "Failed" );
  m.process_event( Halt() );
  checkState( m, "Failed" );
  m.dumpHistory( cout );

  m.terminate();

}
