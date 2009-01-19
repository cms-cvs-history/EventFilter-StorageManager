// $Id$

#ifndef StorageManager_DQMEventProcessor_h
#define StorageManager_DQMEventProcessor_h

#include "EventFilter/StorageManager/interface/DQMConsumerQueue.h"
#include "EventFilter/StorageManager/interface/DQMEventQueue.h"
#include "EventFilter/StorageManager/interface/QueueCollection.h"


namespace stor {
  
  class DQMEventProcessor
  {
  public:
    
    DQMEventProcessor();
    
    ~DQMEventProcessor();
    
    void processNextDQMEvent();

    
  private:

    QueueCollection<DQMConsumerQueue> _dqmConsumerQueueCollection;
    DQMEventQueue _daqEventQueue;

    
  };
  
} // namespace stor

#endif // StorageManager_DQMEventProcessor_h 


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
