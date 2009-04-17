// $Id: DQMEventStore.cc,v 1.1.2.6 2009/02/24 14:04:22 biery Exp $

#include "EventFilter/StorageManager/interface/DQMEventStore.h"
#include "EventFilter/StorageManager/interface/Utils.h"

using namespace stor;


void DQMEventStore::addDQMEvent(I2OChain const& chain)
{
}


bool DQMEventStore::getCompletedDQMRecordIfAvailable(DQMRecord& record)
{
  return false;
}



/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
