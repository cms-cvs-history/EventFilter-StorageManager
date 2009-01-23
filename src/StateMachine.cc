#include "EventFilter/StorageManager/interface/StateMachine.h"

using namespace stor;

void StateMachine::handleI2OEventMessage()
{
  const Operations& ref = state_cast<Operations const&>();
  ref.handleI2OEventMessage();
}

std::string StateMachine::getCurrentStateName()
{
  return state_cast< const Operations & >().stateName();
}
