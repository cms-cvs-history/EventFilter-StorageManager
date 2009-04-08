// -*- c++ -*-
// $Id: DQMEventConsumerRegistrationInfo.h,v 1.1.2.9 2009/04/03 12:22:08 dshpakov Exp $

#ifndef DQMEVENTCONSUMERREGISTRATIONINFO_H
#define DQMEVENTCONSUMERREGISTRATIONINFO_H

#include <iosfwd>
#include <string>

#include "EventFilter/StorageManager/interface/RegistrationInfoBase.h"
#include "EventFilter/StorageManager/interface/CommonRegistrationInfo.h"

namespace stor
{
  /**
   * Holds the registration information for a DQM event consumer.
   *
   * $Author: dshpakov $
   * $Revision: 1.1.2.9 $
   * $Date: 2009/04/03 12:22:08 $
   */

  class DQMEventConsumerRegistrationInfo : public RegistrationInfoBase
  {
  public:

    /**
     * Constructs an instance from the specified registration information.
     */
    DQMEventConsumerRegistrationInfo( const std::string& consumerName,
				      unsigned int headerRetryInterval,// seconds
				      double maxEventRequestRate, // Hz
				      const std::string& topLevelFolderName,
				      QueueID queueId,
				      size_t maxQueueSize );

    // Destructor:
    ~DQMEventConsumerRegistrationInfo();

    // Additional accessors:
    const std::string& topLevelFolderName() const { return _topLevelFolderName; }
    size_t maxQueueSize() const { return _maxQueueSize; }

    // Output:
    std::ostream& write(std::ostream& os) const;

    // Implementation of the Template Method pattern.
    virtual void do_registerMe(EventDistributor*);
    virtual QueueID do_queueId() const;
    virtual std::string do_consumerName() const;
    virtual ConsumerID   do_consumerID() const;
    virtual void         do_setConsumerID(ConsumerID id);
    virtual unsigned int do_headerRetryInterval() const;
    virtual double       do_maxEventRequestRate() const;

  private:

    CommonRegistrationInfo _common;

    std::string _topLevelFolderName;
    size_t _maxQueueSize;

  };

  /**
     Print the given DQMEventConsumerRegistrationInfo to the given
     stream.
  */
  inline
  std::ostream& operator<<(std::ostream& os, 
			   const DQMEventConsumerRegistrationInfo& ri)
  {
    return ri.write(os);
  }
  
} // namespace stor

#endif
