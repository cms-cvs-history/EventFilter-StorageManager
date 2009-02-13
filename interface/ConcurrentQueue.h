// $Id: RunMonitorCollection.h,v 1.1.2.1 2009/02/12 15:14:09 mommsen Exp $
#ifndef EventFilter_StorageManager_ConcurrentQueue_h
#define EventFilter_StorageManager_ConcurrentQueue_h

#include <algorithm>
#include <cstddef>
#include <limits>
#include <list>

#include <iostream> // debugging

#include "boost/thread/condition.hpp"
#include "boost/thread/mutex.hpp"
#include "boost/thread/xtime.hpp"

namespace stor
{
  /**
   * Class template for concurrent queues
   *
   * $Author: mommsen $
   * $Revision: 1.1.2.3 $
   * $Date: 2009/01/30 10:49:40 $
   */

  template <class T>
  class ConcurrentQueue
  {
  public:
    typedef T value_type;
    typedef std::list<value_type> sequence_type;
    typedef typename sequence_type::size_type size_type;

    /**
       ConcurrentQueue is always bounded. By default, the bound is
       absurdly large.
    */
    explicit ConcurrentQueue(size_type max = 
                             std::numeric_limits<size_type>::max());

    /**
       Applications should arrange to make sure that the destructor of
       a ConcurrentQueue is not called while some other thread is
       using that queue. There is some protection against doing this,
       but it seems impossible to make sufficient protection.
     */
    ~ConcurrentQueue();

    /**
       Copying a ConcurrentQueue is illegal, as is asigning to a
       ConcurrentQueue.
     */

    /**
       Add a copy of item to the queue.  If successful, return true;
       on failure, false.  This function will fail without waiting if
       the queue is full. This may throw any exception thrown by the
       assignment operator of type value_type, or bad_alloc.
    */
    bool enq_nowait(value_type const& item);
    
    /**
       Add a copy of item to the queue. If the queue is full wait
       until it becomes non-full. This may throw any exception thrown
       by the assignment operator of type value_type, or bad_alloc.
     */
    void enq_wait(value_type const& p);

    /**
       Add a copy of item to the queue. If the queue is full wait
       until it becomes non-full or until wait_sec seconds have
       passed. Return true if the items has been put onto the queue or
       false if the timeout has expired. This may throw any exception
       thrown by the assignment operator of type value_type, or
       bad_alloc.
     */
    bool enq_timed_wait(value_type const& p, unsigned long wait_sec);

    /**
       Assign the value at the head of the queue to item and then
       remove the head of the queue. If successful, return true; on
       failure, return false. This function fill fail without waiting
       if the queue is empty. This function may throw any exception
       thrown by the assignment operator of type value_type.
     */
    bool deq_nowait(value_type& item);

    /**
       Assign the value of the head of the queue to item and then
       remove the head of the queue. If the queue is empty wait until
       is has become non-empty. This may throw any exception thrown by
       the assignment operator of type value_type.
     */
    void deq_wait(value_type& item);

    /**
       Assign the value at the head of the queue to item and then
       remove the head of the queue. If the queue is empty wait until
       is has become non-empty or until wait_sec seconds have
       passed. Return true if an item has been removed from the queue
       or false if the timeout has expired. This may throw any
       exception thrown by the assignment operator of type value_type.
     */
    bool deq_timed_wait(value_type& p, unsigned long wait_sec);

    /**
       Return true if the queue is empty, and false if it is not.
     */
    bool empty() const;

    /**
       Return the size of the queue, that is, the number of items it
       contains.
     */
    size_type size() const;

    /**
       Return the capacity of the queue, that is, the maximum number
       of items it can contain.
     */
    size_type capacity() const;

    /**
       Reset the capacity of the queue. This can only be done if the
       queue is empty. This function returns false if the queue was
       not modified, and true if it was modified.
     */
    bool set_capacity(size_type n);

    /**
       Remove all items from the queue. This changes the size to zero
       but does not change the capacity.
     */
    void clear();

  private:
    typedef boost::mutex::scoped_lock lock_t;

    mutable boost::mutex  _protect_elements;
    mutable boost::condition _queue_not_empty;
    mutable boost::condition _queue_not_full;

    sequence_type _elements;
    size_type _capacity;
    /*
      N.B.: we rely on size_type *not* being some synthesized large
      type, so that reading the value is an atomic action, as is
      incrementing or decrementing the value. We do *not* assume that
      there is any atomic get_and_increment or get_and_decrement
      operation.
    */
    size_type _size;

    /*
      These private member functions assume that whatever locks
      necessary for safe operation have already been obtained.
     */

    /*
      Insert the given item into the list, if it is not already full,
      and increment size. Return true if the item is inserted, and
      false if not.
    */
    bool _insert_if_possible(value_type const& item);

    /*
      Insert the given item into the list, and increment size. It is
      assumed not to be full.
     */
    void _insert(value_type const& item);

    /*
      Remove the object at the head of the queue, if there is one, and
      assign item the value of this object.The assignment may throw an
      exception; even if it does, the head will have been removed from
      the queue, and the size appropriately adjusted. It is assumed
      the queue is nonempty. Return true if the queue was nonempty,
      and false if the queue was empty.
     */
    bool _remove_head_if_possible(value_type& item);

    /*
      Remove the object at the head of the queue, and assign item the
      value of this object. The assignment may throw an exception;
      even if it does, the head will have been removed from the queue,
      and the size appropriately adjusted. It is assumed the queue is
      nonempty.
     */
    void _remove_head(value_type& item);


    /*
      These functions are declared private and not implemented to
      prevent their use.
     */
    ConcurrentQueue(ConcurrentQueue<T> const&);
    ConcurrentQueue& operator=(ConcurrentQueue<T> const&);
  };

  //------------------------------------------------------------------
  // Implementation follows
  //------------------------------------------------------------------

  template <class T>
  ConcurrentQueue<T>::ConcurrentQueue(size_type max) :
    _protect_elements(),
    _elements(),
    _capacity(max),
    _size(0)
  {
  }

  template <class T>
  ConcurrentQueue<T>::~ConcurrentQueue()
  {
    lock_t lock(_protect_elements);
    _elements.clear();
    _size = 0;
  }

  template <class T>
  bool
  ConcurrentQueue<T>::enq_nowait(value_type const& item)
  {
    lock_t lock(_protect_elements);
    return _insert_if_possible(item);
  }

  template <class T>
  void
  ConcurrentQueue<T>::enq_wait(value_type const& item)
  {
    lock_t lock(_protect_elements);
    while ( _size >= _capacity) _queue_not_full.wait(lock);
    _insert(item);
  }

  template <class T>
  bool
  ConcurrentQueue<T>::enq_timed_wait(value_type const& item, 
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
    lock_t lock(_protect_elements);
    return _remove_head_if_possible(item);
  }

  template <class T>
  void
  ConcurrentQueue<T>::deq_wait(value_type& item)
  {
    lock_t lock(_protect_elements);
    while (_size == 0) _queue_not_empty.wait(lock);
    _remove_head(item);
  }

  template <class T>
  bool
  ConcurrentQueue<T>::deq_timed_wait(value_type& item,
                                     unsigned long wait_usec)
  {
    lock_t lock(_protect_elements);
    // THis is not yet implemented
    return false;
  }

  template <class T>
  bool 
  ConcurrentQueue<T>::empty() const
  {
    // No lock is necessary: the read is atomic.
    return _size == 0;
  }


  template <class T>
  typename ConcurrentQueue<T>::size_type 
  ConcurrentQueue<T>::size() const
  {
    // No lock is necessary: the read is atomic.
    return _size;
  }

  template <class T>
  typename ConcurrentQueue<T>::size_type
  ConcurrentQueue<T>::capacity() const
  {
    // No lock is necessary: the read is atomic.
    return _capacity;
  }

  template <class T>
  bool
  ConcurrentQueue<T>::set_capacity(size_type newcapacity)
  {
    lock_t lock(_protect_elements);
    bool is_empty = (_size == 0);
    if (is_empty) _capacity = newcapacity;
    return is_empty;
  }

  template <class T>
  void 
  ConcurrentQueue<T>::clear()
  {
    lock_t lock(_protect_elements);
    _elements.clear();
    _size = 0;
  }

  //-----------------------------------------------------------
  // Private member functions
  //-----------------------------------------------------------

  template <class T>
  bool
  ConcurrentQueue<T>::_insert_if_possible(value_type const& item)
  {
    bool item_accepted = false;
    if (_size < _capacity)
      {
        _insert(item);
        item_accepted = true;
      }
    return item_accepted;
  }

  template <class T>
  void
  ConcurrentQueue<T>::_insert(value_type const& item)
  {
    _elements.push_back(item);
    ++_size;
    _queue_not_empty.notify_one();
  }

  template <class T>
  bool
  ConcurrentQueue<T>::_remove_head_if_possible(value_type& item)
  {
    bool item_obtained = false;
    if (!_size == 0)
      {
        _remove_head(item);
        item_obtained = true;
      }
    return item_obtained;
  }

  template <class T>
  void
  ConcurrentQueue<T>::_remove_head(value_type& item)
  {
    sequence_type holder;
    // Move the item out of _elements in a manner that will not throw.
    holder.splice(holder.begin(), _elements, _elements.begin());
    // Record the change in the length of _elements.
    --_size;

    _queue_not_full.notify_one();
    
    // Assign the item. This might throw.
    item = holder.front();
  }

}

#endif

/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -

