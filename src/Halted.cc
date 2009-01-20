#include "EventFilter/StorageManager/interface/StateMachine.h"

#include <iostream>

using namespace std;

Halted::Halted( my_context c ): my_base(c)
{
  cout << "Entering " << stateName() << " state" << endl;
}

Halted::~Halted()
{
  cout << "Exiting " << stateName() << " state" << endl;
}

string Halted::stateName() const
{
  return string( "Halted" );
}

void Halted::handleI2OEventMessage() const
{
  cerr << "Error: " << stateName() << " state cannot handle I2O messages" << endl;
}
