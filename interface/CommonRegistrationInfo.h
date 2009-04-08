// -*- c++ -*-
// $Id: CommonRegistrationInfo.h,v 1.1.2.2 2009/04/03 12:22:07 dshpakov Exp $

#ifndef EventFilter_StorageManager_CommonRegistrationInfo_h
#define EventFilter_StorageManager_CommonRegistrationInfo_h


#include <iosfwd>
#include <string>

#include "EventFilter/StorageManager/interface/QueueID.h"
#include "EventFilter/StorageManager/interface/ConsumerID.h"

namespace stor
{
  /**
   * This struct holds the registration information sufficient to
   * implement the RegistrationInfoBase interface. Derived classes
   * that don't have reason to do otherwise should include this as a
   * data member.
   *
   * $Author: dshpakov $
   * $Revision: 1.1.2.2 $
   * $Date: 2009/04/03 12:22:07 $
   */

  struct CommonRegistrationInfo
  {
    CommonRegistrationInfo( std::string const& consumerName,
			    unsigned int headerRetryInterval,
			    double maxEventRequestRate,
			    QueueID queueID );

    std::string   consumerName;
    unsigned int  headerRetryInterval;
    double        maxEventRequestRate;
    QueueID       queueId;
    ConsumerID    consumerId;
  };

  // If changing the print order of things in
  // EventConsumerRegistrationInfo is acceptable, the following could
  // be used to implement EventConsumerRegistrationInfo::write()

  std::ostream& operator<<(std::ostream& os, 
			   CommonRegistrationInfo const& ri);

  
} // namespace stor

#endif
