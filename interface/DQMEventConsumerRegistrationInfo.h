// -*- c++ -*-
// $Id: DQMEventConsumerRegistrationInfo.h,v 1.1.2.12 2009/04/27 17:00:18 mommsen Exp $

#ifndef DQMEVENTCONSUMERREGISTRATIONINFO_H
#define DQMEVENTCONSUMERREGISTRATIONINFO_H

#include <iosfwd>
#include <string>

#include "EventFilter/StorageManager/interface/RegistrationInfoBase.h"
#include "EventFilter/StorageManager/interface/CommonRegistrationInfo.h"
#include "EventFilter/StorageManager/interface/Utils.h"

namespace stor
{
  /**
   * Holds the registration information for a DQM event consumer.
   *
   * $Author: mommsen $
   * $Revision: 1.1.2.12 $
   * $Date: 2009/04/27 17:00:18 $
   */

  class DQMEventConsumerRegistrationInfo : public RegistrationInfoBase
  {
  public:

    /**
     * Constructs an instance from the specified registration information.
     */
    DQMEventConsumerRegistrationInfo( const std::string& consumerName,
                                      const std::string& topLevelFolderName,
                                      const size_t& queueSize,
                                      const enquing_policy::PolicyTag& policy,
                                      const utils::duration_t& secondsToStale );

    // Destructor:
    ~DQMEventConsumerRegistrationInfo();

    // Additional accessors:
    const std::string& topLevelFolderName() const { return _topLevelFolderName; }

    // Output:
    std::ostream& write(std::ostream& os) const;

    // Implementation of the Template Method pattern.
    virtual void do_registerMe(EventDistributor*);
    virtual QueueID do_queueId() const;
    virtual void do_setQueueID(QueueID const& id);
    virtual std::string do_consumerName() const;
    virtual ConsumerID do_consumerId() const;
    virtual void do_setConsumerID(ConsumerID const& id);
    virtual size_t do_queueSize() const;
    virtual enquing_policy::PolicyTag do_queuePolicy() const;
    virtual utils::duration_t do_secondsToStale() const;


  private:

    CommonRegistrationInfo _common;

    std::string _topLevelFolderName;
  };

  typedef boost::shared_ptr<stor::DQMEventConsumerRegistrationInfo> DQMEventConsRegPtr;


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


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
