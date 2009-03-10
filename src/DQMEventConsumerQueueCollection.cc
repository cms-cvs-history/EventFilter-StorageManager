// $Id: DQMEventConsumerQueueCollection.cc,v 1.1.2.1 2009/01/30 10:49:56 mommsen Exp $

#include "EventFilter/StorageManager/interface/DQMEventConsumerQueueCollection.h"

using namespace stor;


DQMEventConsumerQueueCollection::DQMEventConsumerQueueCollection()
{

}


DQMEventConsumerQueueCollection::~DQMEventConsumerQueueCollection()
{

}


const QueueID DQMEventConsumerQueueCollection::registerDQMEventConsumer
(
  boost::shared_ptr<DQMEventConsumerRegistrationInfo>
)
{
  QueueID queueID;
  return queueID;
}


void DQMEventConsumerQueueCollection::addEvent(I2OChain &chain)
{

}


I2OChain DQMEventConsumerQueueCollection::popEvent(const QueueID id)
{
  return _collection[id]->popEvent();
}


void DQMEventConsumerQueueCollection::disposeOfStaleStuff()
{

}



/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
