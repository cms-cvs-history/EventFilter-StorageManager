// $Id: QueueCollection.h,v 1.1.2.1 2009/01/19 18:12:17 mommsen Exp $

#ifndef StorageManager_QueueCollection_h
#define StorageManager_QueueCollection_h

#include <map>

#include "EventFilter/StorageManager/interface/Chain.h"

namespace stor {
  
  template<class T>
  class QueueCollection
  {
  public:
    
    QueueCollection();
    
    ~QueueCollection();

    int registerNewConsumer();
    
    void addEvent(Chain&);

    Chain popEvent(int);

    void disposeOfStaleStuff();

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
  
  
  template<class T> void QueueCollection<T>::addEvent(Chain &chain)
  {
    
  }
  
  
  template<class T> Chain QueueCollection<T>::popEvent(int index)
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
