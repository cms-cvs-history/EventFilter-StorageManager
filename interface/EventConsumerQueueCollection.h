// $Id: EventConsumerQueueCollection.h,v 1.1.2.1 2009/01/30 10:49:40 mommsen Exp $

#ifndef StorageManager_EventConsumerQueueCollection_h
#define StorageManager_EventConsumerQueueCollection_h

#include <vector>

#include "boost/shared_ptr.hpp"
#include "boost/thread/mutex.hpp"

#include "EventFilter/StorageManager/interface/EventConsumerQueue.h"
#include "EventFilter/StorageManager/interface/EventConsumerRegistrationInfo.h"
#include "EventFilter/StorageManager/interface/I2OChain.h"
#include "EventFilter/StorageManager/interface/Types.h"


namespace stor {

  /**
   * A collection of Event consumer queues
   *
   * $Author: mommsen $
   * $Revision: 1.1.2.1 $
   * $Date: 2009/01/30 10:49:40 $
   */
  
  class EventConsumerQueueCollection
  {
  public:

    /**
       The compiler-generated default constructor, copy constructor,
       copy assignment, and destructor all do the right thing.
     */    


    /**
      Registers a new Event consumer connected by HTTP.  It takes the
      registration information to create a queue type as requested by
      the consumer.  It returns a unique identifier to later identify
      requests originating from this consumer.
    */
    QueueID
    registerConsumer(EventConsumerRegistrationInfo const& ri);
    
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
      Remove queues which haven't been requested by a consumer since a
      given time.
     */
    void expireStaleQueues();

  private:

    typedef ConcurrentQueue<I2OChain, RejectNewest<I2OChain> > reject_new_q_t;
    typedef ConcurrentQueue<I2OChain, KeepNewest<I2OChain> >   keep_new_q_t;


    // These typedefs need to be changed when we move to Boost 1.38
    typedef boost::mutex::scoped_lock read_lock_t;
    typedef boost::mutex::scoped_lock write_lock_t;
    typedef boost::mutex  read_write_mutex;

    mutable read_write_mutex  _protect_reject_new_queues;
    mutable read_write_mutex  _protect_keep_new_queues;
    
    std::vector<reject_new_q_t> _reject_new_queues;
    std::vector<keep_new_q_t>   _keep_new_queues;
  };
  
} // namespace stor

#endif // StorageManager_EventConsumerQueueCollection_h 


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
