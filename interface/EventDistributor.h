// $Id: EventDistributor.h,v 1.1.2.14 2009/03/10 12:37:50 dshpakov Exp $

#ifndef StorageManager_EventDistributor_h
#define StorageManager_EventDistributor_h

#include "EventFilter/StorageManager/interface/EventConsumerQueue.h"
#include "EventFilter/StorageManager/interface/EventConsumerQueueCollection.h"
#include "EventFilter/StorageManager/interface/DQMEventConsumerQueueCollection.h"
#include "EventFilter/StorageManager/interface/EventConsumerRegistrationInfo.h"
#include "EventFilter/StorageManager/interface/EventStreamConfigurationInfo.h"
#include "EventFilter/StorageManager/interface/ErrorStreamConfigurationInfo.h"
#include "EventFilter/StorageManager/interface/DQMEventQueue.h"
#include "EventFilter/StorageManager/interface/I2OChain.h"
#include "EventFilter/StorageManager/interface/StreamQueue.h"
#include "EventFilter/StorageManager/interface/Types.h"
#include "EventFilter/StorageManager/interface/EventStreamSelector.h"
#include "EventFilter/StorageManager/interface/DQMEventSelector.h"
#include "EventFilter/StorageManager/interface/ErrorStreamSelector.h"
#include "EventFilter/StorageManager/interface/EventConsumerSelector.h"
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
   * $Author: dshpakov $
   * $Revision: 1.1.2.14 $
   * $Date: 2009/03/10 12:37:50 $
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
     * Registers a new DQM consumer
     */
    typedef
    boost::shared_ptr<DQMEventConsumerRegistrationInfo>
    DQMRegPtr;
    const QueueID registerDQMEventConsumer( DQMRegPtr );

    /**
     * Registers the full set of event streams.
     */
    typedef std::vector<EventStreamConfigurationInfo> EvtStrConfList;
    void registerEventStreams( const EvtStrConfList& );

    /**
     * Registers DQM streams.
     */
    typedef std::vector<DQMEventConsumerRegistrationInfo> DQMRegList;
    void registerDQMStreams( const DQMRegList& );

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

    EventConsumerQueueCollection _eventConsumerQueueCollection;
    DQMEventConsumerQueueCollection _DQMQueueCollection;

    DQMEventQueue _dqmEventQueue;
    StreamQueue _streamQueue;

    boost::shared_ptr<InitMsgCollection> _initMsgCollection;

    typedef std::vector<EventStreamSelector> EvtSelList;
    EvtSelList _eventStreamSelectors;

    typedef std::vector<DQMEventSelector> DQMSelList;
    DQMSelList _DQMSelectors;

    typedef std::vector<ErrorStreamSelector> ErrSelList;
    ErrSelList _errorStreamSelectors;

    typedef std::vector<EventConsumerSelector> ConsSelList;
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
