// $Id: EventConsumerQueueCollection.cc,v 1.1.2.1 2009/01/30 10:49:57 mommsen Exp $

#include "EventFilter/StorageManager/interface/EventConsumerQueueCollection.h"

namespace stor
{

  QueueID 
  EventConsumerQueueCollection::registerConsumer(EventConsumerRegistrationInfo const& ri) 
  {
    // This implementation is wrong.
    return 0;
  }


  void 
  EventConsumerQueueCollection::addEvent(I2OChain const& event)
  {
    
  }


  I2OChain 
  EventConsumerQueueCollection::popEvent(QueueID id)
  {
    // This implementation is wrong.
    read_lock_t lock(_protect_keep_new_queues);
    I2OChain result;
    _keep_new_queues[id].deq_nowait(result);
    return result;
  }

  void
  EventConsumerQueueCollection::clearQueue(QueueID id)
  {
  } 

  void 
  EventConsumerQueueCollection::expireStaleQueues()
  {
  }


}

/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
