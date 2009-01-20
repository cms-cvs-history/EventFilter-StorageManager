#include "EventFilter/StorageManager/interface/StateMachine.h"

#include <iostream>

using namespace std;

Stopped::Stopped( my_context c ): my_base(c)
{
  cout << "Entering " << stateName() << " state" << endl;
}

Stopped::~Stopped()
{
  cout << "Exiting " << stateName() << " state" << endl;
}

string Stopped::stateName() const
{
  return string( "Stopped" );
}

void Stopped::handleI2OEventMessage() const
{
  cerr << "ERROR: " << stateName() << " state cannot handle I2O messages" << endl;
}
