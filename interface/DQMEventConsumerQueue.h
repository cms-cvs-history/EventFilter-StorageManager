// $Id: DQMEventConsumerQueue.h,v 1.1.2.2 2009/01/20 10:54:04 mommsen Exp $

#ifndef StorageManager_DQMEventConsumerQueue_h
#define StorageManager_DQMEventConsumerQueue_h

#include "EventFilter/StorageManager/interface/Queue.h"


namespace stor {

  /**
   * Queue holding I2OChains of complete DQM events (histograms) 
   * waiting to be served over HTTP to a DQM event consumer
   *
   * $Author:$
   * $Revision:$
   * $Date:$
   */
  
  class DQMEventConsumerQueue : public Queue
  {
  public:
    
    DQMEventConsumerQueue();
    
    ~DQMEventConsumerQueue();
    
    
  private:
    
  };
  
} // namespace stor

#endif // StorageManager_DQMEventConsumerQueue_h 


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
