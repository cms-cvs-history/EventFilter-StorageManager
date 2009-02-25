// $Id: $

#include "EventFilter/StorageManager/interface/EventStreamConfigurationInfo.h"

using stor::EventStreamConfigurationInfo;
using namespace std;

ostream&
stor::operator << ( ostream& os,
		    const EventStreamConfigurationInfo& ci )
{

  os << "EventStreamConfigurationInfo:" << endl
     << " Stream Label: " << ci.streamLabel() << endl
     << " Maximum Size: " << ci.maxSize() << endl
     << " HLT Output: " << ci.selHLTOut() << endl
     << " Event Filters:";

  for( unsigned int i = 0; i < ci.selEvents().size(); ++i )
    {
      os << endl << "  " << ci.selEvents()[i];
    }

    return os;

}
