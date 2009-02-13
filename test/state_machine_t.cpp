#include <iostream>
#include <map>
#include <list>

#include <boost/statechart/event_base.hpp>
#include <boost/shared_ptr.hpp>

#include "EventFilter/StorageManager/interface/StateMachine.h"

using namespace std;
using namespace boost::statechart;
using namespace stor;

using boost::shared_ptr;

// Typedefs:
typedef map< string, shared_ptr<event_base> > EventMap;
typedef list<string> EventList;

//////////////////////////////////////////////////////
//// Exit with non-zero code if unexpected state: ////
//////////////////////////////////////////////////////
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

/////////////////////////////////////////////////////////
//// Check if every signal on the list gets ignored: ////
/////////////////////////////////////////////////////////
void checkSignals( StateMachine& m,
		   const EventList& elist,
		   const string& expected )
{

  EventMap emap;
  emap[ "Configure" ] = shared_ptr<event_base>( new Configure() );
  emap[ "Enable" ] = shared_ptr<event_base>( new Enable() );
  emap[ "Stop" ] = shared_ptr<event_base>( new Stop() );
  emap[ "Halt" ] = shared_ptr<event_base>( new Halt() );
  emap[ "Reconfigure" ] = shared_ptr<event_base>( new Reconfigure() );
  emap[ "EmergencyStop" ] = shared_ptr<event_base>( new EmergencyStop() );
  emap[ "StopDone" ] = shared_ptr<event_base>( new StopDone() );
  emap[ "Fail" ] = shared_ptr<event_base>( new Fail() );

  for( EventList::const_iterator i = elist.begin(); i != elist.end(); ++i )
    {

      EventMap::const_iterator j = emap.find( *i );
      if( j == emap.end() )
	{
	  cerr << "Cannot find event " << *i << " in event map" << endl;
	  exit(2);
	}
      m.process_event( *(j->second) );
      checkState( m, expected );
    }

}

///////////////
//// Main: ////
///////////////
int main()
{

  StateMachine m( 0, 0, 0 );

  EventList elist;

  //
  //// Halted: ////
  //

  m.initiate();
  checkState( m, "Halted" );

  elist.clear();
  elist.push_back( "Stop" );
  elist.push_back( "Halt" );
  elist.push_back( "EmergencyStop" );
  elist.push_back( "StopDone" );

  checkSignals( m, elist, "Halted" );

  //
  //// Stopped: ////
  //

  m.process_event( Configure() );
  checkState( m, "Stopped" );

  elist.clear();
  elist.push_back( "Stop" );
  elist.push_back( "EmergencyStop" );
  elist.push_back( "StopDone" );

  checkSignals( m, elist, "Stopped" );

  //
  //// Processing: ////
  //

  m.process_event( Enable() );
  checkState( m, "Processing" );

  elist.clear();
  elist.push_back( "StopDone" );
  elist.push_back( "Enable" );
  elist.push_back( "Configure" );
  elist.push_back( "Reconfigure" );

  checkSignals( m, elist, "Processing" );

  //
  //// Check Reconfigure from stopped: ////
  //

  m.clearHistory();
  m.process_event( Stop() );
  checkState( m, "Stopped" );
  m.process_event( Reconfigure() );
  checkState( m, "Stopped" );
  m.dumpHistory( cout );

  //
  //// Failed: ////
  //

  m.process_event( Fail() );
  checkState( m, "Failed" );

  elist.clear();
  elist.push_back( "StopDone" );
  elist.push_back( "Enable" );
  elist.push_back( "Configure" );
  elist.push_back( "Reconfigure" );
  elist.push_back( "Stop" );
  elist.push_back( "EmergencyStop" );
  elist.push_back( "Halt" );
  elist.push_back( "Fail" );

  checkSignals( m, elist, "Failed" );

  m.terminate();

}
