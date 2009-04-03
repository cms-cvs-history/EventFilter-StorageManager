// $Id: EventConsumerRegistrationInfo.cc,v 1.1.2.6 2009/04/02 13:55:28 dshpakov Exp $

#include "EventFilter/StorageManager/interface/EventConsumerRegistrationInfo.h"
#include "EventFilter/StorageManager/interface/EventDistributor.h"

#include "xgi/Input.h"

#include "IOPool/Streamer/interface/ConsRegMessage.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include <algorithm>
#include <iterator>
#include <ostream>
#include <string>

using namespace std;

namespace stor
{

  // Create consumer registration info (free function):
  ConsRegPtr parseEventConsumerRegistration( xgi::Input* in )
  {

    // TODO: what if in = 0 ?

    string name = "unknown";
    string pset_str = "<>";

    const string l_str = in->getenv( "CONTENT_LENGTH" );
    unsigned long l = atol( l_str.c_str() );

    if( l > 0 )
      {
        auto_ptr< vector<char> > buf( new vector<char>(l) );
        in->read( &(*buf)[0], l );
        ConsRegRequestView req( &(*buf)[0] );
        name = req.getConsumerName();
        pset_str = req.getRequestParameterSet();
      }

    const edm::ParameterSet pset( pset_str );

    // TODO: hanlde exceptions, choose defaults
    const double max_rate =
      pset.getUntrackedParameter<double>( "maxEventRequestRate", 1.0 );
    const string sel_hlt_out =
      pset.getUntrackedParameter<string>( "SelectHLTOutput", "none" );
    const EventConsumerRegistrationInfo::FilterList sel_events =
      pset.getParameter<Strings>( "SelectEvents" );
    const unsigned int max_conn_retr =
      pset.getUntrackedParameter<int>( "maxConnectTries", 10 );
    const unsigned int conn_retr_interval =
      pset.getUntrackedParameter<int>( "connectTrySleepTime", 10 );
    const unsigned int hdr_retr_interval =
      pset.getUntrackedParameter<int>( "headerRetryInterval", 5 );
    const unsigned int secs_to_stale =
      pset.getUntrackedParameter<int>( "secondsToStale", 10 ); // TODO

    ConsRegPtr cr( new EventConsumerRegistrationInfo( max_conn_retr,
                                                      conn_retr_interval,
                                                      name,
                                                      hdr_retr_interval,
                                                      max_rate,
                                                      sel_events,
                                                      sel_hlt_out,
                                                      secs_to_stale ) );
    return cr;

  }

  EventConsumerRegistrationInfo::EventConsumerRegistrationInfo
  ( unsigned int maxConnectRetries,
    unsigned int connectRetryInterval, // seconds
    const string& consumerName,
    unsigned int headerRetryInterval, // seconds
    double maxEventRequestRate, // Hz
    const FilterList& selEvents,
    const string& selHLTOut,
    unsigned int secondsToStale ):
    _common( consumerName, headerRetryInterval, 
             maxEventRequestRate, QueueID() ),
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
