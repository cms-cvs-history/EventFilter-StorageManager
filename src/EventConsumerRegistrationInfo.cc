// $Id: nInfo.cc,v 1.1.2.4 2009/03/10 20:39:44 biery Exp $

#include "EventFilter/StorageManager/interface/EventConsumerRegistrationInfo.h"
#include "EventFilter/StorageManager/interface/EventDistributor.h"

#include <algorithm>
#include <iterator>
#include <ostream>
#include <string>

using namespace std;

namespace stor
{
  EventConsumerRegistrationInfo::EventConsumerRegistrationInfo
  (string const& sourceURL,
   unsigned int maxConnectRetries,
   unsigned int connectRetryInterval, // seconds
   const string& consumerName,
   unsigned int headerRetryInterval, // seconds
   double maxEventRequestRate, // Hz
   const FilterList& selEvents,
   const string& selHLTOut,
   unsigned int secondsToStale,
   QueueID queueId) :
    _common(sourceURL, consumerName, headerRetryInterval, 
            maxEventRequestRate, queueId),
    _maxConnectRetries( maxConnectRetries ),
    _connectRetryInterval( connectRetryInterval ),
    _selEvents( selEvents ),
    _selHLTOut( selHLTOut ),
    _secondsToStale( secondsToStale )
  { }

  EventConsumerRegistrationInfo::~EventConsumerRegistrationInfo()
  { }

  void 
  EventConsumerRegistrationInfo::do_registerMe(EventDistributor* evtDist)
  {
    evtDist->registerEventConsumer(this);
  }

  QueueID
  EventConsumerRegistrationInfo::do_queueId() const
  {
    return _common.queueId;
  }

  string
  EventConsumerRegistrationInfo::do_sourceURL() const
  {
    return _common.sourceURL;
  }

  string
  EventConsumerRegistrationInfo::do_consumerName() const
  {
    return _common.consumerName;
  }

  unsigned int
  EventConsumerRegistrationInfo::do_headerRetryInterval() const
  {
    return _common.headerRetryInterval;
  }

  double
  EventConsumerRegistrationInfo::do_maxEventRequestRate() const
  {
    return _common.maxEventRequestRate;
  }

  ostream& 
  EventConsumerRegistrationInfo::write(ostream& os) const
  {
    os << "EventConsumerRegistrationInfo:" << '\n'
       << " Source URL: " << _common.sourceURL << '\n'
       << " Maximum number of connection attempts: "
       << _maxConnectRetries << '\n'
       << " Connection retry interval, seconds: "
       << _connectRetryInterval << '\n'
       << " Consumer name: " << _common.consumerName << '\n'
       << " Header retry interval, seconds: "
       << _common.headerRetryInterval << '\n'
       << " Maximum event request rate, Hz: "
       << _common.maxEventRequestRate << '\n'
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
       << " Enquing policy: " << queuePolicy();
    return os;
  }

}

/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
