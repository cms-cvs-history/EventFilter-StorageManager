// $Id: EventQueueCollection.h,v 1.1.2.4 2009/03/10 15:32:59 mommsen Exp $

#ifndef StorageManager_EventQueueCollection_h
#define StorageManager_EventQueueCollection_h

#include <vector>
#include "boost/thread/mutex.hpp"
#include "boost/shared_ptr.hpp"

#include "EventFilter/StorageManager/interface/ConcurrentQueue.h"
#include "EventFilter/StorageManager/interface/I2OChain.h"
#include "EventFilter/StorageManager/interface/QueueID.h"

namespace stor {

  /**
   * A collection of ConcurrentQueue<I2OChain>.
   *
   * $Author: mommsen $
   * $Revision: 1.1.2.4 $
   * $Date: 2009/03/10 15:32:59 $
   */
  
  class EventQueueCollection
  {
  public:

    /**
       A default-constructed EventQueueCollection contains no queues.
     */
    EventQueueCollection();

    /**
       Create a new contained queue, with the given policy and given
      maximum size. It returns a unique identifier to later identify
      requests originating from this consumer.
    */
    QueueID createQueue(enquing_policy::PolicyTag policy,
			size_t max);

    
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
       Test to see if the queue with the given QueueID is empty.
    */
    bool empty(QueueID id) const;

    /**
       Test to see if the queue with the given QueueID is full.
     */
    bool full(QueueID id) const;

    /**
      Remove queues which haven't been requested by a consumer since a
      given time.
     */
    void expireStaleQueues();

  private:
    typedef ConcurrentQueue<I2OChain, RejectNewest<I2OChain> > discard_new_queue_t;
    typedef ConcurrentQueue<I2OChain, KeepNewest<I2OChain> >   discard_old_queue_t;
    
    // We use shared_ptr not because we want sharing between two
    // different EventQueueCollections (they are not copyable), but
    // rather because if a vector is resized due to a push_back,
    // shared_ptr provides the correct semantics without necessitating
    // a deep copy of any contained queue.
    
    typedef boost::shared_ptr<discard_new_queue_t> discard_new_queue_ptr;
    typedef boost::shared_ptr<discard_old_queue_t> discard_old_queue_ptr;

    // These typedefs need to be changed when we move to Boost 1.38
    typedef boost::mutex::scoped_lock read_lock_t;
    typedef boost::mutex::scoped_lock write_lock_t;
    typedef boost::mutex  read_write_mutex;

    mutable read_write_mutex  _protect_discard_new_queues;
    mutable read_write_mutex  _protect_discard_old_queues;
    
    std::vector<discard_new_queue_ptr> _discard_new_queues;
    std::vector<discard_old_queue_ptr> _discard_old_queues;

    /*
      These functions are declared private and not implemented to
      prevent their use.
    */
    EventQueueCollection(EventQueueCollection const&);
    EventQueueCollection& operator=(EventQueueCollection const&);
  };
  
} // namespace stor

#endif // StorageManager_EventQueueCollection_h 

// emacs configuration
// Local Variables: -
// mode: c++ -
// c-basic-offset: 2 -
// indent-tabs-mode: nil -
// End: -
