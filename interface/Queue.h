// $Id: Queue.h,v 1.1.2.1 2009/01/19 18:12:17 mommsen Exp $

#ifndef StorageManager_Queue_h
#define StorageManager_Queue_h

#include <queue>
#include <boost/thread/mutex.hpp>

#include "EventFilter/StorageManager/interface/Chain.h"


namespace stor {
  
  class Queue
  {
  public:

    Queue();
    
    virtual ~Queue();
    
    virtual void addEvent(Chain&);

    virtual Chain popEvent();


  private:
    
    static std::queue<Chain> _queue;
    boost::mutex _mutex;

  };
  
} // namespace stor

#endif // StorageManager_Queue_h 


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
