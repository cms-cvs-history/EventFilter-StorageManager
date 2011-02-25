// $Id: EventConsumerRegistrationInfo.h,v 1.13.2.11 2011/02/24 13:37:13 mommsen Exp $
/// @file: EventConsumerRegistrationInfo.h 

#ifndef EventFilter_StorageManager_EventConsumerRegistrationInfo_h
#define EventFilter_StorageManager_EventConsumerRegistrationInfo_h

#include <iosfwd>
#include <string>

#include <boost/shared_ptr.hpp>

#include "toolbox/net/Utils.h"

#include "EventFilter/StorageManager/interface/Configuration.h"
#include "EventFilter/StorageManager/interface/RegistrationInfoBase.h"
#include "EventFilter/StorageManager/interface/Utils.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "IOPool/Streamer/interface/HLTInfo.h"
#include "IOPool/Streamer/interface/MsgHeader.h"


namespace stor
{
  /**
   * Holds the registration information from a event consumer.
   *
   * $Author: mommsen $
   * $Revision: 1.13.2.11 $
   * $Date: 2011/02/24 13:37:13 $
   */

  class EventConsumerRegistrationInfo: public RegistrationInfoBase
  {

  public:
    
    EventConsumerRegistrationInfo
    (
      const edm::ParameterSet& pset,
      const EventServingParams& eventServingParams,
      const std::string& remoteHost = toolbox::net::getHostName()
    );

    EventConsumerRegistrationInfo
    (
      const edm::ParameterSet& pset,
      const std::string& remoteHost = toolbox::net::getHostName()
    );

    ~EventConsumerRegistrationInfo() {};

    // Setters:
    void setMinEventRequestInterval(const utils::duration_t& interval) { _minEventRequestInterval= interval; }

    // Accessors:
    const std::string& triggerSelection() const { return _triggerSelection; }
    const Strings& eventSelection() const { return _eventSelection; }
    const std::string& outputModuleLabel() const { return _outputModuleLabel; }
    const int& prescale() const { return _prescale; }
    const bool& uniqueEvents() const { return _uniqueEvents; }
    const utils::duration_t& minEventRequestInterval() const { return _minEventRequestInterval; }
    const int& headerRetryInterval() const { return _headerRetryInterval; }
    uint32 eventRequestCode() const { return Header::EVENT_REQUEST; }
    uint32 eventCode() const { return Header::EVENT; }
    std::string eventURL() const { return sourceURL() + "/geteventdata"; }
    std::string registerURL() const { return sourceURL() + "/registerConsumer"; }

    // Comparison:
    bool operator<(const EventConsumerRegistrationInfo&) const;
    bool operator==(const EventConsumerRegistrationInfo&) const;
    bool operator!=(const EventConsumerRegistrationInfo&) const;

    // Output:
    std::ostream& write(std::ostream& os) const;

    // Implementation of Template Method pattern.
    virtual void do_registerMe(EventDistributor*);
    virtual void do_eventType(std::ostream&) const;
    virtual void do_appendToPSet(edm::ParameterSet&) const;

  private:

    void parsePSet(const edm::ParameterSet&);

    std::string _triggerSelection;
    Strings _eventSelection;
    std::string _outputModuleLabel;
    int _prescale;
    bool _uniqueEvents;
    utils::duration_t _minEventRequestInterval;
    int _headerRetryInterval;
  };

  typedef boost::shared_ptr<stor::EventConsumerRegistrationInfo> EventConsRegPtr;

} // namespace stor

#endif // EventFilter_StorageManager_EventConsumerRegistrationInfo_h

/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
