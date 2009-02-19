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

  StateMachine m( 0, 0, 0, 0 );

  CommandQueue q;

  /*
  q.enq_nowait( shared_ptr<event_base>( new Configure() ) );
  q.enq_nowait( shared_ptr<event_base>( new Enable() ) );
  q.enq_nowait( shared_ptr<event_base>( new Enable() ) );
  q.enq_nowait( shared_ptr<event_base>( new Stop() ) );
  q.enq_nowait( shared_ptr<event_base>( new Halt() ) );
  q.enq_nowait( shared_ptr<event_base>( new Reconfigure() ) );
  q.enq_nowait( shared_ptr<event_base>( new EmergencyStop() ) );
  q.enq_nowait( shared_ptr<event_base>( new StopDone() ) );
  */
  q.enq_nowait( shared_ptr<event_base>( new Fail() ) );
  q.enq_nowait( shared_ptr<event_base>( new Configure() ) );

  m.initiate();

  shared_ptr<event_base> evt;

  while( !q.empty() )
    {
      if( !q.deq_nowait( evt ) )
	{
	  cerr << "Queue was not empty but deq_nowait failed"
	       << endl;
	  exit(1);
	}
      m.process_event( *evt );
    }

  m.terminate();

}
