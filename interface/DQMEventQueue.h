// $Id: DQMEventQueue.h,v 1.1.2.1 2009/01/19 18:12:17 mommsen Exp $

/**
 * @file
 * Queue holding I2OChains of complete DQM events (histograms)
 * waiting to be processed by the DQMEventProcessor
 */

#ifndef StorageManager_DQMEventQueue_h
#define StorageManager_DQMEventQueue_h

#include "EventFilter/StorageManager/interface/Queue.h"


namespace stor {
  
  class DQMEventQueue : public Queue
  {
  public:
    
    DQMEventQueue();
    
    ~DQMEventQueue();
    
    
  private:
    
  };
  
} // namespace stor

#endif // StorageManager_DQMEventQueue_h 


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
