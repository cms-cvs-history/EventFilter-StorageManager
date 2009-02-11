// $Id: EventConsumerQueueCollection.h,v 1.1.2.1 2009/01/30 10:49:40 mommsen Exp $

#ifndef StorageManager_EventConsumerQueueCollection_h
#define StorageManager_EventConsumerQueueCollection_h

#include <map>

#include <boost/shared_ptr.hpp>

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
    
    EventConsumerQueueCollection();
    
    ~EventConsumerQueueCollection();

    /**
     * Registers a new Event consumer connected by HTTP.
     * It takes the registration information to create
     * a queue type as requested by the consumer.
     * It returns a unique identifier to later identify
     * requests originating from this consumer.
     */
    const QueueID registerEventConsumer
    (
      boost::shared_ptr<EventConsumerRegistrationInfo>
    );
    
    /**
     * Add an event to all queues matching the specifications.
     */
    void addEvent(I2OChain&);

    /**
     * Remove and return an event from the queue for the consumer with id
     */
    I2OChain popEvent(const QueueID);

    /**
     * Remove queues which haven't been requested by a consumer since a given time.
     */
    void disposeOfStaleStuff();

    /**
     * Type of the map
     */
    typedef std::map<const QueueID, boost::shared_ptr<EventConsumerQueue> > queueMap;

    
  private:
    
    queueMap _collection;

  };
  
} // namespace stor

#endif // StorageManager_EventConsumerQueueCollection_h 


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
