// -*- c++ -*-
// $Id: EventConsumerRegistrationInfo.h,v 1.1.2.7 2009/03/10 21:19:38 biery Exp $

#ifndef EVENTCONSUMERREGISTRATIONINFO_H
#define EVENTCONSUMERREGISTRATIONINFO_H


#include <iosfwd>
#include <string>
#include <vector>

#include "EventFilter/StorageManager/interface/RegistrationInfoBase.h"
#include "EventFilter/StorageManager/interface/CommonRegistrationInfo.h"

namespace stor
{
  /**
   * Holds the registration information from a event consumer.
   *
   * $Author: biery $
   * $Revision: 1.1.2.7 $
   * $Date: 2009/03/10 21:19:38 $
   */

  class EventConsumerRegistrationInfo : public RegistrationInfoBase
  {
  public:

    typedef std::vector<std::string> FilterList;

    /**
     * Constructs an instance with the specified registration information.
     */
    EventConsumerRegistrationInfo(const std::string& sourceURL,
				  unsigned int maxConnectRetries,
				  unsigned int connectRetryInterval,// seconds
				  const std::string& consumerName,
				  unsigned int headerRetryInterval, // seconds
				  double maxEventRequestRate, // Hz
				  const FilterList& selEvents,
				  const std::string& selHLTOut,
				  unsigned int secondsToStale,
				  QueueID queueId);

    ~EventConsumerRegistrationInfo();


    // Additional accessors:
    unsigned int maxConnectRetries() const { return _maxConnectRetries; }
    unsigned int connectRetryInterval() const { return _connectRetryInterval; }
    const FilterList& selEvents() const { return _selEvents; }
    const std::string& selHLTOut() const { return _selHLTOut; }
    unsigned int secondsToStale() const { return _secondsToStale; }

    // Output:
    std::ostream& write(std::ostream& os) const;

    // Implementation of Template Method pattern.
    virtual void do_registerMe(EventDistributor*);
    virtual QueueID do_queueId() const;
    virtual std::string do_sourceURL() const;
    virtual std::string do_consumerName() const;
    virtual unsigned int do_headerRetryInterval() const;
    virtual double       do_maxEventRequestRate() const;

  private:

    CommonRegistrationInfo _common;

    unsigned int     _maxConnectRetries;
    unsigned int     _connectRetryInterval;
    FilterList       _selEvents;
    std::string      _selHLTOut;
    unsigned long    _secondsToStale;

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
