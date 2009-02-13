#include <iostream>
#include <map>
#include <list>
#include <vector>

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
typedef vector<string> TransitionList;

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

////////////////////////////////////////////////////////////////////
//// Check if history matches expected sequence of transitions: ////
////////////////////////////////////////////////////////////////////
void checkHistory( const StateMachine& m,
		   const string& from_state,
		   const string& to_state,
		   const TransitionList& steps )
{

  const StateMachine::History h = m.history();
  const unsigned int hsize = h.size();
  const unsigned int ssize = steps.size();

  bool ok = true;

  if( ssize * 2 + 2 != hsize )
    {
      cout << "Debug 1" << endl;
      cout << ssize << " " << hsize << endl;
      ok = false;
    }

  if( ok )
    {
      if( h[0].stateName() != from_state )
	{
	  cout << "Debug 2" << endl;
	  ok = false;
	}
    }
 
  if( ok )
    {

      unsigned int i = 0;

      while( i < ssize )
	{

	  if( h[ 2*i + 1 ].stateName() != steps[i] )
	    {
	      cout << "Debug 3" << endl;
	      ok = false;
	      break;
	    }

	  if( h[ 2*i + 2 ].stateName() != steps[i] )
	    {
	      cout << "Debug 4" << endl;
	      ok = false;
	      break;
	    }

	  ++i;

	}

    }

  if( ok )
    {
      if( h[ hsize - 1 ].stateName() != to_state )
	{
	  cout << "Debug 5" << endl;
	  ok = false;
	}
    }

  if( !ok )
    {
      cerr << "**** History mismatch ****" << endl;
      cerr << "Actual:" << endl;
      m.dumpHistory( cerr );
      cerr << "Expected:" << endl;
      cerr << " from " << from_state;
      for( unsigned int j = 0; j < ssize; ++j )
	{
	  cerr << " to " << steps[j];
	}
      cerr << " to " << to_state << endl;
      exit(3);
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
  //// Make sure it goes through DrainingQueues: ////
  //

  m.clearHistory();
  m.process_event( Stop() );
  checkState( m, "Stopped" );

  TransitionList steps;
  steps.push_back( "DrainingQueues" );
  steps.push_back( "Enabled" );
  checkHistory( m, "Processing", "Stopped", steps );

  //
  //// Check Reconfigure: ////
  //

  m.clearHistory();
  m.process_event( Reconfigure() );
  checkState( m, "Stopped" );

  steps.clear();
  steps.push_back( "Ready" );
  checkHistory( m, "Stopped", "Stopped", steps );

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
