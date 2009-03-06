// $Id: EventDistributor.h,v 1.1.2.7 2009/03/05 22:31:35 biery Exp $

#ifndef StorageManager_EventDistributor_h
#define StorageManager_EventDistributor_h

#include "EventFilter/StorageManager/interface/EventConsumerQueue.h"
#include "EventFilter/StorageManager/interface/EventConsumerQueueCollection.h"
#include "EventFilter/StorageManager/interface/EventConsumerRegistrationInfo.h"
#include "EventFilter/StorageManager/interface/EventStreamConfigurationInfo.h"
#include "EventFilter/StorageManager/interface/DQMEventQueue.h"
#include "EventFilter/StorageManager/interface/I2OChain.h"
#include "EventFilter/StorageManager/interface/StreamQueue.h"
#include "EventFilter/StorageManager/interface/Types.h"
#include "EventFilter/StorageManager/interface/EventSelector.h"
#include "EventFilter/StorageManager/interface/InitMsgCollection.h"

#include "boost/shared_ptr.hpp"


namespace stor {

  /**
   * Distributes complete events to appropriate queues
   *
   * It receives complete events in form of I2OChains and
   * distributes it to the appropriate queues by checking
   * the I2O message type and the trigger bits in the event
   * header.
   *
   * $Author: biery $
   * $Revision: 1.1.2.7 $
   * $Date: 2009/03/05 22:31:35 $
   */

  class EventDistributor
  {
  public:

    /**
     * @deprecated
     */
    EventDistributor() {}

    EventDistributor(boost::shared_ptr<InitMsgCollection>);

    ~EventDistributor();

    /**
     * Add the event given as I2OChain to the appropriate queues
     */
    void addEventToRelevantQueues(I2OChain&);

    /**
     * Returns false if no further events can be processed,
     * e.g. the StreamQueue is full
     */
    const bool full() const;

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
    typedef std::vector<EventStreamConfigurationInfo> StreamConfList;
    void registerEventStreams( const StreamConfList& );

    /**
     * Clears out all existing event streams.
     */
    void clearEventStreams();

    /**
     * Returns the number of streams that have been configured.
     */
    unsigned int configuredStreamCount() const;

    /**
     * Returns the number of streams that have been configured and initialized.
     */
    unsigned int initializedStreamCount() const;

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

    boost::shared_ptr<InitMsgCollection> _initMsgCollection;

    typedef std::vector<EventSelector> ESList;
    ESList _eventSelectors;

  };
  
} // namespace stor

#endif // StorageManager_EventDistributor_h 


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
