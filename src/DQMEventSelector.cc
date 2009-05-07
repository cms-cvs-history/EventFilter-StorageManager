// $Id: DQMEventSelector.h,v 1.1.2.5 2009/03/10 21:19:38 biery Exp $

#include "EventFilter/StorageManager/interface/DQMEventSelector.h"

using namespace stor;

bool DQMEventSelector::acceptEvent( const I2OChain& ioc )
{
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
