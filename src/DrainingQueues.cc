#include "EventFilter/StorageManager/interface/StateMachine.h"

#include "toolbox/task/Action.h"
#include "toolbox/task/WorkLoopFactory.h"

#include <iostream>

using namespace std;
using namespace stor;

DrainingQueues::DrainingQueues( my_context c ): my_base(c)
{
  cout << "Entering " << stateName() << " state" << endl;

  // Get the work loop
  // This workloop may or may not exists from a previous invocation of
  // DrainingQueues, but should have no actions attached to it.
  // Note that the workloop is ownded by the factory.
  _workloop =
    toolbox::task::getWorkLoopFactory()->getWorkLoop("DrainingQueuesWorkloop", "waiting");

  if ( ! _workloop->isActive() )
  {
    // only do it if the we are not already working on draining the queues
    _doDraining = true;

    // Bind the action
    toolbox::task::ActionSignature* actionSignature = 
      toolbox::task::bind(this, &stor::DrainingQueues::action, "DrainingQueuesAction");

    // Add action to workloop
    _workloop->submit(actionSignature);
    
    // Activate the workloop
    _workloop->activate();
  } 
  else
  {
    // I guess we need something more drastic here
    cout << "Draining queues already in progress" << endl;
  }
}

DrainingQueues::~DrainingQueues()
{
  cout << "Exiting " << stateName() << " state" << endl;

  // Stop the draining action
  _doDraining = false;

  // Cancel the workloop
  // Cancel will wait until the draining action has finished
  _workloop->cancel();
}

string DrainingQueues::do_stateName() const
{
  return string( "DrainingQueues" );
}

// void DrainingQueues::handleI2OEventMessage() const
// {
//   cerr << "Error: " << stateName() << " state cannot handle I2O messages" << endl;
// }


void DrainingQueues::emergencyStop(const EmergencyStop &event)
{
  _doDraining = false;
}


bool DrainingQueues::action(toolbox::task::WorkLoop* wl)
{
  unsigned int count = 0;

  while ( _doDraining && count < 10 )
  {
    std::cout << "Draining queues" << std::endl;
    ::usleep(100);
    ++count;
  }

  post_event( StopDone() );

  std::cout << "Draining queues finished" << std::endl;

  return false; // do not reschedule me (removes the action from the wl)
}


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
