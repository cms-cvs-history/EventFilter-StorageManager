// $Id: EventDistributor.h,v 1.1.2.10 2009/03/06 22:09:47 biery Exp $

#ifndef StorageManager_EventDistributor_h
#define StorageManager_EventDistributor_h

#include "EventFilter/StorageManager/interface/EventConsumerQueue.h"
#include "EventFilter/StorageManager/interface/EventConsumerQueueCollection.h"
#include "EventFilter/StorageManager/interface/EventConsumerRegistrationInfo.h"
#include "EventFilter/StorageManager/interface/EventStreamConfigurationInfo.h"
#include "EventFilter/StorageManager/interface/ErrorStreamConfigurationInfo.h"
#include "EventFilter/StorageManager/interface/DQMEventQueue.h"
#include "EventFilter/StorageManager/interface/I2OChain.h"
#include "EventFilter/StorageManager/interface/StreamQueue.h"
#include "EventFilter/StorageManager/interface/Types.h"
#include "EventFilter/StorageManager/interface/EventSelector.h"
#include "EventFilter/StorageManager/interface/ErrorEventSelector.h"
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
   * $Revision: 1.1.2.10 $
   * $Date: 2009/03/06 22:09:47 $
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
     * Registers the full set of event streams.
     */
    typedef std::vector<EventStreamConfigurationInfo> EvtStrConfList;
    void registerEventStreams( const EvtStrConfList& );

    /**
     * Registers the full set of error event streams.
     */
    typedef std::vector<ErrorStreamConfigurationInfo> ErrStrConfList;
    void registerErrorStreams( const ErrStrConfList& );

    /**
     * Clears out all existing event and error streams.
     */
    void clearStreams();

    /**
     * Returns the number of streams that have been configured.
     */
    unsigned int configuredStreamCount() const;

    /**
     * Returns the number of streams that have been configured and initialized.
     */
    unsigned int initializedStreamCount() const;

    /**
     * Clears out all existing consumer registrations.
     */
    void clearConsumers();

    /**
     * Returns the number of consumers that have been configured.
     */
    unsigned int configuredConsumerCount() const;

    /**
     * Returns the number of consumers that have been configured and initialized.
     */
    unsigned int initializedConsumerCount() const;

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

    typedef std::vector<EventSelector<EventStreamConfigurationInfo> > EvtSelList;
    EvtSelList _eventStreamSelectors;

    typedef std::vector<ErrorEventSelector> ErrSelList;
    ErrSelList _errorStreamSelectors;

    typedef std::vector<EventSelector<EventConsumerRegistrationInfo> > ConsSelList;
    ConsSelList _eventConsumerSelectors;

  };
  
} // namespace stor

#endif // StorageManager_EventDistributor_h 


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
