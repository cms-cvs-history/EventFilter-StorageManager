// $Id: DQMConsumerQueue.h,v 1.1.2.1 2009/01/19 18:12:16 mommsen Exp $

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
