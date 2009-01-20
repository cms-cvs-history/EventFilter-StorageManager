#include "EventFilter/StorageManager/interface/StateMachine.h"

#include <iostream>

using namespace std;

unsigned int Processing::_counter = 0;

Processing::Processing( my_context c ): my_base(c)
{

  ++_counter;
  if( _counter > 2 )
    {
      cerr << "ERROR: " << stateName() << " state created too many times" << endl;
      post_event( Fail() );
      return;
    }

  cout << "Entering " << stateName() << " state, counter = " << _counter << endl;

}

Processing::~Processing()
{
  cout << "Exiting " << stateName() << " state" << endl;
}

string Processing::stateName() const
{
  return string( "Processing" );
}

void Processing::handleI2OEventMessage() const
{
  cout << "Handling I2O event message..." << endl;
}
