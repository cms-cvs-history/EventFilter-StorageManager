// $Id: EventConsumerQueueCollection.cc,v 1.1.2.2 2009/03/02 17:44:46 paterno Exp $

#include "EventFilter/StorageManager/interface/EnquingPolicyTag.h"
#include "EventFilter/StorageManager/interface/EventConsumerQueueCollection.h"

namespace stor
{

  QueueID 
  EventConsumerQueueCollection::registerConsumer(EventConsumerRegistrationInfo const& ri) 
  {
    // This implementation is wrong.
    QueueID queue;
    return queue;
  }


  void 
  EventConsumerQueueCollection::addEvent(I2OChain const& event)
  {
    
  }


  I2OChain 
  EventConsumerQueueCollection::popEvent(QueueID id)
  {
    I2OChain result;
    if ( id.policy() == enquing_policy::DiscardNew )
    {
      read_lock_t lock(_protect_reject_new_queues);
      _reject_new_queues[id.index()].deq_nowait(result);
    }
    else if ( id.policy() == enquing_policy::DiscardOld )
    {
      read_lock_t lock(_protect_keep_new_queues);
      _keep_new_queues[id.index()].deq_nowait(result);
    }
    else
    {
      // do what?
    }
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
