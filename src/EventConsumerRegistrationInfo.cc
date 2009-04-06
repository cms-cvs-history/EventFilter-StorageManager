// $Id: EventConsumerRegistrationInfo.cc,v 1.1.2.12 2009/04/06 13:28:42 dshpakov Exp $

#include "EventFilter/StorageManager/interface/EventConsumerRegistrationInfo.h"
#include "EventFilter/StorageManager/interface/EventDistributor.h"
#include "EventFilter/StorageManager/interface/Exception.h"

#include "xgi/Input.h"
#include "xgi/exception/Exception.h"

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
  ConsRegPtr parseEventConsumerRegistration( xgi::Input* in,
                                             unsigned int sts )
  {

    if( in == 0 )
      {
        XCEPT_RAISE( xgi::exception::Exception,
                     "Null xgi::Input* in parseEventConsumerRegistration" );
      }

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
    else
      {
        XCEPT_RAISE( stor::exception::ConsumerRegistration,
                     "Bad request length" );
      }

    const edm::ParameterSet pset( pset_str );

    //
    //// Check if HLT output module is there: ////
    //

    string sel_hlt_out = "";

    try
      {
        // new-style consumer
        sel_hlt_out = pset.getParameter<string>( "TrackedHLTOutMod" );
      }
    catch( xcept::Exception& e )
      {
        // old-style consumer or param not specified
        sel_hlt_out =
          pset.getUntrackedParameter<string>( "SelectHLTOutput", "" );
      }

    if( sel_hlt_out == "" )
      {
        XCEPT_RAISE( stor::exception::ConsumerRegistration,
                     "No HLT output module specified" );
      }

    // Maximum event rate:
    double max_rate = 1.0;
    try
      {
        max_rate = pset.getParameter<double>( "TrackedMaxRate" );
      }
    catch( xcept::Exception& e )
      {
        max_rate =
          pset.getUntrackedParameter<double>( "maxEventRequestRate", 1.0 );
      }

    // Event filters:
    EventConsumerRegistrationInfo::FilterList sel_events;
    try
      {
        sel_events = pset.getParameter<Strings>( "TrackedEventSelection" );
      }
    catch( xcept::Exception& e )
      {
        pset.getParameter<Strings>( "SelectEvents" );
      }

    // Number of retries:
    unsigned int max_conn_retr = 5;
    try
      {
        max_conn_retr = pset.getParameter<int>( "TrackedMaxConnectTries" );
      }
    catch( xcept::Exception& e )
      {
        pset.getUntrackedParameter<int>( "maxConnectTries", 5 );
      }

    // Retry interval:
    unsigned int conn_retr_interval = 10;
    try
      {
        conn_retr_interval =
          pset.getParameter<int>( "TrackedConnectTrySleepTime" );
      }
    catch( xcept::Exception& e )
      {
        conn_retr_interval =
          pset.getUntrackedParameter<int>( "connectTrySleepTime", 10 );
      }

    // Header retry interval:
    unsigned int hdr_retr_interval = 5;
    try
      {
        hdr_retr_interval =
          pset.getParameter<int>( "TrackedHeaderRetryInterval" );
      }
    catch( xcept::Exception& e )
      {
        hdr_retr_interval =
          pset.getUntrackedParameter<int>( "headerRetryInterval", 5 );
      }

    ConsRegPtr cr( new EventConsumerRegistrationInfo( max_conn_retr,
                                                      conn_retr_interval,
                                                      name,
                                                      hdr_retr_interval,
                                                      max_rate,
                                                      sel_events,
                                                      sel_hlt_out,
                                                      sts ) );
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
  {
    if( consumerName == "SMProxyServer" )
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
