#ifndef EventFilter_StorageManager_ConcurrentQueue_h
#define EventFilter_StorageManager_ConcurrentQueue_h

#include <algorithm>
#include <cstddef>
#include <list>

#include <iostream> // debugging

#include "boost/thread/mutex.hpp"
#include "boost/thread/condition.hpp"
#include "boost/thread/xtime.hpp"

namespace stor
{

  template <class T>
  class ConcurrentQueue
  {
  public:
    typedef T value_type;
    typedef std::list<value_type> sequence_type;
    typedef typename sequence_type::size_type size_type;

    explicit ConcurrentQueue(size_type max = 1000*1000);

    bool enq_nowait(value_type p);
    bool enq_wait(value_type p);
    bool enq_timed_wait(value_type p, unsigned long wait_sec);

    bool deq_nowait(value_type& p);

    bool empty() const;
    size_type size() const;
    void set_capacity(size_type n);
    void clear();

  private:
    typedef boost::mutex::scoped_lock lock_t;
    mutable boost::mutex  _protect_elements;
    mutable boost::condition _queue_not_empty;
    mutable boost::condition _queue_not_full;
    sequence_type _elements;
    size_type     _capacity;
    size_type     _size;

    bool _insert_if_possible(T const& item);
  };

  template <class T>
  ConcurrentQueue<T>::ConcurrentQueue(size_type max) :
    _protect_elements(),
    _elements(),
    _capacity(max),
    _size(0)
  {
  }

  template <class T>
  bool
  ConcurrentQueue<T>::enq_nowait(value_type item)
  {
    lock_t lock(_protect_elements);
    return _insert_if_possible(item);
  }

  template <class T>
  bool
  ConcurrentQueue<T>::enq_wait(value_type item)
  {
    lock_t lock(_protect_elements);
    while ( _size >= _capacity) 
      {
        std::cerr << "Queue is full. size = "
                  << _size
                  << " capacity: "
                  << _capacity
                  << " , please wait\n";
        _queue_not_full.wait(lock);
      }
    return _insert_if_possible(item);
  }

  template <class T>
  bool
  ConcurrentQueue<T>::enq_timed_wait(value_type item, 
                                     unsigned long wait_sec)
  {
    lock_t lock(_protect_elements);

    if (! (_size < _capacity) )
      {
        boost::xtime now;
        if (boost::xtime_get(&now, CLOCK_MONOTONIC) != CLOCK_MONOTONIC) 
          return false;
        now.sec += wait_sec;
        _queue_not_full.timed_wait(lock, now);
      }

    return  _insert_if_possible(item);
  }

  template <class T>
  bool
  ConcurrentQueue<T>::deq_nowait(value_type& item)
  {
    {
      lock_t lock(_protect_elements);
      if (_elements.empty()) return false; // wait for condition here
      
      item = _elements.front();
      _elements.pop_front();
    }
    _queue_not_full.notify_all();
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

  // Private member functions

  template <class T>
  bool
  ConcurrentQueue<T>::_insert_if_possible(value_type const& item)
  {
    bool item_accepted = false;
    if (_size < _capacity)
      {
        _elements.push_back(item);
        // Signal nonemtpy here
        item_accepted = true;
        ++_size;
      }
    return item_accepted;
  }

}

#endif

/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -

