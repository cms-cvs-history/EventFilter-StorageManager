#include "EventFilter/StorageManager/interface/Failed.h"
#include "EventFilter/StorageManager/interface/StateMachine.h"
#include "EventFilter/StorageManager/interface/Operations.h"

#include <iostream>

using namespace std;

Failed::Failed()
{
  cout << "Entering " << state_name() << " outer state" << endl;
}

Failed::~Failed()
{
  cout << "Exiting " << state_name() << " outer state" << endl;
}

const string& Failed::state_name() const
{
  return string( "Failed" );
}

void Failed::handle_I2O_event_message() const
{
  cerr << state_name() << " state cannot handle I2O messages" << endl;
}
