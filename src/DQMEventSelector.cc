#include "EventFilter/StorageManager/interface/DQMEventSelector.h"

using namespace stor;

bool DQMEventSelector::acceptEvent( const I2OChain& ioc )
{
  if( _regInfo.topLevelFolderName() == std::string( "*" ) ) return true;
  if( ioc.topFolderName() == _regInfo.topLevelFolderName() ) return true;
  return false;
}
