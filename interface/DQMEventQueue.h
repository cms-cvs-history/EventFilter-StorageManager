// $Id: DQMEventQueue.h,v 1.1.2.2 2009/01/20 10:54:04 mommsen Exp $

#ifndef StorageManager_DQMEventQueue_h
#define StorageManager_DQMEventQueue_h

#include "EventFilter/StorageManager/interface/Queue.h"


namespace stor {

  /**
   * Queue holding I2OChains of complete DQM events (histograms)
   * waiting to be processed by the DQMEventProcessor
   *
   * $Author:$
   * $Revision:$
   * $Date:$
   */
  
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
