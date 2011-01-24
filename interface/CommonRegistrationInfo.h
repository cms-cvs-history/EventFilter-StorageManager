// $Id: CommonRegistrationInfo.h,v 1.4.8.2 2011/01/19 13:50:38 mommsen Exp $
/// @file: CommonRegistrationInfo.h 

#ifndef EventFilter_StorageManager_CommonRegistrationInfo_h
#define EventFilter_StorageManager_CommonRegistrationInfo_h


#include <iosfwd>
#include <string>

#include "EventFilter/StorageManager/interface/Configuration.h"
#include "EventFilter/StorageManager/interface/ConsumerID.h"
#include "EventFilter/StorageManager/interface/QueueID.h"
#include "EventFilter/StorageManager/interface/Utils.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

namespace stor
{
  /**
   * This struct holds the registration information sufficient to
   * implement the RegistrationInfoBase interface. Derived classes
   * that don't have reason to do otherwise should include this as a
   * data member.
   *
   * $Author: mommsen $
   * $Revision: 1.4.8.2 $
   * $Date: 2011/01/19 13:50:38 $
   */

  struct CommonRegistrationInfo
  {
    CommonRegistrationInfo
    (
      const std::string& consumerName,
      const std::string& remoteHost,
      const int& queueSize,
      const enquing_policy::PolicyTag& queuePolicy,
      const utils::duration_t& secondsToStale
    );

    CommonRegistrationInfo
    (
      const std::string& consumerName,
      const std::string& remoteHost,
      const edm::ParameterSet& pset,
      const EventServingParams& eventServingParams,
      const bool useEventServingParams = true
    );
    
    const std::string                _consumerName;
    const std::string                _remoteHost;
    int                              _queueSize;
    enquing_policy::PolicyTag        _queuePolicy;
    utils::duration_t                _secondsToStale;
    QueueID                          _queueId;
    ConsumerID                       _consumerId;
  };

  // If changing the print order of things in
  // EventConsumerRegistrationInfo is acceptable, the following could
  // be used to implement EventConsumerRegistrationInfo::write()

  std::ostream& operator<<(std::ostream& os, 
                           CommonRegistrationInfo const& ri);

  
} // namespace stor

#endif // EventFilter_StorageManager_CommonRegistrationInfo_h


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
