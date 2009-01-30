// $Id: Queue.h,v 1.1.2.2 2009/01/20 10:54:04 mommsen Exp $

#ifndef StorageManager_Queue_h
#define StorageManager_Queue_h

#include <queue>
#include <boost/thread/mutex.hpp>

#include "EventFilter/StorageManager/interface/I2OChain.h"


namespace stor {

  /**
   * Generic queue class holding I2OChains
   *
   * $Author:$
   * $Revision:$
   * $Date:$
   */
  
  class Queue
  {
  public:

    Queue();
    
    virtual ~Queue();
    
    /**
     * Add a I2OChain to the queue
     */
    virtual void addEvent(I2OChain&);

    /**
     * Removes the oldest element from the queue and returns it
     */
    virtual I2OChain popEvent();

    /**
     * Returns true if the queue is empty
     */
    virtual bool empty();


  private:
    
    std::queue<I2OChain> _queue;
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
