#include "EventFilter/StorageManager/interface/Ready.h"
#include "EventFilter/StorageManager/interface/StateMachine.h"
#include "EventFilter/StorageManager/interface/Operations.h"
#include "EventFilter/StorageManager/interface/Normal.h"
#include "EventFilter/StorageManager/interface/Enabled.h"
#include "EventFilter/StorageManager/interface/Halted.h"

#include <iostream>

using namespace std;

Ready::Ready()
{
  cout << "Entering " << state_name() << " inner state" << endl;
}

Ready::~Ready()
{
  cout << "Exiting " << state_name() << " inner state" << endl;
}

const string& Ready::state_name() const
{
  return string( "Ready" );
}

void Ready::handle_I2O_event_message() const
{
  cerr << " state cannot handle I2O messages" << endl;
}
