// $Id: EventConsumerMonitorCollection.cc,v 1.2.12.2 2011/02/28 17:56:06 mommsen Exp $
/// @file: EventConsumerMonitorCollection.cc

#include "EventFilter/StorageManager/interface/EventConsumerMonitorCollection.h"
#include "EventFilter/StorageManager/interface/QueueID.h"

using namespace stor;


EventConsumerMonitorCollection::EventConsumerMonitorCollection(const utils::Duration_t& updateInterval):
ConsumerMonitorCollection(updateInterval, boost::posix_time::seconds(10))
{}


void EventConsumerMonitorCollection::do_appendInfoSpaceItems(InfoSpaceItems& infoSpaceItems)
{
  infoSpaceItems.push_back(std::make_pair("eventConsumers", &eventConsumers_));
}


void EventConsumerMonitorCollection::do_updateInfoSpaceItems()
{
  boost::mutex::scoped_lock l( mutex_ );
  eventConsumers_ = smap_.size();
}



/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
