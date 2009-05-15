// $Id: DQMEventSelector.cc,v 1.1.2.4 2009/05/07 18:52:21 mommsen Exp $

#include "EventFilter/StorageManager/interface/DQMEventSelector.h"

using namespace stor;

bool DQMEventSelector::acceptEvent( const I2OChain& ioc )
{
  if( _stale ) return false;
  if( _topLevelFolderName == std::string( "*" ) ) return true;
  if( ioc.topFolderName() == _topLevelFolderName ) return true;
  return false;
}


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
