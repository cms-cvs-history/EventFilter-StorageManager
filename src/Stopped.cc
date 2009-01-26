#include "EventFilter/StorageManager/interface/StateMachine.h"

#include <iostream>

using namespace std;
using namespace stor;

Stopped::Stopped( my_context c ): my_base(c)
{
  cout << "Entering " << stateName() << " state" << endl;
}

Stopped::~Stopped()
{
  cout << "Exiting " << stateName() << " state" << endl;
}

string Stopped::do_stateName() const
{
  return string( "Stopped" );
}

// void Stopped::handleI2OEventMessage() const
// {
//   cerr << "Error: " << stateName() << " state cannot handle I2O messages" << endl;
// }
