#include "EventFilter/StorageManager/interface/StateMachine.h"

using namespace stor;
using namespace std;

// void StateMachine::handleI2OEventMessage()
// {
//   const Operations& ref = state_cast<Operations const&>();
//   ref.handleI2OEventMessage();
// }

Operations const&
StateMachine::getCurrentState()
{
  return state_cast<Operations const&>();
}

string StateMachine::getCurrentStateName()
{
  return getCurrentState().stateName();
}

void StateMachine::updateHistory( const TransitionRecord& tr )
{
  _history.push_back( tr );
}
