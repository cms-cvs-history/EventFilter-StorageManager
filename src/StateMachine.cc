#include "EventFilter/StorageManager/interface/StateMachine.h"
#include "EventFilter/StorageManager/interface/Operations.h"

void StateMachine::handle_I2O_event_message()
{
  const Operations& ref = state_cast<Operations const&>();
  ref.handle_I2O_event_message();
}
