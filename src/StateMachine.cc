#include "EventFilter/StorageManager/interface/StateMachine.h"

using namespace stor;

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

std::string StateMachine::getCurrentStateName()
{
  return getCurrentState().stateName();
}
