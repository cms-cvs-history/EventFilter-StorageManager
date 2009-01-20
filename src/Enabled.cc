#include "EventFilter/StorageManager/interface/StateMachine.h"

#include <iostream>

using namespace std;

Enabled::Enabled( my_context c ): my_base(c)
{
  cout << "Entering " << stateName() << " state" << endl;
}

Enabled::~Enabled()
{
  cout << "Closing files and exiting " << stateName() << " state" << endl;
}

string Enabled::stateName() const
{
  return string( "Enabled" );
}

void Enabled::handleI2OEventMessage() const
{
  cerr << "ERROR: " << stateName() << " state cannot handle I2O messages" << endl;
}
