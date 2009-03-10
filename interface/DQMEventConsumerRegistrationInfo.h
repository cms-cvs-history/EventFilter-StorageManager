// -*- c++ -*-
// $Id: DQMEventConsumerRegistrationInfo.h,v 1.1.2.4 2009/03/10 12:37:50 dshpakov Exp $

#ifndef DQMEVENTCONSUMERREGISTRATIONINFO_H
#define DQMEVENTCONSUMERREGISTRATIONINFO_H

#include <string>
#include <iostream>

#include "EventFilter/StorageManager/interface/EnquingPolicyTag.h"
#include "EventFilter/StorageManager/interface/QueueID.h"

namespace stor
{
  /**
   * Holds the registration information for a DQM event consumer.
   *
   * $Author: dshpakov $
   * $Revision: 1.1.2.4 $
   * $Date: 2009/03/10 12:37:50 $
   */

  class DQMEventConsumerRegistrationInfo
  {

  public:

    // Constructor:
    DQMEventConsumerRegistrationInfo( const std::string& sourceURL,
				      const std::string& consumerName,
				      unsigned int headerRetryInterval, // seconds
				      double maxEventRequestRate, // Hz
				      const std::string& topLevelFolderName,
				      enquing_policy::PolicyTag policy ):
      _sourceURL( sourceURL ),
      _consumerName( consumerName ),
      _headerRetryInterval( headerRetryInterval ),
      _maxEventRequestRate( maxEventRequestRate ),
      _topLevelFolderName( topLevelFolderName ),
      _policy( policy )
    {}

    // Destructor:
    ~DQMEventConsumerRegistrationInfo() {}

    // Accessors:
    const std::string& sourceURL() const { return _sourceURL; }
    const std::string& consumerName() const { return _consumerName; }
    unsigned int headerRetryInterval() const { return _headerRetryInterval; }
    double maxEventRequestRate() const { return _maxEventRequestRate; }
    const std::string& topLevelFolderName() const { return _topLevelFolderName; }
    const QueueID& queueId() const { return _queueId; }
    const enquing_policy::PolicyTag& policy() const { return _policy; }

    // Set queue Id:
    void setQueueId( QueueID qid ) { _queueId = qid; }

    // Output:
    friend std::ostream& operator <<
      ( std::ostream&, const DQMEventConsumerRegistrationInfo& );

  private:

    std::string _sourceURL;
    std::string _consumerName;
    unsigned int _headerRetryInterval;
    double _maxEventRequestRate;
    std::string _topLevelFolderName;
    QueueID _queueId;
    enquing_policy::PolicyTag _policy;

  };
  
} // namespace stor

#endif
