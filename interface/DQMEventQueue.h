// $Id: DQMEventQueue.h,v 1.1.2.3 2009/01/30 10:49:40 mommsen Exp $

#ifndef StorageManager_DQMEventQueue_h
#define StorageManager_DQMEventQueue_h

#include "EventFilter/StorageManager/interface/Queue.h"


namespace stor {

  /**
   * Queue holding I2OChains of complete DQM events (histograms)
   * waiting to be processed by the DQMEventProcessor
   *
   * $Author: mommsen $
   * $Revision: 1.1.2.3 $
   * $Date: 2009/01/30 10:49:40 $
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
