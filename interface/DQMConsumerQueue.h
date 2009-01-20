// $Id: DQMConsumerQueue.h,v 1.1.2.1 2009/01/19 18:12:16 mommsen Exp $

/**
 * @file
 * Queue holding I2OChains of complete DQM events (histograms) 
 * waiting to be served over HTTP to a DQM event consumer
 *
 */

#ifndef StorageManager_DQMConsumerQueue_h
#define StorageManager_DQMConsumerQueue_h

#include "EventFilter/StorageManager/interface/Queue.h"


namespace stor {
  
  class DQMConsumerQueue : public Queue
  {
  public:
    
    DQMConsumerQueue();
    
    ~DQMConsumerQueue();
    
    
  private:
    
  };
  
} // namespace stor

#endif // StorageManager_DQMConsumerQueue_h 


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
