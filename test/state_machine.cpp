#include <iostream>
#include <string>
#include <map>

#include <boost/shared_ptr.hpp>
#include <boost/statechart/event_base.hpp>

#include "EventFilter/StorageManager/interface/StateMachine.h"
#include "EventFilter/StorageManager/interface/I2OChain.h"
#include "EventFilter/StorageManager/interface/DiskWriter.h"
#include "EventFilter/StorageManager/interface/EventDistributor.h"
#include "EventFilter/StorageManager/interface/FragmentProcessor.h"
#include "EventFilter/StorageManager/interface/FragmentStore.h"

using namespace boost::statechart;
using namespace stor;
using namespace std;

using boost::shared_ptr;

typedef map< string, shared_ptr<event_base> > EventMap;

/////////////////////////////////////////////////////////
//// Call process_event and processI2OFragment once: ////
/////////////////////////////////////////////////////////
void onePass( StateMachine& machine, const EventMap::const_iterator& it )
{

  cout << "### Requesting " << it->first << " in "
       << machine.getCurrentState().stateName() << ":" << endl;

  machine.process_event( *(it->second) );

  cout << "### Arriving at " << machine.getCurrentState().stateName() << endl;

  I2OChain i2oc;
  machine.getCurrentState().processI2OFragment( i2oc );

}

///////////////
//// Main: ////
///////////////
int main()
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

  StateMachine machine( 0, 0, 0, 0 );  // will need real worker objects soon

  for( EventMap::const_iterator it = emap.begin(); it != emap.end(); ++it )
    {

      machine.initiate(); // expect Halted
      onePass( machine, it );

      machine.initiate();
      machine.process_event( *(emap[ "Configure" ]) ); // expect Configured
      onePass( machine, it );

      machine.initiate();
      machine.process_event( *(emap[ "Configure" ]) );
      machine.process_event( *(emap[ "Enable" ]) ); // expect Processing
      onePass( machine, it );

    }

  machine.dumpHistory( cout );

  // explicitly terminate the state machine before exiting so that we don't
  // get into a situation where a state object is trying to access the
  // state machine in its destructor, but the state machine has already
  // been destroyed
  machine.terminate();

  return 0;

}
