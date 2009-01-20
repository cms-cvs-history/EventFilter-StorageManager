// $Id: EventDistributor.h,v 1.1.2.1 2009/01/19 18:12:17 mommsen Exp $

#ifndef StorageManager_EventDistributor_h
#define StorageManager_EventDistributor_h

#include "EventFilter/StorageManager/interface/EventConsumerQueue.h"
#include "EventFilter/StorageManager/interface/DQMEventQueue.h"
#include "EventFilter/StorageManager/interface/QueueCollection.h"
#include "EventFilter/StorageManager/interface/StreamQueue.h"


namespace stor {
  
  class EventDistributor
  {
  public:
    
    EventDistributor();
    
    ~EventDistributor();
    
    void addEventToRelevantQueues(Chain&);


  private:
    
    QueueCollection<EventConsumerQueue> _eventConsumerQueueCollection;
    DQMEventQueue _dqmEventQueue;
    StreamQueue _streamQueue;


  };
  
} // namespace stor

#endif // StorageManager_EventDistributor_h 


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
