#include "EventFilter/StorageManager/interface/StateMachine.h"

#include <iostream>
#include <sys/time.h>

using namespace std;
using namespace stor;

TransitionRecord::TransitionRecord( const std::string& state_name,
				    bool is_entry ):
  _stateName( state_name ),
  _isEntry( is_entry )
{
  gettimeofday( &_timestamp, NULL );
}
