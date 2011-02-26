// $Id: DQMEventSelector.cc,v 1.5.2.1 2011/01/24 14:03:20 mommsen Exp $
/// @file: DQMEventSelector.cc

#include "EventFilter/StorageManager/interface/DQMEventSelector.h"

using namespace stor;

bool DQMEventSelector::acceptEvent
(
  const I2OChain& ioc,
  const utils::time_point_t& now
)
{
  if( _registrationInfo->isStale(now) ) return false;
  if( _registrationInfo->topLevelFolderName() == std::string( "*" ) ) return true;
  if( _registrationInfo->topLevelFolderName() == ioc.topFolderName() ) return true;
  return false;
}


bool DQMEventSelector::operator<(const DQMEventSelector& other) const
{
  if ( queueId() != other.queueId() )
    return ( queueId() < other.queueId() );
  return ( *(_registrationInfo) < *(other._registrationInfo) );
}



/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
