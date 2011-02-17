// $Id: DQMEventConsumerRegistrationInfo.h,v 1.8.2.5 2011/02/11 12:10:30 mommsen Exp $
/// @file: DQMEventConsumerRegistrationInfo.h 

#ifndef EventFilter_StorageManager_DQMEventConsumerRegistrationInfo_h
#define EventFilter_StorageManager_DQMEventConsumerRegistrationInfo_h

#include <iosfwd>
#include <string>

#include "IOPool/Streamer/interface/HLTInfo.h"
#include "EventFilter/StorageManager/interface/RegistrationInfoBase.h"
#include "EventFilter/StorageManager/interface/Utils.h"

namespace stor
{
  /**
   * Holds the registration information for a DQM event consumer.
   *
   * $Author: mommsen $
   * $Revision: 1.8.2.5 $
   * $Date: 2011/02/11 12:10:30 $
   */

  class DQMEventConsumerRegistrationInfo : public RegistrationInfoBase
  {
  public:

    /**
     * Constructs an instance from the specified registration information.
     */
    DQMEventConsumerRegistrationInfo
    (
      const std::string& consumerName,
      const std::string& remoteHost,
      const std::string& topLevelFolderName,
      const int& queueSize,
      const enquing_policy::PolicyTag& policy,
      const utils::duration_t& secondsToStale
    );

    // Destructor:
    ~DQMEventConsumerRegistrationInfo();

    // Additional accessors:
    const std::string& topLevelFolderName() const { return _topLevelFolderName; }

    // Comparison:
    bool operator<(const DQMEventConsumerRegistrationInfo&) const;
    bool operator==(const DQMEventConsumerRegistrationInfo&) const;
    bool operator!=(const DQMEventConsumerRegistrationInfo&) const;

    // Output:
    std::ostream& write(std::ostream& os) const;

    // Implementation of the Template Method pattern.
    virtual void do_registerMe(EventDistributor*);
    virtual void do_eventType(std::ostream&) const;


  private:

    std::string _topLevelFolderName;
  };

  typedef boost::shared_ptr<stor::DQMEventConsumerRegistrationInfo> DQMEventConsRegPtr;
  
} // namespace stor

#endif // EventFilter_StorageManager_DQMEventConsumerRegistrationInfo_h


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
