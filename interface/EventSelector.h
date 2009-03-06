// -*- c++ -*-
// $Id: EventSelector.h,v 1.1.2.4 2009/03/05 22:29:48 biery Exp $

#ifndef EVENTSELECTOR_H
#define EVENTSELECTOR_H

#include <boost/shared_ptr.hpp>

#include "EventFilter/StorageManager/interface/I2OChain.h"
#include "FWCore/Framework/interface/EventSelector.h"
#include "IOPool/Streamer/interface/InitMessage.h"

namespace stor {

  template <class INFO_TYPE>
  class EventSelector
  {

  public:

    // Constructor:
    EventSelector( const INFO_TYPE& configInfo ):
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
    const INFO_TYPE& configInfo() const { return _configInfo; }
    bool isInitialized() const { return _initialized; }

  private:

    bool _initialized;
    unsigned int _outputModuleId;
    INFO_TYPE _configInfo;

    boost::shared_ptr<edm::EventSelector> _eventSelector;

  };

  //------------------------------------------------------------------
  // Implementation follows
  //------------------------------------------------------------------

  template <class INFO_TYPE>
  void EventSelector<INFO_TYPE>::initialize( const InitMsgView& imv )
  {

    if( _initialized ) return;

    if( _configInfo.selHLTOut() != imv.outputModuleLabel() ) return; 

    _outputModuleId = imv.outputModuleId();

    edm::ParameterSet pset;
    pset.addParameter<Strings>( "SelectEvents", _configInfo.selEvents() );

    Strings tnames;
    imv.hltTriggerNames( tnames );
    _eventSelector.reset( new edm::EventSelector( pset, tnames ) );

    _initialized = true;

  }

  template <class INFO_TYPE>
  bool EventSelector<INFO_TYPE>::acceptEvent( const I2OChain& ioc )
  {

    if( !_initialized ) return false;

    if( ioc.outputModuleId() != _outputModuleId ) return false;

    std::vector<unsigned char> hlt_out;
    ioc.hltTriggerBits( hlt_out );

    return _eventSelector->wantAll()
      || _eventSelector->acceptEvent( &hlt_out[0], ioc.hltTriggerCount() );

  }

} // namespace stor

#endif
