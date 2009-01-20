// $Id: QueueCollection.h,v 1.1.2.1 2009/01/19 18:12:17 mommsen Exp $

/**
 * @file
 * Templated class to hold a map of queues
 *
 * It handles multiple queues storing complete events as requested 
 * by consumers. Each consumer can register and specify what events shall 
 * be stored in its queue. The consumer can come back later requesting
 * the events using a unique ID handed out at registration time.
 */

#ifndef StorageManager_QueueCollection_h
#define StorageManager_QueueCollection_h

#include <map>

#include "EventFilter/StorageManager/interface/I2OChain.h"

namespace stor {
  
  template<class T>
  class QueueCollection
  {
  public:
    
    QueueCollection();
    
    ~QueueCollection();

    /**
     * Registers a new consumer connected by HTTP.
     * It returns a unique identifier to later identify requests
     * originating from this consumer.
     */
    int registerNewConsumer();
    
    /**
     * Add an event to all queues matching the specifications.
     */
    void addEvent(I2OChain&);

    /**
     * Remove and return an event from the queue for the consumer with id
     */
    I2OChain popEvent(int id);

    /**
     * Remove queues which haven't been requested by a consumer since a given time.
     */
    void disposeOfStaleStuff();

    /**
     * Type of the map
     */
    typedef std::map<int, T> queueMap;

    
  private:

    static queueMap _collection;

  };
  

  // Implementation
  
  template<class T> QueueCollection<T>::QueueCollection()
  {
    
  }
  
  
  template<class T> QueueCollection<T>::~QueueCollection()
  {
    
  }
  
  
  template<class T> int QueueCollection<T>::registerNewConsumer()
  {
    return -1;
  }
  
  
  template<class T> void QueueCollection<T>::addEvent(I2OChain &chain)
  {
    
  }
  
  
  template<class T> I2OChain QueueCollection<T>::popEvent(int index)
  {
    return _collection[index].popEvent();
  }
  
  
  template<class T> void QueueCollection<T>::disposeOfStaleStuff()
  {
    
  }

} // namespace stor


#endif // StorageManager_QueueCollection_h 


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
