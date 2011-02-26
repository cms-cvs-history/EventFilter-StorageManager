// $Id: EventConsumerSelector.h,v 1.8.2.3 2011/01/24 14:03:20 mommsen Exp $
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
   * $Revision: 1.8.2.3 $
   * $Date: 2011/01/24 14:03:20 $
   */

  class EventConsumerSelector
  {

  public:

    /**
     * Constructs an EventConsumerSelector instance based on the
     * specified registration information.
     */
    EventConsumerSelector( const EventConsRegPtr registrationInfo ):
      _initialized( false ),
      _outputModuleId( 0 ),
      _registrationInfo( registrationInfo ),
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
    QueueID const queueId() const { return _registrationInfo->queueId(); }

    /**
     * Tests whether this selector has been initialized.
     */
    bool isInitialized() const { return _initialized; }

    /**
     *  Comparison:
     */
    bool operator<(const EventConsumerSelector& other) const;

  private:

    bool _initialized;
    unsigned int _outputModuleId;
    const EventConsRegPtr _registrationInfo;
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
