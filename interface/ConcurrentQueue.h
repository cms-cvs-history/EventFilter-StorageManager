#ifndef EventFilter_StorageManager_ConcurrentQueue_h
#define EventFilter_StorageManager_ConcurrentQueue_h

#include <algorithm>
#include <cstddef>
#include <list>

#include "boost/thread/mutex.hpp"

namespace stor
{

  template <class T>
  class ConcurrentQueue
  {
  public:
    typedef T value_type;
    typedef std::list<value_type> sequence_type;
    typedef typename sequence_type::size_type size_type;
    
    void push_front(value_type p);
    bool pop_back(value_type& p);
    bool empty() const;
    size_type size() const;
    void clear();

  private:
    typedef boost::mutex::scoped_lock lock_t;
    mutable boost::mutex  _protect_elements;
    sequence_type _elements;
  };

  template <class T>
  void 
  ConcurrentQueue<T>::push_front(value_type item)
  {
    lock_t lock(_protect_elements);
    // Signal the condition here.
    _elements.push_back(item);
  }

  template <class T>
  bool 
  ConcurrentQueue<T>::pop_back(value_type& item)
  {
    lock_t lock(_protect_elements);
    if (_elements.empty()) return false; // wait for condition here
    item = _elements.back();
    _elements.pop_back();
    return true;
  }

  template <class T>
  bool 
  ConcurrentQueue<T>::empty() const
  {
    lock_t lock(_protect_elements);    
    return _elements.empty();
  }


  template <class T>
  typename ConcurrentQueue<T>::size_type 
  ConcurrentQueue<T>::size() const
  {
    lock_t lock(_protect_elements);
    return _elements.size();
  }

  template <class T>
  void 
  ConcurrentQueue<T>::clear()
  {
    lock_t lock(_protect_elements);
    _elements.clear();
  }
}

#endif

/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -

