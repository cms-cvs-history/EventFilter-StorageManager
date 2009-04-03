// -*- c++ -*-
// $Id: EventConsumerRegistrationInfo.h,v 1.1.2.11 2009/04/03 12:30:44 dshpakov Exp $

#ifndef EVENTCONSUMERREGISTRATIONINFO_H
#define EVENTCONSUMERREGISTRATIONINFO_H


#include <iosfwd>
#include <string>
#include <vector>

#include "boost/shared_ptr.hpp"

#include "EventFilter/StorageManager/interface/RegistrationInfoBase.h"
#include "EventFilter/StorageManager/interface/CommonRegistrationInfo.h"

namespace xgi
{
  class Input;
}

namespace stor
{
  /**
   * Holds the registration information from a event consumer.
   *
   * $Author: dshpakov $
   * $Revision: 1.1.2.11 $
   * $Date: 2009/04/03 12:30:44 $
   */

  class EventConsumerRegistrationInfo : public RegistrationInfoBase
  {
  public:

    typedef std::vector<std::string> FilterList;

    /**
     * Constructs an instance with the specified registration information.
     */
    EventConsumerRegistrationInfo( unsigned int maxConnectRetries,
				   unsigned int connectRetryInterval,// seconds
				   const std::string& consumerName,
				   unsigned int headerRetryInterval, // seconds
				   double maxEventRequestRate, // Hz
				   const FilterList& selEvents,
				   const std::string& selHLTOut,
				   unsigned int secondsToStale );

    ~EventConsumerRegistrationInfo();


    // Additional accessors:
    unsigned int maxConnectRetries() const { return _maxConnectRetries; }
    unsigned int connectRetryInterval() const { return _connectRetryInterval; }
    const FilterList& selEvents() const { return _selEvents; }
    const std::string& selHLTOut() const { return _selHLTOut; }
    unsigned int secondsToStale() const { return _secondsToStale; }
    bool isProxyServer() const { return _isProxy; }

    // Output:
    std::ostream& write(std::ostream& os) const;

    // Implementation of Template Method pattern.
    virtual void do_registerMe(EventDistributor*);
    virtual QueueID do_queueId() const;
    virtual std::string do_consumerName() const;
    virtual unsigned int do_headerRetryInterval() const;
    virtual double       do_maxEventRequestRate() const;

    // Set queue ID:
    void setQueueID( const QueueID& qid ) { _common.queueId = qid; }

  private:

    CommonRegistrationInfo _common;

    unsigned int     _maxConnectRetries;
    unsigned int     _connectRetryInterval;
    FilterList       _selEvents;
    std::string      _selHLTOut;
    unsigned long    _secondsToStale;
    bool             _isProxy;

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

  typedef boost::shared_ptr<stor::EventConsumerRegistrationInfo> ConsRegPtr;

  /**
     Parse consumer registration request (free function):
  */
  ConsRegPtr parseEventConsumerRegistration( xgi::Input* in,
					     unsigned int secondsToStale );

} // namespace stor

#endif
