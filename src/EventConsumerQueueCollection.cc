// $Id$

#include "EventFilter/StorageManager/interface/EventConsumerQueueCollection.h"

using namespace stor;


EventConsumerQueueCollection::EventConsumerQueueCollection()
{

}


EventConsumerQueueCollection::~EventConsumerQueueCollection()
{

}


const QueueID EventConsumerQueueCollection::registerEventConsumer
(
  boost::shared_ptr<EventConsumerRegistrationInfo>
)
{
  return 0;
}


void EventConsumerQueueCollection::addEvent(I2OChain &chain)
{

}


I2OChain EventConsumerQueueCollection::popEvent(const QueueID id)
{
  return _collection[id]->popEvent();
}


void EventConsumerQueueCollection::disposeOfStaleStuff()
{

}



/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
