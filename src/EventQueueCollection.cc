// $Id: EventQueueCollection.cc,v 1.1.2.3 2009/03/10 15:34:56 mommsen Exp $

#include "EventFilter/StorageManager/interface/EnquingPolicyTag.h"
#include "EventFilter/StorageManager/interface/EventQueueCollection.h"

namespace stor
{
  EventQueueCollection::EventQueueCollection() :
    _protect_discard_new_queues(),
    _protect_discard_old_queues(),
    _discard_new_queues(),
    _discard_old_queues()
  { }

  QueueID 
  EventQueueCollection::createQueue(enquing_policy::PolicyTag policy,
				    size_t max)
  {
    QueueID result;
    if (policy == enquing_policy::DiscardNew)
      {
	write_lock_t lock(_protect_discard_new_queues);
        discard_new_queue_ptr newborn(new discard_new_queue_t(max));
	_discard_new_queues.push_back(newborn);
	result = QueueID(enquing_policy::DiscardNew,
			 _discard_new_queues.size());
      }
    else if (policy == enquing_policy::DiscardOld)
      {
	write_lock_t lock(_protect_discard_old_queues);
        discard_old_queue_ptr newborn(new discard_old_queue_t(max));
	_discard_old_queues.push_back(newborn);
	result = QueueID(enquing_policy::DiscardOld,
			 _discard_old_queues.size());

      }
    return result;
  }




  void 
  EventQueueCollection::addEvent(I2OChain const& event)
  {
    
  }


  I2OChain 
  EventQueueCollection::popEvent(QueueID id)
  {
    I2OChain result;
    if ( id.policy() == enquing_policy::DiscardNew )
    {
      read_lock_t lock(_protect_discard_new_queues);
      _discard_new_queues[id.index()]->deq_nowait(result);
    }
    else if ( id.policy() == enquing_policy::DiscardOld )
    {
      read_lock_t lock(_protect_discard_old_queues);
      _discard_old_queues[id.index()]->deq_nowait(result);
    }
    else
    {
      // do what?
    }
    return result;
  }

  void
  EventQueueCollection::clearQueue(QueueID id)
  {
  } 

  void 
  EventQueueCollection::expireStaleQueues()
  {
  }


}

/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
