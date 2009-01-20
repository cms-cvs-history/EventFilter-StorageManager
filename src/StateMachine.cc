#include "EventFilter/StorageManager/interface/StateMachine.h"

void StateMachine::handleI2OEventMessage()
{
  const Operations& ref = state_cast<Operations const&>();
  ref.handleI2OEventMessage();
}
