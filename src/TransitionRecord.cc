// $Id: StateMachine.cc,v 1.1.2.20 2009/04/23 19:19:49 mommsen Exp $

#include "EventFilter/StorageManager/interface/StateMachine.h"

#include <iostream>
#include <sys/time.h>

using namespace stor;
using namespace std;

TransitionRecord::TransitionRecord( const std::string& state_name,
				    bool is_entry ):
  _stateName( state_name ),
  _isEntry( is_entry )
{
  gettimeofday( &_timestamp, NULL );
}

std::ostream& stor::operator << ( std::ostream& os,
				  const TransitionRecord& tr )
{

  os << tr.timeStamp().tv_sec << "."
     << tr.timeStamp().tv_usec << ": ";

  if( tr.isEntry() )
    {
      os << "entered";
    }
  else
    {
      os << "exited";
    }

  os << " " << tr.stateName();

  return os;

}


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
