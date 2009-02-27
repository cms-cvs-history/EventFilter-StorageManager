// -*- c++ -*-
// $Id: $

#ifndef EVENTCONSUMERREGISTRATIONINFO_H
#define EVENTCONSUMERREGISTRATIONINFO_H

#include <string>
#include <vector>
#include <iostream>

namespace stor
{

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
                                   const std::string& selHLTOut ):
      _sourceURL( sourceURL ),
      _maxConnectRetries( maxConnectRetries ),
      _connectRetryInterval( connectRetryInterval ),
      _consumerName( consumerName ),
      _headerRetryInterval( headerRetryInterval ),
      _maxEventRequestRate( maxEventRequestRate ),
      _selEvents( selEvents ),
      _selHLTOut( selHLTOut )
    {}

    // Destructor:
    ~EventConsumerRegistrationInfo() {}

    // Accessors:
    const std::string& sourceURL() const { return _sourceURL; }
    unsigned int maxConnectRetries() const { return _maxConnectRetries; }
    unsigned int connectRetryInterval() const { return _connectRetryInterval; }
    const std::string& consumerName() const { return _consumerName; }
    unsigned int headerRetryInterval() const { return _headerRetryInterval; }
    double maxEventRequestRate() const { return _maxEventRequestRate; }
    const FilterList& selEvents() const { return _selEvents; }
    const std::string& selHLTOut() const { return _selHLTOut; }

    // Output:
    friend std::ostream& operator <<
      ( std::ostream&, const EventConsumerRegistrationInfo& );

  private:

    std::string _sourceURL;
    unsigned int _maxConnectRetries;
    unsigned int _connectRetryInterval;
    std::string _consumerName;
    unsigned int _headerRetryInterval;
    double _maxEventRequestRate;
    FilterList _selEvents;
    std::string _selHLTOut;

  };
  
} // namespace stor

#endif
