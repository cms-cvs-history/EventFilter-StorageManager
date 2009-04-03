// -*- c++ -*-
// $Id: CommonRegistrationInfo.h,v 1.1.2.1 2009/04/01 18:44:55 paterno Exp $

#ifndef EventFilter_StorageManager_CommonRegistrationInfo_h
#define EventFilter_StorageManager_CommonRegistrationInfo_h


#include <iosfwd>
#include <string>

#include "EventFilter/StorageManager/interface/QueueID.h"

namespace stor
{
  /**
   * This struct holds the registration information sufficient to
   * implement the RegistrationInfoBase interface. Derived classes
   * that don't have reason to do otherwise should include this as a
   * data member.
   *
   * $Author: paterno $
   * $Revision: 1.1.2.1 $
   * $Date: 2009/04/01 18:44:55 $
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
  };

  // If changing the print order of things in
  // EventConsumerRegistrationInfo is acceptable, the following could
  // be used to implement EventConsumerRegistrationInfo::write()

  std::ostream& operator<<(std::ostream& os, 
			   CommonRegistrationInfo const& ri);

  
} // namespace stor

#endif
