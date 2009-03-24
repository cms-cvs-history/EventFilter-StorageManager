// $Id: ConcurrentQueue.h,v 1.1.2.9 2009/03/13 21:16:32 paterno Exp $


#ifndef EventFilter_StorageManager_ExpirableEventQueue_h
#define EventFilter_StorageManager_ExpirableEventQueue_h

#include <cstddef> // for size_t

#include "EventFilter/StorageManager/interface/ConcurrentQueue.h"
#include "EventFilter/StorageManager/interface/EnquingPolicyTag.h"
#include "EventFilter/StorageManager/interface/I2OChain.h"
#include "EventFilter/StorageManager/interface/QueueID.h"
#include "EventFilter/StorageManager/interface/Utils.h"

namespace stor
{
  /**
     Class ExpirableEventQueue encapsulates an EventQueue (held
     through a shared_ptr, for inexpensive copying) and timing
     information. It keeps track of when the most recent called to deq
     was made.
   
     $Author: paterno $
     $Revision: 1.1.2.9 $
     $Date: 2009/03/13 21:16:32 $
   */

  template <class Policy>
  class ExpirableEventQueue
  {
  public:
    typedef Policy policy_type; // publish template parameter
    /**
       Create an ExpirableEventQueue with the given maximum size and
       given "time to stale", specified in seconds.
     */
    explicit ExpirableEventQueue(size_t maxsize=std::numeric_limits<size_t>::max(),
                                 utils::duration_t seconds_to_stale = 120.0,
                                 utils::time_point_t now = utils::getCurrentTime());
    /**
      Try to remove an event from the queue, without blocking.
      If an event is available, return 'true' and set the output
      argument 'event'.
      If no event is available, return 'false'.
      In either case, update the staleness time to reflect this
      attempt to get an event.
    */
    bool deq_nowait(I2OChain& event);

    /**
       Put an event onto the queue, respecting the Policy of this
       queue that controls what is done in the case of a full queue.
       This does not affect the staleness time of this queue.
     */
    typename Policy::return_type enq_nowait(I2OChain const& event);

    /**
       Set the staleness interval.
     */
    void set_staleness_interval(utils::duration_t seconds_to_stale);

    /**
       Get the staleness interval.
    */
    utils::duration_t staleness_interval() const;    

    /**
       Clear the queue.
     */
    void clear();

    /**
       Return true if the queue is empty, and false otherwise.
     */
    bool empty() const;

    /**
       Return true if the queue is full, and false otherwise.
    */
    bool full() const;

    /**
       Return true if the queue is stale, and false if it is not. The
       queue is stale if its staleness_time is before the given
       time. If the queue is stale, we also clear it.
    */
    bool clearIfStale(utils::time_point_t now = utils::getCurrentTime());

  private:
    typedef ConcurrentQueue<I2OChain, Policy> queue_t;

    queue_t      _events;
    /**  Time in seconds it takes for this queue to become stale. */
    utils::duration_t   _staleness_interval;
    /** Point in time at which this queue will become stale. */
    utils::time_point_t _staleness_time;

    /*
      The following are not implemented, to prevent copying and
      assignment.
     */
    ExpirableEventQueue(ExpirableEventQueue&);
    ExpirableEventQueue& operator=(ExpirableEventQueue&);
  };

  
  template <class Policy>
  ExpirableEventQueue<Policy>::ExpirableEventQueue(size_t maxsize,
                                                   utils::duration_t seconds_to_stale,
                                                   utils::time_point_t now) :
    _events(maxsize),
    _staleness_interval(seconds_to_stale),
    _staleness_time(now+_staleness_interval)
  {
  }

  template <class Policy>
  bool
  ExpirableEventQueue<Policy>::deq_nowait(I2OChain& event)
  {
    _staleness_time = utils::getCurrentTime() + _staleness_interval;
    return _events.deq_nowait(event);
  }

  template <class Policy>
  typename Policy::return_type
  ExpirableEventQueue<Policy>::enq_nowait(I2OChain const& event)
  {
    return _events.enq_nowait(event);
  }  

  template <class Policy>
  inline
  void
  ExpirableEventQueue<Policy>::set_staleness_interval(utils::duration_t t)
  {
    _staleness_interval = t;
  }

  template <class Policy>
  inline
  utils::duration_t
  ExpirableEventQueue<Policy>::staleness_interval() const
  {
    return _staleness_interval;
  }

  template <class Policy>
  inline
  void
  ExpirableEventQueue<Policy>::clear()
  {
    _events.clear();
  }  

  template <class Policy>
  inline
  bool
  ExpirableEventQueue<Policy>::empty() const
  {
    return _events.empty();
  }


  template <class Policy>
  inline
  bool
  ExpirableEventQueue<Policy>::full() const
  {
    return _events.full();
  }

  template <class Policy>
  inline
  bool
  ExpirableEventQueue<Policy>::clearIfStale(utils::time_point_t now)
  {
    return (_staleness_time < now)
      ? _events.clear(), true
      : false;
  }

  /**
     These typedefs are the only instantiations of ExpirableEventQueue
     actually needed by the StorageManager.
   */

  typedef ExpirableEventQueue<RejectNewest<I2OChain> > ExpirableEventQueueDiscardNew;
  typedef ExpirableEventQueue<KeepNewest<I2OChain> >   ExpirableEventQueueDiscardOld;

} // namespace stor
  

#endif

/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -

