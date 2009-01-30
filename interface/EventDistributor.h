// $Id: EventDistributor.h,v 1.1.2.2 2009/01/20 10:54:04 mommsen Exp $

#ifndef StorageManager_EventDistributor_h
#define StorageManager_EventDistributor_h

#include "EventFilter/StorageManager/interface/EventConsumerQueue.h"
#include "EventFilter/StorageManager/interface/EventConsumerQueueCollection.h"
#include "EventFilter/StorageManager/interface/EventConsumerRegistrationInfo.h"
#include "EventFilter/StorageManager/interface/DQMEventQueue.h"
#include "EventFilter/StorageManager/interface/I2OChain.h"
#include "EventFilter/StorageManager/interface/StreamQueue.h"
#include "EventFilter/StorageManager/interface/Types.h"


namespace stor {

  /**
   * Distributes complete events to appropriate queues
   *
   * It receives complete events in form of I2OChains and
   * distributes it to the appropriate queues by checking
   * the I2O message type and the trigger bits in the event
   * header.
   *
   * $Author:$
   * $Revision:$
   * $Date:$
   */
  
  class EventDistributor
  {
  public:
    
    EventDistributor();
    
    ~EventDistributor();
    
    /**
     * Add the event given as I2OChain to the appropriate queues
     */
    void addEventToRelevantQueues(I2OChain&);

    /**
     * Returns false if no further events can be processed,
     * e.g. the StreamQueue is full
     */
    bool full();

    /**
     * Registers a new consumer
     */
    const QueueID registerEventConsumer
    (
      boost::shared_ptr<EventConsumerRegistrationInfo>
    );

    /**
     * Registers new event streams ???
     */
    void registerEventStreams();


  private:
    

    /**
     * Create a new event selector ???
     */
    void makeEventSelector
    (
      const QueueID, 
      boost::shared_ptr<EventConsumerRegistrationInfo>
    );

    EventConsumerQueueCollection _eventConsumerQueueCollection;
    DQMEventQueue _dqmEventQueue;
    StreamQueue _streamQueue;


  };
  
} // namespace stor

#endif // StorageManager_EventDistributor_h 


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
