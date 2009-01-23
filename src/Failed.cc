#include "EventFilter/StorageManager/interface/StateMachine.h"

#include <iostream>

using namespace std;
using namespace stor;

Failed::Failed( my_context c ): my_base(c)
{
  cout << "Entering " << stateName() << " state" << endl;
}

Failed::~Failed()
{
  cout << "Exiting " << stateName() << " state" << endl;
}

string Failed::stateName() const
{
  return string( "Failed" );
}

void Failed::handleI2OEventMessage() const
{
  cerr << "Error: " << stateName() << " state cannot handle I2O messages" << endl;
}
