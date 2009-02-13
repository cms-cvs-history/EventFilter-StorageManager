#include "EventFilter/StorageManager/interface/StateMachine.h"

#include "toolbox/task/Action.h"
#include "toolbox/task/WorkLoopFactory.h"

#include <iostream>

using namespace std;
using namespace stor;

DrainingQueues::DrainingQueues( my_context c ): my_base(c)
{
  TransitionRecord tr( stateName(), true );
  outermost_context().updateHistory( tr );
  post_event( StopDone() );
}

DrainingQueues::~DrainingQueues()
{
  TransitionRecord tr( stateName(), false );
  outermost_context().updateHistory( tr );
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
}


bool DrainingQueues::action(toolbox::task::WorkLoop* wl)
{
  return false;
}

/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
