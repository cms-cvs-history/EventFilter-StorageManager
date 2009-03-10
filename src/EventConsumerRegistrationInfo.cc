// $Id: EventConsumerRegistrationInfo.cc,v 1.1.2.3 2009/03/02 17:41:43 paterno Exp $

#include "EventFilter/StorageManager/interface/EventConsumerRegistrationInfo.h"
#include "EventFilter/StorageManager/interface/EventDistributor.h"

#include <algorithm>
#include <iterator>
#include <ostream>
#include <string>

using namespace std;

namespace stor
{
  void EventConsumerRegistrationInfo::registerMe(EventDistributor* evtDist)
  {
    evtDist->registerEventConsumer(this);
  }

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
