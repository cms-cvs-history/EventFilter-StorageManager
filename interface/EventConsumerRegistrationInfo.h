// -*- c++ -*-
// $Id: EventConsumerRegistrationInfo.h,v 1.1.2.3 2009/03/02 17:41:43 paterno Exp $

#ifndef EVENTCONSUMERREGISTRATIONINFO_H
#define EVENTCONSUMERREGISTRATIONINFO_H


#include <iosfwd>
#include <string>
#include <vector>

#include "EventFilter/StorageManager/interface/EnquingPolicyTag.h"
#include "EventFilter/StorageManager/interface/Types.h"

namespace stor
{
  /**
   * This struct holds the registration information from a event
   * consumer
   *
   * $Author: paterno $
   * $Revision: 1.1.2.3 $
   * $Date: 2009/03/02 17:41:43 $
   */

  class EventConsumerRegistrationInfo

  {
  public:

    typedef std::vector<std::string> FilterList;

    // Constructor:
    EventConsumerRegistrationInfo( const std::string& sourceURL,
                                   unsigned int maxConnectRetries,
                                   unsigned int connectRetryInterval, // seconds
                                   const std::string& consumerName,
                                   unsigned int headerRetryInterval, // seconds
                                   double maxEventRequestRate, // Hz
                                   const FilterList& selEvents,
                                   const std::string& selHLTOut,
				   unsigned int secondsToStale,
				   enquing_policy::PolicyTag policy) :
      _sourceURL( sourceURL ),
      _maxConnectRetries( maxConnectRetries ),
      _connectRetryInterval( connectRetryInterval ),
      _consumerName( consumerName ),
      _headerRetryInterval( headerRetryInterval ),
      _maxEventRequestRate( maxEventRequestRate ),
      _selEvents( selEvents ),
      _selHLTOut( selHLTOut ),
      _secondsToStale( secondsToStale ),
      _policy( policy ),
      _queueId( 0 )
    {}

    // Compiler-generated copy constructor, copy assignment, and
    // destructor do the right thing.

    // Accessors:
    const std::string& sourceURL() const { return _sourceURL; }
    unsigned int maxConnectRetries() const { return _maxConnectRetries; }
    unsigned int connectRetryInterval() const { return _connectRetryInterval; }
    const std::string& consumerName() const { return _consumerName; }
    unsigned int headerRetryInterval() const { return _headerRetryInterval; }
    double maxEventRequestRate() const { return _maxEventRequestRate; }
    const FilterList& selEvents() const { return _selEvents; }
    const std::string& selHLTOut() const { return _selHLTOut; }
    unsigned int secondsToStale() const { return _secondsToStale; }
    enquing_policy::PolicyTag policy() const { return _policy; }
    QueueID queueId() const { return _queueId; }

    // Set queue Id:
    void setQueueId( QueueID qid ) { _queueId = qid; }

    // Output:
    std::ostream& write(std::ostream& os) const;

  private:

    std::string      _sourceURL;
    unsigned int     _maxConnectRetries;
    unsigned int     _connectRetryInterval;
    std::string      _consumerName;
    unsigned int     _headerRetryInterval;
    double           _maxEventRequestRate;
    FilterList       _selEvents;
    std::string      _selHLTOut;
    unsigned long    _secondsToStale;
    enquing_policy::PolicyTag _policy;
    QueueID          _queueId;
  };

  /**
     Print the given EventConsumerRegistrationInfo to the given
     stream.
   */
  inline
  std::ostream& operator<<(std::ostream& os, 
                           EventConsumerRegistrationInfo const& ri)
  {
    return ri.write(os);
  }
  
} // namespace stor

#endif
