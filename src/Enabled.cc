#include "EventFilter/StorageManager/interface/Enabled.h"
#include "EventFilter/StorageManager/interface/Normal.h"
#include "EventFilter/StorageManager/interface/StateMachine.h"
#include "EventFilter/StorageManager/interface/Operations.h"
#include "EventFilter/StorageManager/interface/Halted.h"
#include "EventFilter/StorageManager/interface/Ready.h"

#include <iostream>

using namespace std;

unsigned int Enabled::_counter = 0;

Enabled::Enabled( my_context c ): my_base(c)
{

  ++_counter;
  if( _counter > 3 )
    {
      cerr << state_name() << " created too many times" << endl;
      post_event( Fail() );
      return;
    }

  cout << "Entering " << state_name() << " inner state" << endl;

}

Enabled::~Enabled()
{
  cout << "Closing files..." << endl;
  cout << "Exiting " << state_name() << " inner state" << endl;
}

const string& Enabled::state_name() const
{
  return string( "Enabled" );
}

void Enabled::handle_I2O_event_message() const
{
  cout << "Handling I2O event message..." << endl;
}
