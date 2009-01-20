#include "EventFilter/StorageManager/interface/StateMachine.h"

#include <iostream>

using namespace std;

Ready::Ready( my_context c ): my_base(c)
{
  cout << "Entering " << stateName() << " state" << endl;
}

Ready::~Ready()
{
  cout << "Exiting " << stateName() << " state" << endl;
}

string Ready::stateName() const
{
  return string( "Ready" );
}

void Ready::handleI2OEventMessage() const
{
  cerr << "ERROR: " << stateName() << " state cannot handle I2O messages" << endl;
}
