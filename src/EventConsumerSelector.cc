// $Id: EventConsumerSelector.cc,v 1.1.2.1 2009/03/09 20:30:59 biery Exp $

#include <vector>

#include "EventFilter/StorageManager/interface/EventConsumerSelector.h"

using namespace stor;

void EventConsumerSelector::initialize( const InitMsgView& imv )
{

  if( _initialized ) return;

  if( _outputModuleLabel != imv.outputModuleLabel() ) return; 

  _outputModuleId = imv.outputModuleId();

  edm::ParameterSet pset;
  pset.addParameter<Strings>( "SelectEvents", _eventSelectionStrings );

  Strings tnames;
  imv.hltTriggerNames( tnames );
  _eventSelector.reset( new edm::EventSelector( pset, tnames ) );

  _initialized = true;

}

bool EventConsumerSelector::acceptEvent( const I2OChain& ioc )
{

  if( !_initialized ) return false;

  if( ioc.outputModuleId() != _outputModuleId ) return false;

  std::vector<unsigned char> hlt_out;
  ioc.hltTriggerBits( hlt_out );

  return _eventSelector->wantAll()
    || _eventSelector->acceptEvent( &hlt_out[0], ioc.hltTriggerCount() );

}
