// $Id: $

#include <vector>

#include "EventFilter/StorageManager/interface/EventSelector.h"

using namespace stor;

void EventSelector::initialize( const InitMsgView& imv )
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

bool EventSelector::acceptEvent( const I2OChain& ioc )
{

  if( !_initialized ) return false;

  if( ioc.outputModuleId() != _outputModuleId ) return false;

  std::vector<unsigned char> hlt_out;
  ioc.hltTriggerBits( hlt_out );

  return _eventSelector->wantAll()
    || _eventSelector->acceptEvent( &hlt_out[0], ioc.hltTriggerCount() );

}
