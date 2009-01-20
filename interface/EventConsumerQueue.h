// $Id: EventConsumerQueue.h,v 1.1.2.1 2009/01/19 18:12:17 mommsen Exp $

/**
 * @file
 * Queue holding I2OChains of complete events waiting to be served
 * over HTTP to an event consumer
 *
 */

#ifndef StorageManager_EventConsumerQueue_h
#define StorageManager_EventConsumerQueue_h

#include "EventFilter/StorageManager/interface/Queue.h"


namespace stor {
  
  class EventConsumerQueue : public Queue
  {
  public:
    
    EventConsumerQueue();
    
    ~EventConsumerQueue();
    
    
  private:
    
  };
  
} // namespace stor

#endif // StorageManager_EventConsumerQueue_h 


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
