// -*- c++ -*-
// $Id: CommonRegistrationInfo.h,v 1.1.2.7 2009/03/10 21:19:38 biery Exp $

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
   * $Author: biery $
   * $Revision: 1.1.2.7 $
   * $Date: 2009/03/10 21:19:38 $
   */

  struct CommonRegistrationInfo
  {
    CommonRegistrationInfo(std::string const& sourceURL,
			   std::string const& consumerName,
			   unsigned int headerRetryInterval,
			   double maxEventRequestRate,
			   QueueID queueID);

    std::string   sourceURL;
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
