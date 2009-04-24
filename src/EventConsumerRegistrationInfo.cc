// $Id: EventConsumerRegistrationInfo.cc,v 1.1.2.17 2009/04/13 08:51:19 dshpakov Exp $

#include "EventFilter/StorageManager/interface/EventConsumerRegistrationInfo.h"
#include "EventFilter/StorageManager/interface/EventDistributor.h"
#include "EventFilter/StorageManager/interface/Exception.h"

#include <algorithm>
#include <iterator>
#include <ostream>

using namespace std;

namespace stor
{

  EventConsumerRegistrationInfo::EventConsumerRegistrationInfo
  ( unsigned int maxConnectRetries,
    unsigned int connectRetryInterval, // seconds
    const string& consumerName,
    unsigned int headerRetryInterval, // seconds
    double maxEventRequestRate, // Hz
    const FilterList& selEvents,
    const string& selHLTOut,
    utils::duration_t secondsToStale ):
    _common( consumerName, headerRetryInterval, 
             maxEventRequestRate, QueueID() ),
    _maxConnectRetries( maxConnectRetries ),
    _connectRetryInterval( connectRetryInterval ),
    _selEvents( selEvents ),
    _selHLTOut( selHLTOut ),
    _secondsToStale( secondsToStale )
  {
    if( consumerName == "SMProxyServer" ||
        ( consumerName.find( "urn" ) != std::string::npos &&
          consumerName.find( "xdaq" ) != std::string::npos &&
          consumerName.find( "pushEventData" ) != std::string::npos ) )
      {
        _isProxy = true;
      }
    else
      {
        _isProxy = false;
      }
  }

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
  EventConsumerRegistrationInfo::do_consumerName() const
  {
    return _common.consumerName;
  }

  ConsumerID
  EventConsumerRegistrationInfo::do_consumerID() const
  {
    return _common.consumerId;
  }

  void
  EventConsumerRegistrationInfo::do_setConsumerID(ConsumerID id)
  {
    _common.consumerId = id;
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
