// -*- c++ -*-
// $Id: EventConsumerSelector.h,v 1.1.2.5 2009/03/06 22:09:47 biery Exp $

#ifndef EVENTCONSUMERSELECTOR_H
#define EVENTCONSUMERSELECTOR_H

#include <boost/shared_ptr.hpp>

#include "EventFilter/StorageManager/interface/EventConsumerRegistrationInfo.h"
#include "EventFilter/StorageManager/interface/I2OChain.h"
#include "FWCore/Framework/interface/EventSelector.h"
#include "IOPool/Streamer/interface/InitMessage.h"

namespace stor {

  class EventConsumerSelector
  {

  public:

    // Constructor:
    EventConsumerSelector( const EventConsumerRegistrationInfo& configInfo ):
      _initialized( false ),
      _outputModuleId(0),
      _configInfo( configInfo )
    {}

    // Destructor:
    ~EventConsumerSelector() {}

    // Initialize:
    void initialize( const InitMsgView& );

    // Accept event:
    bool acceptEvent( const I2OChain& );

    // Accessors:
    unsigned int outputModuleId() const { return _outputModuleId; }
    const EventConsumerRegistrationInfo& configInfo() const { return _configInfo; }
    bool isInitialized() const { return _initialized; }

  private:

    bool _initialized;
    unsigned int _outputModuleId;
    EventConsumerRegistrationInfo _configInfo;

    boost::shared_ptr<edm::EventSelector> _eventSelector;

  };

} // namespace stor

#endif
