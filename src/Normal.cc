#include "EventFilter/StorageManager/interface/StateMachine.h"

#include <iostream>

using namespace std;
using namespace stor;

Normal::Normal( my_context c ): my_base(c)
{
  cout << "Entering " << stateName() << " state" << endl;
}

Normal::~Normal()
{
  cout << "Exiting " << stateName() << " state" << endl;
}

string Normal::do_stateName() const
{
  return string( "Normal" );
}

// void Normal::handleI2OEventMessage() const
// {
//   cerr << "Error: " << stateName() << " state cannot handle I2O messages" << endl;
// }
