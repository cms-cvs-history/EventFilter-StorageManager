#include "EventFilter/StorageManager/interface/StateMachine.h"
#include "EventFilter/StorageManager/interface/Failed.h"

#include <iostream>

using namespace std;

Normal::Normal()
{
  cout << "Entering " << state_name() << " outer state" << endl;
}

Normal::~Normal()
{
  cout << "Exiting " << state_name() << " outer state" << endl;
}

const string& Normal::state_name() const
{
  return string( "Normal" );
}

void Normal::handle_I2O_event_message() const
{
  cerr << state_name() << " state cannot handle I2O messages" << endl;
}
