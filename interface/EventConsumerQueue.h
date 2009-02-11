// $Id: EventConsumerQueue.h,v 1.1.2.3 2009/01/30 10:49:40 mommsen Exp $

#ifndef StorageManager_EventConsumerQueue_h
#define StorageManager_EventConsumerQueue_h

#include "EventFilter/StorageManager/interface/Queue.h"


namespace stor {

  /**
   * Queue holding I2OChains of complete events waiting to be served
   * over HTTP to an event consumer
   *
   * $Author: mommsen $
   * $Revision: 1.1.2.3 $
   * $Date: 2009/01/30 10:49:40 $
   */
  
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
