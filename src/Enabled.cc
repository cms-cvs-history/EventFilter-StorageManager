#include "EventFilter/StorageManager/interface/StateMachine.h"

#include <iostream>

using namespace std;
using namespace stor;

Enabled::Enabled( my_context c ): my_base(c)
{
  cout << "Entering " << stateName() << " state" << endl;
}

Enabled::~Enabled()
{
  cout << "Clearing queues, closing files, and exiting " << stateName() << " state" << endl;
}

string Enabled::do_stateName() const
{
  return string( "Enabled" );
}

// void Enabled::handleI2OEventMessage() const
// {
//   cerr << "Error: " << stateName() << " state cannot handle I2O messages" << endl;
// }
