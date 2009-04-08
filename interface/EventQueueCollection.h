// $Id: EventQueueCollection.h,v 1.1.2.8 2009/03/26 01:53:18 paterno Exp $

#ifndef StorageManager_EventQueueCollection_h
#define StorageManager_EventQueueCollection_h

#include <vector>
#include <limits>
#include "boost/thread/mutex.hpp"
#include "boost/shared_ptr.hpp"

#include "EventFilter/StorageManager/interface/ConcurrentQueue.h"
#include "EventFilter/StorageManager/interface/ConsumerID.h"
#include "EventFilter/StorageManager/interface/ExpirableEventQueue.h"
#include "EventFilter/StorageManager/interface/I2OChain.h"
#include "EventFilter/StorageManager/interface/QueueID.h"
#include "EventFilter/StorageManager/interface/Utils.h"

namespace stor {

  /**
   * A collection of ConcurrentQueue<I2OChain>.
   *
   * $Author: paterno $
   * $Revision: 1.1.2.8 $
   * $Date: 2009/03/26 01:53:18 $
   */
  
  class EventQueueCollection
  {
  public:

    /**
       A default-constructed EventQueueCollection contains no queues
     */
    EventQueueCollection();

    /**
       Set or get the time in seconds that the queue with the given id
       can be unused (by a consumer) before becoming stale.
     */
    void setExpirationInterval(QueueID id, utils::duration_t interval);
    utils::duration_t getExpirationInterval(QueueID id) const;

    /**
       Create a new contained queue, with the given policy and given
      maximum size. It returns a unique identifier to later identify
      requests originating from this consumer.
    */
    QueueID createQueue(ConsumerID cid,
                        enquing_policy::PolicyTag policy,
                        size_t max = std::numeric_limits<size_t>::max(),
                        utils::duration_t interval = 120.0,
                        utils::time_point_t now = utils::getCurrentTime());

    /**
       Remove all contained queues. Note that this has the effect of
       clearing all the queues as well.
    */
    void removeQueues();

    /**
       Return the number of queues in the collection.
      */
    size_t size() const;
    
    /**
       Add an event to all queues matching the specifications.
     */
    void addEvent(I2OChain const& event);

    /**
      Remove and return an event from the queue for the consumer with
      the given id. If there is no event in that queue, an empty
      I2OChain is returned.
     */
    I2OChain popEvent(QueueID id);

    /**
       Clear the queue with the given QueueID.
     */
    void clearQueue(QueueID id);

    /**
       Clear all the contained queues.
    */
    void clearQueues();

    /**
       Test to see if the queue with the given QueueID is empty.
    */
    bool empty(QueueID id) const;

    /**
       Test to see if the queue with the given QueueID is full.
     */
    bool full(QueueID id) const;

    /**
       Clear queues which are 'stale'; a queue is stale if it hasn't
      been requested by a consumer within its 'staleness
      interval. Return the QueueID for each queue that is stale (not
      merely those that have become stale recently, but all that are
      stale) in the output argument 'stale_queues'.
     */
    void clearStaleQueues(std::vector<QueueID>& stale_queues);

  private:
    typedef ExpirableEventQueueDiscardNew expirable_discard_new_queue_t;
    typedef ExpirableEventQueueDiscardOld expirable_discard_old_queue_t;


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

    std::vector<expirable_discard_new_queue_ptr> _discard_new_queues;
    std::vector<expirable_discard_old_queue_ptr> _discard_old_queues;
    std::map<ConsumerID, QueueID>                _queue_id_lookup;
    

    /*
      These functions are declared private and not implemented to
      prevent their use.
    */
    EventQueueCollection(EventQueueCollection const&);
    EventQueueCollection& operator=(EventQueueCollection const&);

    /*
      These are helper functions used in the implementation.
    */
    
    void _enqueue_event(QueueID const& id, I2OChain const& event);
  };
  
} // namespace stor

#endif // StorageManager_EventQueueCollection_h 

// emacs configuration
// Local Variables: -
// mode: c++ -
// c-basic-offset: 2 -
// indent-tabs-mode: nil -
// End: -
