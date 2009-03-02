// $Id: EventConsumerRegistrationInfo.cc,v 1.1.2.2 2009/02/27 13:25:00 dshpakov Exp $

#include "EventFilter/StorageManager/interface/EventConsumerRegistrationInfo.h"

#include <algorithm>
#include <iterator>
#include <ostream>
#include <string>

using namespace std;

namespace stor
{
  ostream& 
  EventConsumerRegistrationInfo::write(ostream& os) const
  {
    os << "EventConsumerRegistrationInfo:" << '\n'
       << " Source URL: " << _sourceURL << '\n'
       << " Maximum number of connection attempts: "
       << _maxConnectRetries << '\n'
       << " Connection retry interval, seconds: "
       << _connectRetryInterval << '\n'
       << " Consumer name: " << _consumerName << '\n'
       << " Header retry interval, seconds: "
       << _headerRetryInterval << '\n'
       << " Maximum event request rate, Hz: "
       << _maxEventRequestRate << '\n'
       << " HLT output: " << _selHLTOut << '\n'
       << " Event filters:";

    copy(_selEvents.begin(), 
         _selEvents.end(),
         ostream_iterator<FilterList::value_type>(os, "\n"));
    
//     for( unsigned int i = 0; i < _selEvents.size(); ++i )
//       {
//         os << '\n' << "  " << _selEvents[i];
//       }
    os << " Stale event timeout (seconds): " << _secondsToStale << '\n'
       << " Enquing policy: " << _policy;
    return os;
  }

}

/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
