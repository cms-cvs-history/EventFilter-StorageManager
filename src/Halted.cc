#include "EventFilter/StorageManager/interface/StateMachine.h"
#include "EventFilter/StorageManager/interface/Operations.h"
#include "EventFilter/StorageManager/interface/Normal.h"
#include "EventFilter/StorageManager/interface/Configure.h"
#include "EventFilter/StorageManager/interface/Ready.hpp"

#include <iostream>

using namespace std;

Halted::Halted()
{
  cout << "Entering " << state_name() << " inner state" << endl;
}

Halted::~Halted()
{
  cout << "Exiting " << state_name() << " inner state" << endl;
}

const string& Halted::state_name() const
{
  return string( "Halted" );
}

void Halted::hanlde_I2O_event_message() const
{
  cerr << state_name() << " state cannot handle I2O messages" << endl;
}
