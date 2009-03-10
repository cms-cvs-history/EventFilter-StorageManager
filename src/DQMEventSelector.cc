#include "EventFilter/StorageManager/interface/DQMEventSelector.h"

using namespace stor;

bool DQMEventSelector::acceptEvent( const I2OChain& ioc )
{
  if( _topLevelFolderName == std::string( "*" ) ) return true;
  if( ioc.topFolderName() == _topLevelFolderName ) return true;
  return false;
}
