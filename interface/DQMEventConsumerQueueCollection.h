// $Id$

#ifndef StorageManager_DQMEventConsumerQueueCollection_h
#define StorageManager_DQMEventConsumerQueueCollection_h

#include <map>

#include <boost/shared_ptr.hpp>

#include "EventFilter/StorageManager/interface/DQMEventConsumerQueue.h"
#include "EventFilter/StorageManager/interface/DQMEventConsumerRegistrationInfo.h"
#include "EventFilter/StorageManager/interface/I2OChain.h"
#include "EventFilter/StorageManager/interface/Types.h"


namespace stor {

  /**
   * A collection of DQMEvent consumer queues
   *
   * $Author:$
   * $Revision:$
   * $Date:$
   */
  
  class DQMEventConsumerQueueCollection
  {
  public:
    
    DQMEventConsumerQueueCollection();
    
    ~DQMEventConsumerQueueCollection();

    /**
     * Registers a new DQMEvent consumer connected by HTTP.
     * It takes the registration information to create
     * a queue type as requested by the consumer.
     * It returns a unique identifier to later identify
     * requests originating from this consumer.
     */
    const QueueID registerDQMEventConsumer
    (
      boost::shared_ptr<DQMEventConsumerRegistrationInfo>
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
    typedef std::map<const QueueID, boost::shared_ptr<DQMEventConsumerQueue> > queueMap;

    
  private:
    
    queueMap _collection;

  };
  
} // namespace stor

#endif // StorageManager_DQMEventConsumerQueueCollection_h 


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
