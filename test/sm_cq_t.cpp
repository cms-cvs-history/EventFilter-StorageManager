#include <iostream>

#include <boost/statechart/event_base.hpp>
#include <boost/shared_ptr.hpp>

#include "EventFilter/StorageManager/interface/StateMachine.h"
#include "EventFilter/StorageManager/interface/CommandQueue.h"

using namespace std;
using namespace boost::statechart;
using namespace stor;

using boost::shared_ptr;

int main()
{

  StateMachine m( 0, 0, 0 );

  CommandQueue q;

  q.push_front( shared_ptr<event_base>( new Configure() ) );
  q.push_front( shared_ptr<event_base>( new Enable() ) );
  q.push_front( shared_ptr<event_base>( new Enable() ) );
  q.push_front( shared_ptr<event_base>( new Stop() ) );
  q.push_front( shared_ptr<event_base>( new Halt() ) );
  q.push_front( shared_ptr<event_base>( new Reconfigure() ) );
  q.push_front( shared_ptr<event_base>( new EmergencyStop() ) );
  q.push_front( shared_ptr<event_base>( new StopDone() ) );
  q.push_front( shared_ptr<event_base>( new Fail() ) );
  q.push_front( shared_ptr<event_base>( new Configure() ) );

  m.initiate();

  shared_ptr<event_base> evt;

  while( !q.empty() )
    {
      if( !q.pop_back( evt ) )
	{
	  cerr << "Queue was not empty but pop_back failed"
	       << endl;
	  exit(1);
	}
      m.process_event( *evt );
    }

  m.terminate();

}
