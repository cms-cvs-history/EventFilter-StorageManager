// $Id: EventConsumerRegistrationInfo.h,v 1.13.2.6 2011/01/26 14:28:28 mommsen Exp $
/// @file: EventConsumerRegistrationInfo.h 

#ifndef EventFilter_StorageManager_EventConsumerRegistrationInfo_h
#define EventFilter_StorageManager_EventConsumerRegistrationInfo_h

#include <iosfwd>
#include <string>
#include <vector>

#include "boost/shared_ptr.hpp"

#include "EventFilter/StorageManager/interface/CommonRegistrationInfo.h"
#include "EventFilter/StorageManager/interface/Configuration.h"
#include "EventFilter/StorageManager/interface/Exception.h"
#include "EventFilter/StorageManager/interface/RegistrationInfoBase.h"
#include "EventFilter/StorageManager/interface/Utils.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "IOPool/Streamer/interface/HLTInfo.h"

namespace stor
{
  /**
   * Holds the registration information from a event consumer.
   *
   * $Author: mommsen $
   * $Revision: 1.13.2.6 $
   * $Date: 2011/01/26 14:28:28 $
   */

  class EventConsumerRegistrationInfo: public RegistrationInfoBase
  {

  public:
    
    EventConsumerRegistrationInfo
    (
      const std::string& consumerName,
      const std::string& remoteHost,
      const std::string& triggerSelection,
      const Strings& eventSelection,
      const std::string& outputModuleLabel,
      const int& prescale,
      const bool& uniqueEvents,
      const int& queueSize,
      const enquing_policy::PolicyTag& queuePolicy,
      const utils::duration_t& secondsToStale,
      const utils::duration_t& minEventRequestInterval = boost::posix_time::not_a_date_time
    );

    EventConsumerRegistrationInfo
    (
      const std::string& consumerName,
      const std::string& remoteHost,
      const edm::ParameterSet& pset,
      const EventServingParams& eventServingParams
    );

    EventConsumerRegistrationInfo
    (
      const std::string& consumerName,
      const std::string& remoteHost,
      const edm::ParameterSet& pset
    );

    ~EventConsumerRegistrationInfo();

    // Accessors:
    const std::string& triggerSelection() const { return _triggerSelection; }
    const Strings& eventSelection() const { return _eventSelection; }
    const std::string& outputModuleLabel() const { return _outputModuleLabel; }
    const int& prescale() const { return _prescale; }
    const bool& uniqueEvents() const { return _uniqueEvents; }
    const utils::duration_t& minEventRequestInterval() const { return _minEventRequestInterval; }
    edm::ParameterSet getPSet() const;

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

    void parsePSet(const edm::ParameterSet&);

    CommonRegistrationInfo _common;

    std::string _triggerSelection;
    Strings _eventSelection;
    std::string _outputModuleLabel;
    int _prescale;
    bool _uniqueEvents;
    utils::duration_t _minEventRequestInterval;
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

#endif // EventFilter_StorageManager_EventConsumerRegistrationInfo_h

/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
