// $Id: EventConsumerSelector.h,v 1.8.2.1 2011/01/14 18:30:22 mommsen Exp $
/// @file: EventConsumerSelector.h 

#ifndef EventFilter_StorageManager_EventConsumerSelector_h
#define EventFilter_StorageManager_EventConsumerSelector_h

#include <boost/shared_ptr.hpp>

#include "EventFilter/StorageManager/interface/EventConsumerRegistrationInfo.h"
#include "EventFilter/StorageManager/interface/I2OChain.h"
#include "EventFilter/StorageManager/interface/TriggerSelector.h"
#include "IOPool/Streamer/interface/InitMessage.h"

namespace stor {

  /**
   * Defines the common interface for event and DQM consumer
   * registration info objects.
   *
   * $Author: mommsen $
   * $Revision: 1.8.2.1 $
   * $Date: 2011/01/14 18:30:22 $
   */

  class EventConsumerSelector
  {

  public:

    /**
     * Constructs an EventConsumerSelector instance based on the
     * specified registration information.
     */
    EventConsumerSelector( const EventConsumerRegistrationInfo* registrationInfo ):
      _initialized( false ),
      _stale( false ),
      _outputModuleId( 0 ),
      _registrationInfo( *registrationInfo ),
      _acceptedEvents( 0 )
    {}

    /**
     * Destructs the EventConsumerSelector instance.
     */
    ~EventConsumerSelector() {}

    /**
     * Initializes the selector instance from the specified
     * INIT message.  EventConsumerSelector instances need to be
     * initialized before they will accept any events.
     */
    void initialize( const InitMsgView& );

    /**
     * Tests whether the specified event is accepted by this selector -
     * returns true if the event is accepted, false otherwise.
     */
    bool acceptEvent( const I2OChain& );

    /**
     * Returns the ID of the queue corresponding to this selector.
     */
    QueueID const queueId() const { return _registrationInfo.queueId(); }

    /**
     * Tests whether this selector has been initialized.
     */
    bool isInitialized() const { return _initialized; }

    /**
     * Return true if no events were requested anymore
     */
    bool isStale() const { return _stale; }

    /**
     * No longer select any events
     */
    void markAsStale() { _stale = true; }

    /**
     * Activate event selection
     */
    void markAsActive() { _stale = false; }

    /**
     *  Comparison:
     */
    bool operator<(const EventConsumerSelector& other) const;

  private:

    bool _initialized;
    bool _stale;
    unsigned int _outputModuleId;
    const EventConsumerRegistrationInfo _registrationInfo;
    TriggerSelectorPtr _eventSelector;
    unsigned long _acceptedEvents;

  };

} // namespace stor

#endif // EventFilter_StorageManager_EventConsumerSelector_h


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
