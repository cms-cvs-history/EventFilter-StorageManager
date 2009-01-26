#include "EventFilter/StorageManager/interface/StateMachine.h"

#include <iostream>

using namespace std;
using namespace stor;

DrainingQueues::DrainingQueues( my_context c ): my_base(c)
{
  cout << "Entering " << stateName() << " state" << endl;

  // At this point, we need to create a thread that waits
  // for the queues to drain and then posts a StopDone event
  // to the command queue - we do not post the event from
  // this class.  Of course, we need to be careful to clean
  // up the thread if we exit the DrainingQueues state before
  // the thread is finished (e.g. an EmergencyStop event is
  // processed).  That would probably mean setting some watch
  // variable in the thread telling it to exit and then 
  // joining it in the destructor of this class.  Although,
  // maybe that would be handled differently with xdaq workloops.
}

DrainingQueues::~DrainingQueues()
{
  cout << "Exiting " << stateName() << " state" << endl;
}

string DrainingQueues::do_stateName() const
{
  return string( "DrainingQueues" );
}

// void DrainingQueues::handleI2OEventMessage() const
// {
//   cerr << "Error: " << stateName() << " state cannot handle I2O messages" << endl;
// }
