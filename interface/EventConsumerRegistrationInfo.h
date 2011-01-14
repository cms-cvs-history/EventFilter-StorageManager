// $Id: EventConsumerRegistrationInfo.h,v 1.13.2.2 2011/01/13 13:28:41 mommsen Exp $
/// @file: EventConsumerRegistrationInfo.h 

#ifndef StorageManager_EventConsumerRegistrationInfo_h
#define StorageManager_EventConsumerRegistrationInfo_h

#include <iosfwd>
#include <string>
#include <vector>

#include "boost/shared_ptr.hpp"

#include "IOPool/Streamer/interface/HLTInfo.h"
#include "EventFilter/StorageManager/interface/RegistrationInfoBase.h"
#include "EventFilter/StorageManager/interface/CommonRegistrationInfo.h"
#include "EventFilter/StorageManager/interface/Utils.h"

namespace stor
{
  /**
   * Holds the registration information from a event consumer.
   *
   * $Author: mommsen $
   * $Revision: 1.13.2.2 $
   * $Date: 2011/01/13 13:28:41 $
   */

  class EventConsumerRegistrationInfo: public RegistrationInfoBase
  {

  public:

    /**
     * Constructs an instance with the specified registration information.
     */
    EventConsumerRegistrationInfo( const std::string& consumerName,
                                   const std::string& remoteHost,
                                   const std::string& triggerSelection,
                                   const Strings& eventSelection,
                                   const std::string& outputModuleLabel,
                                   const unsigned int& prescale,
                                   const bool& uniqueEvents,
                                   const int& queueSize,
                                   const enquing_policy::PolicyTag& queuePolicy,
                                   const utils::duration_t& secondsToStale );

    ~EventConsumerRegistrationInfo();

    // Accessors:
    const std::string& triggerSelection() const { return _triggerSelection; }
    const Strings& eventSelection() const { return _eventSelection; }
    const std::string& outputModuleLabel() const { return _outputModuleLabel; }
    const unsigned int& prescale() const { return _prescale; }
    const bool& uniqueEvents() const { return _uniqueEvents; }

    // Comparison:
    bool operator<(const EventConsumerRegistrationInfo&) const;
    bool operator==(const EventConsumerRegistrationInfo&) const;
    bool operator!=(const EventConsumerRegistrationInfo&) const;

    // Output:
    std::ostream& write(std::ostream& os) const;

    // Implementation of Template Method pattern.
    virtual void do_registerMe(EventDistributor*);
    virtual QueueID do_queueId() const;
    virtual void do_setQueueId(QueueID const& id);
    virtual std::string do_consumerName() const;
    virtual std::string do_remoteHost() const;
    virtual ConsumerID do_consumerId() const;
    virtual void do_setConsumerId(ConsumerID const& id);
    virtual int do_queueSize() const;
    virtual enquing_policy::PolicyTag do_queuePolicy() const;
    virtual utils::duration_t do_secondsToStale() const;

  private:

    CommonRegistrationInfo _common;

    std::string _triggerSelection;
    Strings _eventSelection;
    std::string _outputModuleLabel;
    unsigned int _prescale;
    bool _uniqueEvents;

  };

  typedef boost::shared_ptr<stor::EventConsumerRegistrationInfo> EventConsRegPtr;

  /**
     Print the given EventConsumerRegistrationInfo to the given
     stream.
  */
  inline
  std::ostream& operator << ( std::ostream& os, 
                              EventConsumerRegistrationInfo const& ri )
  {
    return ri.write( os );
  }

} // namespace stor

#endif // StorageManager_EventConsumerRegistrationInfo_h

/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
