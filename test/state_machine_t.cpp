#include <iostream>
#include <map>
#include <list>
#include <vector>

#include <boost/statechart/event_base.hpp>
#include <boost/shared_ptr.hpp>

#include "EventFilter/StorageManager/interface/CommandQueue.h"
#include "EventFilter/StorageManager/interface/DiskWriter.h"
#include "EventFilter/StorageManager/interface/EventDistributor.h"
#include "EventFilter/StorageManager/interface/FragmentStore.h"
#include "EventFilter/StorageManager/interface/SharedResources.h"
#include "EventFilter/StorageManager/interface/StateMachine.h"

using namespace std;
using namespace boost::statechart;
using namespace stor;

using boost::shared_ptr;

// Typedefs:
typedef map< string, shared_ptr<event_base> > EventMap;
typedef list<string> EventList;
typedef vector<TransitionRecord> TransitionList;

/////////////////////////////////////////////////////////////////////
//// Simulate the processing of events by the fragment processor ////
/////////////////////////////////////////////////////////////////////
void processEvent(  StateMachine& machine,
                    stor::event_ptr requestedEvent )
{
  boost::shared_ptr<CommandQueue> cmdQueue;
  cmdQueue = machine.getSharedResources()->_commandQueue;

  cmdQueue->enq_wait( requestedEvent );

  stor::event_ptr nextEvent;
  while ( !cmdQueue->empty() )
    {
      bool gotEvent = cmdQueue->deq_nowait( nextEvent );
      if ( gotEvent )
        {
          machine.process_event( *nextEvent );
          machine.getCurrentState().noFragmentToProcess();
        }
    }
}

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
      processEvent( m, j->second );
      checkState( m, expected );
    }

}

////////////////////////////////////////////////////////////////////
//// Check if history matches expected sequence of transitions: ////
////////////////////////////////////////////////////////////////////
void checkHistory( const StateMachine& m,
		   const TransitionList& steps )
{

  const StateMachine::History h = m.history();
  const unsigned int hsize = h.size();
  const unsigned int ssize = steps.size();

  bool ok = true;

  if( ssize != hsize )
    {
      ok = false;
    }

  if( ok )
    {
      for( unsigned int i = 0; i < hsize; ++i )
	{
	  if( h[i].isEntry() != steps[i].isEntry() ||
	      h[i].stateName() != steps[i].stateName() )
	    {
	      ok = false;
	      break;
	    }
	}
    }

  if( !ok )
    {
      cerr << "**** History mismatch ****" << endl;
      cerr << "Actual:" << endl;
      m.dumpHistory( cerr );
      cerr << "Expected:" << endl;
      for( unsigned int j = 0; j < ssize; ++j )
	{
	  if( steps[j].isEntry() )
	    {
	      cerr << " Enter ";
	    }
	  else
	    {
	      cerr << " Exit ";
	    }
	  cerr << steps[j].stateName() << endl;
	}
      exit(3);
    }

}

///////////////
//// Main: ////
///////////////
int main()
{

  DiskWriter dw;
  FragmentStore fs;
  boost::shared_ptr<SharedResources> sr;
  sr.reset(new SharedResources());

  boost::shared_ptr<CommandQueue> cmdQueue(new CommandQueue(32));
  sr->_commandQueue = cmdQueue;

  EventDistributor ed(sr);

  StateMachine m( &dw, &ed, &fs, &(*sr) );

  EventList elist;

  stor::event_ptr stMachEvent;

  //
  //// Halted: ////
  //

  m.initiate();
  checkState( m, "Halted" );

  cout << "**** Testing illegal signals in Halted state ****" << endl;

  elist.clear();
  elist.push_back( "Stop" );
  elist.push_back( "Halt" );
  elist.push_back( "EmergencyStop" );
  elist.push_back( "StopDone" );

  checkSignals( m, elist, "Halted" );

  //
  //// Stopped: ////
  //

  stMachEvent.reset( new Configure() );
  processEvent( m, stMachEvent );
  checkState( m, "Stopped" );

  cout << "**** Testing illegal signals in Stopped state ****" << endl;

  elist.clear();
  elist.push_back( "Stop" );
  elist.push_back( "EmergencyStop" );
  elist.push_back( "StopDone" );

  checkSignals( m, elist, "Stopped" );

  //
  //// Processing: ////
  //

  stMachEvent.reset( new Enable() );
  processEvent( m, stMachEvent );
  checkState( m, "Processing" );

  cout << "**** Testing illegal signals in Processing state ****" << endl;

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
  stMachEvent.reset( new Stop() );
  processEvent( m, stMachEvent );
  checkState( m, "Stopped" );

  cout << "**** Testing if DrainingQueues is entered and exited on Stop ****" << endl;

  TransitionList steps;
  steps.push_back( TransitionRecord( "Processing", false ) );
  steps.push_back( TransitionRecord( "DrainingQueues", true ) );
  steps.push_back( TransitionRecord( "DrainingQueues", false ) );
  steps.push_back( TransitionRecord( "Enabled", false ) );
  steps.push_back( TransitionRecord( "Stopped", true ) );
  checkHistory( m, steps );

  //
  //// Check Reconfigure: ////
  //

  m.clearHistory();
  stMachEvent.reset( new Reconfigure() );
  processEvent( m, stMachEvent );
  checkState( m, "Stopped" );

  cout << "**** Testing if Reconfigure triggers the right sequence ****" << endl;

  steps.clear();
  steps.push_back( TransitionRecord( "Stopped", false ) );
  steps.push_back( TransitionRecord( "Ready", false ) );
  steps.push_back( TransitionRecord( "Ready", true ) );
  steps.push_back( TransitionRecord( "Stopped", true ) );
  checkHistory( m, steps );

  //
  //// Failed: ////
  //

  stMachEvent.reset( new Fail() );
  processEvent( m, stMachEvent );
  checkState( m, "Failed" );

  cout << "**** Making sure no signal changes Failed state ****" << endl;

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

  //
  //// Make sure each state can go to Failed: ////
  //

  cout << "**** Making sure Halted can go to Failed ****" << endl;

  // Halted:
  m.initiate();
  checkState( m, "Halted" );
  stMachEvent.reset( new Fail() );
  processEvent( m, stMachEvent );
  checkState( m, "Failed" );
  m.terminate();

  cout << "**** Making sure Stopped can go to Failed ****" << endl;

  // Stopped:
  m.initiate();
  checkState( m, "Halted" );
  stMachEvent.reset( new Configure() );
  processEvent( m, stMachEvent );
  checkState( m, "Stopped" );
  stMachEvent.reset( new Fail() );
  processEvent( m, stMachEvent );
  checkState( m, "Failed" );
  m.terminate();
 
  cout << "**** Making sure Processing can go to Failed ****" << endl;

  // Processing:
  m.initiate();
  checkState( m, "Halted" );
  stMachEvent.reset( new Configure() );
  processEvent( m, stMachEvent );
  checkState( m, "Stopped" );
  stMachEvent.reset( new Enable() );
  processEvent( m, stMachEvent );
  checkState( m, "Processing" );
  stMachEvent.reset( new Fail() );
  processEvent( m, stMachEvent );
  checkState( m, "Failed" );
  m.terminate();

}
