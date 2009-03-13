// $Id: EventQueueCollection.cc,v 1.1.2.3 2009/03/13 10:23:25 mommsen Exp $

#include "EventFilter/StorageManager/interface/EnquingPolicyTag.h"
#include "EventFilter/StorageManager/interface/EventQueueCollection.h"
#include "EventFilter/StorageManager/interface/Exception.h"
#include "EventFilter/StorageManager/interface/I2OChain.h"

#include "FWCore/Utilities/interface/Algorithms.h"
#include "boost/bind.hpp"

namespace stor
{

  // Local helper functions
  namespace
  {

    void throw_unknown_queueid(QueueID id)
    {
      std::ostringstream msg;
      msg << "Unable to retrieve queue with signature: ";
      msg << id;
      XCEPT_RAISE(exception::UnknownQueueId, msg.str());
    }
  } // anonymous namespace


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

  size_t
  EventQueueCollection::size() const
  {
    // We obtain locks not because it is unsafe to read the sizes
    // without locking, but because we want consistent values.
    read_lock_t lock_discard_old(_protect_discard_new_queues);
    read_lock_t lock_discard_new(_protect_discard_old_queues);
    return _discard_new_queues.size() + _discard_old_queues.size();
  }

  void 
  EventQueueCollection::addEvent(I2OChain const& event)
  {
    read_lock_t lock_discard_old(_protect_discard_new_queues);
    read_lock_t lock_discard_new(_protect_discard_old_queues);
    std::vector<QueueID> routes = event.getEventConsumerTags();
    edm::for_all(routes,
                 boost::bind(&EventQueueCollection::_enqueue_event, 
                             this, _1, event));
  }

  I2OChain 
  EventQueueCollection::popEvent(QueueID id)
  {
    I2OChain result;
    switch (id.policy()) 
      {
      case enquing_policy::DiscardNew:
        {
          read_lock_t lock(_protect_discard_new_queues);
          _discard_new_queues[id.index()]->deq_nowait(result);
          break;
        }
      case enquing_policy::DiscardOld:
        {
          read_lock_t lock(_protect_discard_old_queues);
          _discard_old_queues[id.index()]->deq_nowait(result);
          break;
        }
      default:
        {
          throw_unknown_queueid(id);
        }
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

  void
  EventQueueCollection::_enqueue_event(QueueID const& id, I2OChain const& event)
  {
    switch (id.policy())
      {
      case enquing_policy::DiscardNew:
        {
          _discard_new_queues[id.index()]->enq_nowait(event);
        }
      case enquing_policy::DiscardOld:
        {
          _discard_old_queues[id.index()]->enq_nowait(event);
        }
      default:
        {
          throw_unknown_queueid(id);
        }
      }
  }

}

/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
