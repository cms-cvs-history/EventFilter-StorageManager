// -*- c++ -*-
// $Id: EventSelector.h,v 1.1.2.3 2009/03/04 12:56:47 dshpakov Exp $

#ifndef EVENTSELECTOR_H
#define EVENTSELECTOR_H

#include <boost/shared_ptr.hpp>

#include "FWCore/Framework/interface/EventSelector.h"
#include "IOPool/Streamer/interface/InitMessage.h"

#include "EventFilter/StorageManager/interface/EventStreamConfigurationInfo.h"
#include "EventFilter/StorageManager/interface/I2OChain.h"

//#include "EventFilter/StorageManager/interface/InitMsgCollection.h"

namespace stor {

  class EventSelector
  {

  public:

    // Constructor:
    EventSelector( const EventStreamConfigurationInfo& configInfo ):
      _initialized( false ),
      _outputModuleId(0),
      _configInfo( configInfo )
    {}

    // Destructor:
    ~EventSelector() {}

    // Initialize:
    void initialize( const InitMsgView& );

    // Accept event:
    bool acceptEvent( const I2OChain& );

    // Accessors:
    unsigned int outputModuleId() const { return _outputModuleId; }
    const EventStreamConfigurationInfo& configInfo() const { return _configInfo; }

  private:

    bool _initialized;
    unsigned int _outputModuleId;
    EventStreamConfigurationInfo _configInfo;

    boost::shared_ptr<edm::EventSelector> _eventSelector;

  };

} // namespace stor

#endif
