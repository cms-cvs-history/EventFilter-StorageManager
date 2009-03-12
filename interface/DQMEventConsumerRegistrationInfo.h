// -*- c++ -*-
// $Id: DQMEventConsumerRegistrationInfo.h,v 1.1.2.6 2009/03/10 21:19:38 biery Exp $

#ifndef DQMEVENTCONSUMERREGISTRATIONINFO_H
#define DQMEVENTCONSUMERREGISTRATIONINFO_H

#include <string>
#include <iostream>

#include "EventFilter/StorageManager/interface/RegistrationInfoBase.h"

namespace stor
{
  /**
   * Holds the registration information for a DQM event consumer.
   *
   * $Author: biery $
   * $Revision: 1.1.2.6 $
   * $Date: 2009/03/10 21:19:38 $
   */

  class DQMEventConsumerRegistrationInfo : public RegistrationInfoBase
  {

  public:

    /**
     * Constructs an instance from the specified registration information.
     */
    DQMEventConsumerRegistrationInfo( const std::string& sourceURL,
				      const std::string& consumerName,
				      unsigned int headerRetryInterval, // seconds
				      double maxEventRequestRate, // Hz
				      const std::string& topLevelFolderName,
				      enquing_policy::PolicyTag policy,
				      size_t maxQueueSize):
      _sourceURL( sourceURL ),
      _consumerName( consumerName ),
      _headerRetryInterval( headerRetryInterval ),
      _maxEventRequestRate( maxEventRequestRate ),
      _topLevelFolderName( topLevelFolderName ),
      _policy( policy ),
      _maxQueueSize( maxQueueSize )
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
    const enquing_policy::PolicyTag& queuePolicy() const { return _policy; }
    size_t maxQueueSize() const { return _maxQueueSize; }

    // Set queue Id:
    void setQueueId( QueueID qid ) { _queueId = qid; }

    /**
     * Registers the consumer represented by this registration with
     * the specified EventDistributor.
     */
    void registerMe(EventDistributor*);

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
    size_t _maxQueueSize;

  };
  
} // namespace stor

#endif
