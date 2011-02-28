// $Id: DQMConsumerMonitorCollection.cc,v 1.2.12.1 2011/02/24 14:40:37 mommsen Exp $
/// @file: DQMConsumerMonitorCollection.cc

#include "EventFilter/StorageManager/interface/DQMConsumerMonitorCollection.h"
#include "EventFilter/StorageManager/interface/QueueID.h"

using namespace stor;


DQMConsumerMonitorCollection::DQMConsumerMonitorCollection(const utils::duration_t& updateInterval):
ConsumerMonitorCollection(updateInterval, boost::posix_time::minutes(5))
{}


void DQMConsumerMonitorCollection::do_appendInfoSpaceItems(InfoSpaceItems& infoSpaceItems)
{
  infoSpaceItems.push_back(std::make_pair("dqmConsumers", &dqmConsumers_));
}


void DQMConsumerMonitorCollection::do_updateInfoSpaceItems()
{
  boost::mutex::scoped_lock l( mutex_ );
  dqmConsumers_ = smap_.size();
}



/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
