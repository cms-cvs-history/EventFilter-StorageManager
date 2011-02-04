// $Id: QueueCollection.h,v 1.10.2.9 2011/02/04 13:57:45 mommsen Exp $
/// @file: QueueCollection.h 

#ifndef EventFilter_StorageManager_QueueCollection_h
#define EventFilter_StorageManager_QueueCollection_h

#include <vector>
#include <limits>

#include "boost/bind.hpp"
#include "boost/thread/mutex.hpp"
#include "boost/shared_ptr.hpp"

#include "FWCore/Utilities/interface/Algorithms.h"

#include "EventFilter/StorageManager/interface/ConcurrentQueue.h"
#include "EventFilter/StorageManager/interface/ConsumerID.h"
#include "EventFilter/StorageManager/interface/EnquingPolicyTag.h"
#include "EventFilter/StorageManager/interface/EventConsumerRegistrationInfo.h"
#include "EventFilter/StorageManager/interface/Exception.h"
#include "EventFilter/StorageManager/interface/ExpirableQueue.h"
#include "EventFilter/StorageManager/interface/QueueID.h"
#include "EventFilter/StorageManager/interface/RegistrationInfoBase.h"
#include "EventFilter/StorageManager/interface/Utils.h"
#include "EventFilter/StorageManager/interface/ConsumerMonitorCollection.h"


namespace stor {

  /**
   * Class template QueueCollection provides a collection of 
   * ConcurrentQueue<T>.
   *
   * The class T must implement a method getEventConsumerTags() const,
   * returning a std::vector<QueueID> which gives the list
   * of QueueIDs of queues the class should be added.
   *
   * $Author: mommsen $
   * $Revision: 1.10.2.9 $
   * $Date: 2011/02/04 13:57:45 $
   */

  template <class T>
  class QueueCollection
  {
  public:
    typedef typename ExpirableQueue<T, RejectNewest<T> >::size_type size_type;
    typedef typename ExpirableQueue<T, RejectNewest<T> >::value_type value_type;

    /**
       A default-constructed QueueCollection contains no queues
     */
    QueueCollection(ConsumerMonitorCollection&);

    /**
       Set or get the time in seconds that the queue with the given id
       can be unused (by a consumer) before becoming stale.
     */
    void setExpirationInterval(const QueueID&, const utils::duration_t&);
    utils::duration_t getExpirationInterval(const QueueID& id) const;

    /**
      Create a new contained queue, with the given policy and given
      maximum size. It returns a unique identifier to later identify
      requests originating from this consumer.
    */
    QueueID createQueue
    (
      const EventConsRegPtr,
      const utils::time_point_t& now = utils::getCurrentTime()
    );
    QueueID createQueue
    (
      const RegPtr,
      const utils::time_point_t& now = utils::getCurrentTime()
    );
    
    /**
       Remove all contained queues. Note that this has the effect of
       clearing all the queues as well.
    */
    void removeQueues();

    /**
       Return the number of queues in the collection.
      */
    size_type size() const;
    
    /**
       Add an event to all queues matching the specifications.
     */
    void addEvent(T const&);

    /**
      Remove and return an event from the queue for the consumer with
      the given id. If there is no event in that queue, an empty
      event is returned.
     */
    value_type popEvent(const QueueID&);

    /**
      Remove and return an event from the queue for the consumer with
      the given ConsumerID. If there is no event in that queue, an
      empty event is returned.
     */
    value_type popEvent(const ConsumerID&);

    /**
       Clear the queue with the given QueueID.
     */
    void clearQueue(const QueueID&);

    /**
       Clear all queues which are stale at the specified point in time.
     */
    bool clearStaleQueues(const utils::time_point_t&);

    /**
       Clear all the contained queues.
    */
    void clearQueues();

    /**
       Test to see if the queue with the given QueueID is empty.
    */
    bool empty(const QueueID&) const;

    /**
       Test to see if the queue with the given QueueID is full.
     */
    bool full(const QueueID&) const;

    /**
       Test to see if the queue with the given QueueID is stale
       at the given time.
     */
    bool stale(const QueueID&, const utils::time_point_t&) const;

    /**
       Returns true if all queues are stale at the given time.
     */
    bool allQueuesStale(const utils::time_point_t&) const;

    /**
       Get number of elements in queue
     */
    size_type size(const QueueID&) const;


  private:
    typedef ExpirableQueue<T, RejectNewest<T> > expirable_discard_new_queue_t;
    typedef ExpirableQueue<T, KeepNewest<T> > expirable_discard_old_queue_t;


    typedef boost::shared_ptr<expirable_discard_new_queue_t> 
            expirable_discard_new_queue_ptr;
    typedef boost::shared_ptr<expirable_discard_old_queue_t> 
            expirable_discard_old_queue_ptr;

    // These typedefs need to be changed when we move to Boost 1.38
    typedef boost::mutex::scoped_lock read_lock_t;
    typedef boost::mutex::scoped_lock write_lock_t;
    typedef boost::mutex  read_write_mutex;

    // It is possible that one mutex would be better than these
    // three. Only profiling the application will tell for sure.
    mutable read_write_mutex  _protect_discard_new_queues;
    mutable read_write_mutex  _protect_discard_old_queues;
    mutable read_write_mutex  _protect_lookup;

    typedef std::vector<expirable_discard_new_queue_ptr> discard_new_queues_t;
    discard_new_queues_t _discard_new_queues;
    typedef std::vector<expirable_discard_old_queue_ptr> discard_old_queues_t;
    discard_old_queues_t _discard_old_queues;

    typedef std::map<ConsumerID, QueueID>        id_lookup_t;
    id_lookup_t                                  _queue_id_lookup;
    typedef std::map<EventConsRegPtr, QueueID,
                     utils::ptr_comp<EventConsumerRegistrationInfo> > reginfo_lookup_t;
    reginfo_lookup_t                             _queue_reginfo_lookup;
    ConsumerMonitorCollection& _consumer_monitor_collection;

    /*
      These functions are declared private and not implemented to
      prevent their use.
    */
    QueueCollection(QueueCollection const&);
    QueueCollection& operator=(QueueCollection const&);

    /*
      These are helper functions used in the implementation.
    */
    
    size_type _enqueue_event(QueueID const&, T const&, utils::time_point_t const&);
    QueueID get_queue(const RegPtr, const utils::time_point_t&);

  };

  //------------------------------------------------------------------
  // Implementation follows
  //------------------------------------------------------------------

  /**
   N.B.: To avoid deadlock, in any member function that must obtain a
   lock on both the discard_new_queues and on the discard_old_queues,
   always do the locking in that order.
  */

  namespace
  {
    void throw_unknown_queueid(const QueueID& id)
    {
      std::ostringstream msg;
      msg << "Unable to retrieve queue with signature: ";
      msg << id;
      XCEPT_RAISE(exception::UnknownQueueId, msg.str());
    }
  } // anonymous namespace
  
  template <class T>
  QueueCollection<T>::QueueCollection(ConsumerMonitorCollection& ccp ) :
  _protect_discard_new_queues(),
  _protect_discard_old_queues(),
  _protect_lookup(),
  _discard_new_queues(),
  _discard_old_queues(),
  _queue_id_lookup(),
  _consumer_monitor_collection( ccp )
  { }
  
  template <class T>
  void
  QueueCollection<T>::setExpirationInterval(
    const QueueID& id,
    const utils::duration_t& interval
  )
  {
    switch (id.policy()) 
    {
      case enquing_policy::DiscardNew:
      {
        read_lock_t lock(_protect_discard_new_queues);
        try
        {
          _discard_new_queues.at(id.index())->set_staleness_interval(interval);
        }
        catch(std::out_of_range)
        {
          throw_unknown_queueid(id);
        }
        break;
      }
      case enquing_policy::DiscardOld:
      {
        read_lock_t lock(_protect_discard_old_queues);
        try
        {
          _discard_old_queues.at(id.index())->set_staleness_interval(interval);
        }
        catch(std::out_of_range)
        {
          throw_unknown_queueid(id);
        }
        break;
      }
      default:
      {
        throw_unknown_queueid(id);
        // does not return, no break needed
      }
    }
  }
  
  template <class T>
  utils::duration_t
  QueueCollection<T>::getExpirationInterval(const QueueID& id) const
  {
    utils::duration_t result = boost::posix_time::seconds(0);
    switch (id.policy()) 
    {
      case enquing_policy::DiscardNew:
      {
        read_lock_t lock(_protect_discard_new_queues);
        try
        {
          result = _discard_new_queues.at(id.index())->staleness_interval();
        }
        catch(std::out_of_range)
        {
          throw_unknown_queueid(id);
        }
        break;
      }
      case enquing_policy::DiscardOld:
      {
        read_lock_t lock(_protect_discard_old_queues);
        try
        {
          result = _discard_old_queues.at(id.index())->staleness_interval();
        }
        catch(std::out_of_range)
        {
          throw_unknown_queueid(id);
        }
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
  
  template <class T>
  QueueID
  QueueCollection<T>::createQueue
  (
    const EventConsRegPtr reginfo,
    const utils::time_point_t& now
  )
  {
    QueueID qid;
    const ConsumerID& cid = reginfo->consumerId();
    
    // We don't proceed if the given ConsumerID is invalid, or if
    // we've already seen that value before.
    if (!cid.isValid()) return qid;
    write_lock_t lock_lookup(_protect_lookup);
    if (_queue_id_lookup.find(cid) != _queue_id_lookup.end()) return qid;
    
    if ( reginfo->uniqueEvents() )
    {
      // another consumer wants to share the
      // queue to get unique events.
      reginfo_lookup_t::const_iterator it =
        _queue_reginfo_lookup.find(reginfo);
      if ( it != _queue_reginfo_lookup.end() )
      {
        qid = it->second;
        _queue_id_lookup[cid] = qid;
        return qid;
      }
    }
    
    qid = get_queue(reginfo, now);
    _queue_id_lookup[cid] = qid;
    _queue_reginfo_lookup[reginfo] = qid;
    return qid;
  }
  
  template <class T>
  QueueID 
  QueueCollection<T>::createQueue
  (
    const RegPtr reginfo,
    const utils::time_point_t& now
  )
  {
    QueueID qid;
    const ConsumerID& cid = reginfo->consumerId();

    // We don't proceed if the given ConsumerID is invalid, or if
    // we've already seen that value before.
    if (!cid.isValid()) return qid;
    write_lock_t lock_lookup(_protect_lookup);
    if (_queue_id_lookup.find(cid) != _queue_id_lookup.end()) return qid;
    qid = get_queue(reginfo, now);
    _queue_id_lookup[cid] = qid;
    return qid;
  }
  
  template <class T>
  QueueID
  QueueCollection<T>::get_queue
  (
    const RegPtr reginfo,
    const utils::time_point_t& now
  )
  {
    if (reginfo->queuePolicy() == enquing_policy::DiscardNew)
    {
      write_lock_t lock(_protect_discard_new_queues);
      expirable_discard_new_queue_ptr newborn(
        new expirable_discard_new_queue_t(
          reginfo->queueSize(),
          reginfo->secondsToStale(),
          now
        )
      );
      _discard_new_queues.push_back(newborn);
      return QueueID(
        enquing_policy::DiscardNew,
        _discard_new_queues.size()-1
      );
    }
    else if (reginfo->queuePolicy() == enquing_policy::DiscardOld)
    {
      write_lock_t lock(_protect_discard_old_queues);
      expirable_discard_old_queue_ptr newborn(
        new expirable_discard_old_queue_t(
          reginfo->queueSize(),
          reginfo->secondsToStale(),
          now
        )
      );
      _discard_old_queues.push_back(newborn);
      return QueueID(
        enquing_policy::DiscardOld,
        _discard_old_queues.size()-1
      );
    }
    return QueueID();
  }
  
  template <class T>
  void
  QueueCollection<T>::removeQueues()
  {
    clearQueues();
    
    write_lock_t lock_discard_new(_protect_discard_new_queues);
    write_lock_t lock_discard_old(_protect_discard_old_queues);
    _discard_new_queues.clear();
    _discard_old_queues.clear();    

    write_lock_t lock_lookup(_protect_lookup);
    _queue_id_lookup.clear();
  }
  
  template <class T>
  typename QueueCollection<T>::size_type
  QueueCollection<T>::size() const
  {
    // We obtain locks not because it is unsafe to read the sizes
    // without locking, but because we want consistent values.
    read_lock_t lock_discard_new(_protect_discard_new_queues);
    read_lock_t lock_discard_old(_protect_discard_old_queues);
    return _discard_new_queues.size() + _discard_old_queues.size();
  }
  
  template <class T>
  void 
  QueueCollection<T>::addEvent(T const& event)
  {
    read_lock_t lock_discard_new(_protect_discard_new_queues);
    read_lock_t lock_discard_old(_protect_discard_old_queues);
    
    utils::time_point_t now = utils::getCurrentTime();
    std::vector<QueueID> routes = event.getEventConsumerTags();
    
    for( std::vector<QueueID>::const_iterator it = routes.begin(), itEnd = routes.end();
         it != itEnd; ++it )
    {
      const size_type droppedEvents = _enqueue_event( *it, event, now );
      _consumer_monitor_collection.addQueuedEventSample( *it, event.totalDataSize() );
      _consumer_monitor_collection.addDroppedEvents( *it, droppedEvents );
    }
  }
  
  template <class T>
  typename QueueCollection<T>::value_type
  QueueCollection<T>::popEvent(const QueueID& id)
  {
    value_type result;
    switch (id.policy()) 
    {
      case enquing_policy::DiscardNew:
      {
        read_lock_t lock(_protect_discard_new_queues);
        try
        {
          if ( _discard_new_queues.at(id.index())->deq_nowait(result) )
            _consumer_monitor_collection.addServedEventSample(id,
              result.first.totalDataSize());
        }
        catch(std::out_of_range)
        {
          throw_unknown_queueid(id);
        }
        break;
      }
      case enquing_policy::DiscardOld:
      {
        read_lock_t lock(_protect_discard_old_queues);
        try
        {
          if ( _discard_old_queues.at(id.index())->deq_nowait(result) )
            _consumer_monitor_collection.addServedEventSample(id,
              result.first.totalDataSize());
        }
        catch(std::out_of_range)
        {
          throw_unknown_queueid(id);
        }
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
  
  template <class T>
  typename QueueCollection<T>::value_type
  QueueCollection<T>::popEvent(const ConsumerID& cid)
  {
    value_type result;
    if (!cid.isValid()) return result;
    QueueID id;
    {
      // Scope to control lifetime of lock.
      read_lock_t lock(_protect_lookup);
      id_lookup_t::const_iterator i = _queue_id_lookup.find(cid);
      if (i == _queue_id_lookup.end()) return result;
      id = i->second;
    }
    return popEvent(id);
  }
  
  template <class T>
  void
  QueueCollection<T>::clearQueue(const QueueID& id)
  {
    switch (id.policy()) 
    {
      case enquing_policy::DiscardNew:
      {
        read_lock_t lock(_protect_discard_new_queues);
        try
        {
          const size_type clearedEvents =
            _discard_new_queues.at(id.index())->clear();
          
          _consumer_monitor_collection.addDroppedEvents(
            id, clearedEvents);
        }
        catch(std::out_of_range)
        {
          throw_unknown_queueid(id);
        }
        break;
      }
      case enquing_policy::DiscardOld:
      {
        read_lock_t lock(_protect_discard_old_queues);
        try
        {
          const size_type clearedEvents = 
            _discard_old_queues.at(id.index())->clear();
          
          _consumer_monitor_collection.addDroppedEvents(
            id, clearedEvents);
        }
        catch(std::out_of_range)
        {
          throw_unknown_queueid(id);
        }
        break;
      }
      default:
      {
        throw_unknown_queueid(id);
        // does not return, no break needed
      }
    }
  }
  
  template <class T>
  bool
  QueueCollection<T>::clearStaleQueues(const utils::time_point_t& now)
  {
    bool result(false);
    size_type clearedEvents;
    
    {
      read_lock_t lock(_protect_discard_new_queues);
      const size_type num_queues = _discard_new_queues.size();
      for (size_type i = 0; i < num_queues; ++i)
      {
        if ( _discard_new_queues[i]->clearIfStale(now, clearedEvents) )
        {
          _consumer_monitor_collection.addDroppedEvents(
            QueueID(enquing_policy::DiscardNew, i),
            clearedEvents
          );
          result = true;
        }
      }
    }
    {
      read_lock_t lock(_protect_discard_old_queues);
      const size_type num_queues = _discard_old_queues.size();
      for (size_type i = 0; i < num_queues; ++i)
      {
        if ( _discard_old_queues[i]->clearIfStale(now, clearedEvents) )
        {
          _consumer_monitor_collection.addDroppedEvents(
            QueueID(enquing_policy::DiscardOld, i),
            clearedEvents
          );
          result = true;
        }
      }
    }
    return result;
  }
  
  template <class T>
  void
  QueueCollection<T>::clearQueues()
  {
    {
      read_lock_t lock(_protect_discard_new_queues);
      const size_type num_queues = _discard_new_queues.size();
      for (size_type i = 0; i < num_queues; ++i)
      {
        const size_type clearedEvents =
          _discard_new_queues[i]->clear();

        _consumer_monitor_collection.addDroppedEvents(
          QueueID(enquing_policy::DiscardNew, i),
          clearedEvents
        );
      }
    }
    {
      read_lock_t lock(_protect_discard_old_queues);
      const size_type num_queues = _discard_old_queues.size();
      for (size_type i = 0; i < num_queues; ++i)
      {
        const size_type clearedEvents =
          _discard_old_queues[i]->clear();

        _consumer_monitor_collection.addDroppedEvents(
          QueueID(enquing_policy::DiscardOld, i),
          clearedEvents
        );
      }
    }
  }
  
  template <class T>
  bool
  QueueCollection<T>::empty(const QueueID& id) const
  {
    bool result(true);
    switch (id.policy()) 
    {
      case enquing_policy::DiscardNew:
      {
        read_lock_t lock(_protect_discard_new_queues);
        try
        {
          result = _discard_new_queues.at(id.index())->empty();
        }
        catch(std::out_of_range)
        {
          throw_unknown_queueid(id);
        }
        break;
      }
      case enquing_policy::DiscardOld:
      {
        read_lock_t lock(_protect_discard_old_queues);
        try
        {
          result = _discard_old_queues.at(id.index())->empty();
        }
        catch(std::out_of_range)
        {
          throw_unknown_queueid(id);
        }
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
  
  template <class T>
  bool
  QueueCollection<T>::full(const QueueID& id) const
  {
    bool result(true);
    switch (id.policy()) 
    {
      case enquing_policy::DiscardNew:
      {
        read_lock_t lock(_protect_discard_new_queues);
        try
        {
          result = _discard_new_queues.at(id.index())->full();
        }
        catch(std::out_of_range)
        {
          throw_unknown_queueid(id);
        }
        break;
      }
      case enquing_policy::DiscardOld:
      {
        read_lock_t lock(_protect_discard_old_queues);
        try
        {
          result = _discard_old_queues.at(id.index())->full();
        }
        catch(std::out_of_range)
        {
          throw_unknown_queueid(id);
        }
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
  
  template <class T>
  bool
  QueueCollection<T>::stale(const QueueID& id, const utils::time_point_t& now) const
  {
    bool result(true);
    switch (id.policy()) 
    {
      case enquing_policy::DiscardNew:
      {
        read_lock_t lock(_protect_discard_new_queues);
        try
        {
          result = _discard_new_queues.at(id.index())->stale(now);
        }
        catch(std::out_of_range)
        {
          throw_unknown_queueid(id);
        }
        break;
      }
      case enquing_policy::DiscardOld:
      {
        read_lock_t lock(_protect_discard_old_queues);
        try
        {
          result = _discard_old_queues.at(id.index())->stale(now);
        }
        catch(std::out_of_range)
        {
          throw_unknown_queueid(id);
        }
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
  
  template <class T>
  bool
  QueueCollection<T>::allQueuesStale(const utils::time_point_t& now) const
  {
    {
      read_lock_t lock(_protect_discard_new_queues);
      const size_type num_queues = _discard_new_queues.size();
      for (size_type i = 0; i < num_queues; ++i)
      {
        if ( ! _discard_new_queues[i]->stale(now) ) return false;
      }
    }
    {
      read_lock_t lock(_protect_discard_old_queues);
      const size_type num_queues = _discard_old_queues.size();
      for (size_type i = 0; i < num_queues; ++i)
      {
        if ( ! _discard_old_queues[i]->stale(now) ) return false;
      }
    }
    return true;
  }
  
  template <class T>
  typename QueueCollection<T>::size_type
  QueueCollection<T>::size(const QueueID& id) const
  {
    size_type result = 0;
    switch (id.policy()) 
    {
      case enquing_policy::DiscardNew:
      {
        read_lock_t lock(_protect_discard_new_queues);
        try
        {
          result = _discard_new_queues.at(id.index())->size();
        }
        catch(std::out_of_range)
        {
          throw_unknown_queueid(id);
        }
        break;
      }
      case enquing_policy::DiscardOld:
      {
        read_lock_t lock(_protect_discard_old_queues);
        try
        {
          result = _discard_old_queues.at(id.index())->size();
        }
        catch(std::out_of_range)
        {
          throw_unknown_queueid(id);
        }
        break;
      }
      default:
      {
        throw_unknown_queueid( id );
        // does not return, no break needed
      }
    }
    return result;
  }
  
  template <class T>
  typename QueueCollection<T>::size_type
  QueueCollection<T>::_enqueue_event
  (
    QueueID const& id, 
    T const& event,
    utils::time_point_t const& now
  )
  {
    switch (id.policy())
    {
      case enquing_policy::DiscardNew:
      {
        try
        {
          return _discard_new_queues.at(id.index())->enq_nowait(event,now);
        }
        catch(std::out_of_range)
        {
          throw_unknown_queueid(id);
        }
        break;
      }
      case enquing_policy::DiscardOld:
      {
        try
        {
          return _discard_old_queues.at(id.index())->enq_nowait(event,now);
        }
        catch(std::out_of_range)
        {
          throw_unknown_queueid(id);
        }
        break;
      }
      default:
      {
        throw_unknown_queueid(id);
        // does not return, no break needed
      }
    }
    return 1; // event could not be entered
  }
  
} // namespace stor

#endif // EventFilter_StorageManager_QueueCollection_h 

/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
