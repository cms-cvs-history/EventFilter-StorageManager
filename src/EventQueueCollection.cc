// $Id: EventQueueCollection.cc,v 1.1.2.7 2009/03/25 21:18:44 paterno Exp $

#include "EventFilter/StorageManager/interface/EnquingPolicyTag.h"
#include "EventFilter/StorageManager/interface/EventQueueCollection.h"
#include "EventFilter/StorageManager/interface/Exception.h"
#include "EventFilter/StorageManager/interface/I2OChain.h"

#include "FWCore/Utilities/interface/Algorithms.h"
#include "boost/bind.hpp"

namespace stor
{

  namespace
  {
    const utils::duration_t DEFAULT_STALENESS_INTERVAL = 120.0;

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

  void
  EventQueueCollection::setExpirationInterval(QueueID id,
                                              utils::duration_t interval)
  {
    switch (id.policy()) 
      {
      case enquing_policy::DiscardNew:
        {
          read_lock_t lock(_protect_discard_new_queues);
          if (id.index() < _discard_new_queues.size())
            _discard_new_queues[id.index()]->set_staleness_interval(interval);
          break;
        }
      case enquing_policy::DiscardOld:
        {
          read_lock_t lock(_protect_discard_old_queues);
          if (id.index() < _discard_old_queues.size())
            _discard_old_queues[id.index()]->set_staleness_interval(interval);
          break;
        }
      default:
        {
          throw_unknown_queueid(id);
          // does not return, no break needed
        }
      }
  }

  utils::duration_t
  EventQueueCollection::getExpirationInterval(QueueID id) const
  {
    utils::duration_t result(0.0);
    switch (id.policy()) 
      {
      case enquing_policy::DiscardNew:
        {
          read_lock_t lock(_protect_discard_new_queues);
          if (id.index() < _discard_new_queues.size())
            result = _discard_new_queues[id.index()]->staleness_interval();
          break;
        }
      case enquing_policy::DiscardOld:
        {
          read_lock_t lock(_protect_discard_old_queues);
          if (id.index() < _discard_old_queues.size())
            result = _discard_old_queues[id.index()]->staleness_interval();
          break;
        }
      default:
        {
          throw_unknown_queueid(id);
          // does not return, no break needed
        }
      }
    return result;
  }

  QueueID 
  EventQueueCollection::createQueue(enquing_policy::PolicyTag policy,
				    size_t max,
                                    utils::duration_t interval,
                                    utils::time_point_t now)
  {
    QueueID result;
    if (policy == enquing_policy::DiscardNew)
      {
 	write_lock_t lock(_protect_discard_new_queues);
        expirable_discard_new_queue_ptr newborn(new expirable_discard_new_queue_t(max,
                                                                                  interval,
                                                                                  now));
        _discard_new_queues.push_back(newborn);
        result = QueueID(enquing_policy::DiscardNew,
 			 _discard_new_queues.size()-1);
      }
    else if (policy == enquing_policy::DiscardOld)
      {
	write_lock_t lock(_protect_discard_old_queues);
        expirable_discard_old_queue_ptr newborn(new expirable_discard_old_queue_t(max,
                                                                                  interval,
                                                                                  now));
	_discard_old_queues.push_back(newborn);
	result = QueueID(enquing_policy::DiscardOld,
			 _discard_old_queues.size()-1);

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
          if (id.index() < _discard_new_queues.size())
            _discard_new_queues[id.index()]->deq_nowait(result);
          break;
        }
      case enquing_policy::DiscardOld:
        {
          read_lock_t lock(_protect_discard_old_queues);
          if (id.index() < _discard_old_queues.size())
            _discard_old_queues[id.index()]->deq_nowait(result);
          break;
        }
      default:
        {
          throw_unknown_queueid(id);
          // does not return, no break needed
        }
      }
    return result;
  }

  void
  EventQueueCollection::clearQueue(QueueID id)
  {
    switch (id.policy()) 
      {
      case enquing_policy::DiscardNew:
        {
          read_lock_t lock(_protect_discard_new_queues);
          if (id.index() < _discard_new_queues.size())
            _discard_new_queues[id.index()]->clear();
          break;
        }
      case enquing_policy::DiscardOld:
        {
          read_lock_t lock(_protect_discard_old_queues);
          if (id.index() < _discard_old_queues.size())
            _discard_old_queues[id.index()]->clear();
          break;
        }
      default:
        {
          throw_unknown_queueid(id);
          // does not return, no break needed
        }
      }
  }

  void
  EventQueueCollection::clearQueues()
  {
    read_lock_t lock_discard_new(_protect_discard_new_queues);
    read_lock_t lock_discard_old(_protect_discard_old_queues);
    _discard_new_queues.clear();
    _discard_old_queues.clear();
  }

  bool
  EventQueueCollection::empty(QueueID id) const
  {
    bool result(true);
    switch (id.policy()) 
      {
      case enquing_policy::DiscardNew:
        {
          read_lock_t lock(_protect_discard_new_queues);
          if (id.index() < _discard_new_queues.size())
            result = _discard_new_queues[id.index()]->empty();
          break;
        }
      case enquing_policy::DiscardOld:
        {
          read_lock_t lock(_protect_discard_old_queues);
          if (id.index() < _discard_old_queues.size())
            result = _discard_old_queues[id.index()]->empty();
          break;
        }
      default:
        {
          throw_unknown_queueid(id);
          // does not return, no break needed
        }
      }
    return result;
  }

  bool
  EventQueueCollection::full(QueueID id) const
  {
    bool result(true);
    switch (id.policy()) 
      {
      case enquing_policy::DiscardNew:
        {
          read_lock_t lock(_protect_discard_new_queues);
          if (id.index() < _discard_new_queues.size())
            result = _discard_new_queues[id.index()]->full();
          break;
        }
      case enquing_policy::DiscardOld:
        {
          read_lock_t lock(_protect_discard_old_queues);
          if (id.index() < _discard_old_queues.size())
            result = _discard_old_queues[id.index()]->full();
          break;
        }
      default:
        {
          throw_unknown_queueid(id);
          // does not return, no break needed
        }
      }
    return result;
  }


  void 
  EventQueueCollection::clearStaleQueues(std::vector<QueueID>& result)
  {
    result.clear();
    utils::time_point_t now = utils::getCurrentTime();
    read_lock_t lock_discard_old(_protect_discard_new_queues);
    read_lock_t lock_discard_new(_protect_discard_old_queues);
    
    size_t num_queues = _discard_new_queues.size();
    for (size_t i = 0; i < num_queues; ++i)
      {
        if ( _discard_new_queues[i]->clearIfStale(now))
          result.push_back(QueueID(enquing_policy::DiscardNew, i));
      }

    num_queues = _discard_old_queues.size();
    for (size_t i = 0; i < num_queues; ++i)
      {
        if ( _discard_old_queues[i]->clearIfStale(now))
          result.push_back(QueueID(enquing_policy::DiscardOld, i));
      }
  }

  void
  EventQueueCollection::_enqueue_event(QueueID const& id, 
                                       I2OChain const& event)
  {
    switch (id.policy())
      {
      case enquing_policy::DiscardNew:
        {
          if (id.index() < _discard_new_queues.size())
            _discard_new_queues[id.index()]->enq_nowait(event);
          break;
        }
      case enquing_policy::DiscardOld:
        {
          if (id.index() < _discard_old_queues.size())
            _discard_old_queues[id.index()]->enq_nowait(event);
          break;
        }
      default:
        {
          throw_unknown_queueid(id);
          // does not return, no break needed
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
