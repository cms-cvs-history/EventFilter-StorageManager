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

  typedef map< string, shared_ptr<event_base> > EventMap;
  EventMap emap;
  emap[ "Configure" ] = shared_ptr<event_base>( new Configure() );
  emap[ "Enable" ] = shared_ptr<event_base>( new Enable() );
  emap[ "Stop" ] = shared_ptr<event_base>( new Stop() );
  emap[ "Halt" ] = shared_ptr<event_base>( new Halt() );
  emap[ "Reconfigure" ] = shared_ptr<event_base>( new Reconfigure() );
  emap[ "EmergencyStop" ] = shared_ptr<event_base>( new EmergencyStop() );
  emap[ "StopDone" ] = shared_ptr<event_base>( new StopDone() );
  emap[ "Fail" ] = shared_ptr<event_base>( new Fail() );

  StateMachine m( 0, 0, 0 );

  // Make sure it starts up in Halted and bring it up to Processing:
  m.initiate();
  checkState( m, "Halted" );
  m.process_event( Configure() );
  checkState( m, "Stopped" );
  m.process_event( Enable() );
  checkState( m, "Processing" );

  // Stop and reconfigure:
  m.clearHistory();
  m.process_event( Stop() );
  checkState( m, "Stopped" );
  //m.process_event( Reconfigure() );
  //checkState( m, "Stopped" );
  m.dumpHistory( cout );

  m.terminate();

}