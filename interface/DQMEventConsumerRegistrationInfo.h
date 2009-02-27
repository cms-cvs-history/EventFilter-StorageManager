// -*- c++ -*-
// $Id: $

#ifndef DQMEVENTCONSUMERREGISTRATIONINFO_H
#define DQMEVENTCONSUMERREGISTRATIONINFO_H

#include <string>
#include <iostream>

namespace stor
{

  class DQMEventConsumerRegistrationInfo
  {

  public:

    // Constructor:
    DQMEventConsumerRegistrationInfo( const std::string& sourceURL,
				      const std::string& consumerName,
				      unsigned int headerRetryInterval, // seconds
				      double maxEventRequestRate, // Hz
				      const std::string& topLevelFolderName ):
      _sourceURL( sourceURL ),
      _consumerName( consumerName ),
      _headerRetryInterval( headerRetryInterval ),
      _maxEventRequestRate( maxEventRequestRate ),
      _topLevelFolderName( topLevelFolderName )
    {}

    // Destructor:
    ~DQMEventConsumerRegistrationInfo() {}

    // Accessors:
    const std::string& sourceURL() const { return _sourceURL; }
    const std::string& consumerName() const { return _consumerName; }
    unsigned int headerRetryInterval() const { return _headerRetryInterval; }
    double maxEventRequestRate() const { return _maxEventRequestRate; }
    const std::string& topLevelFolderName() const { return _topLevelFolderName; }

    // Output:
    friend std::ostream& operator <<
      ( std::ostream&, const DQMEventConsumerRegistrationInfo& );

  private:

    std::string _sourceURL;
    std::string _consumerName;
    unsigned int _headerRetryInterval;
    double _maxEventRequestRate;
    std::string _topLevelFolderName;

  };
  
} // namespace stor

#endif
