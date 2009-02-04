#ifndef EventFilter_StorageManager_CommandQueue_h
#define EventFilter_StorageManager_CommandQueue_h

#include <algorithm>
#include <cstddef>
#include <list>
#include "boost/statechart/event_base.hpp"
#include "boost/shared_ptr.hpp"

// This is a NON-THREAD-SAFE mock-up of the interface of the class
// CommandQueue.
//
// Usage for stor::CommandQueue is given below.

namespace stor
{
  // To put something ONTO the queue, use:
  //      void push_front(T const&)

  // To pop something off the queue, use:
  //       T pop_back();
  // DO NOT CALL pop_back() is the queue is empty!

  // To see if the queue is empty, use
  //      bool empty() const;

  // To ask the size of the queue (don't do this if you only need to
  // check for emptiness! This is slow!) use
  //       size_t size() const;
  //  but later this will change to ...
  //        CommandQueue::size_type size() const;

  // To clear the queue, use
  //       void clear();

  // Right now, you can iterate through the contents using
  //
  //    CommandQueue q;
  //    std::for_each(q.begin(), q.end(), some_functor);
  // (I have already included <algorithm>).
  //
  //  Later this will change to:
  //     CommandQueue q;
  //     q.for_each(some_functor);
  //

  class CommandQueue
  {
  public:
    typedef boost::shared_ptr<boost::statechart::event_base> event_ptr;
    typedef std::list<event_ptr> sequence_t;
    typedef sequence_t::size_type size_type;
    
    // Compiler-generated default c'tor, copy c'tor, and d'tor are
    // correct (for now).

    void push_front(event_ptr p) 
    {
      _elements.push_back(p);
    }

    bool pop_back(event_ptr& p)
    {
      if (_elements.empty()) return false;
      p = _elements.back();
      _elements.pop_back();
      return true;
    }

    bool empty() const
    {
      return _elements.empty();
    }

    size_type size() const
    {
      return _elements.size();
    }

    void clear()
    {
      _elements.clear();
    }

  private:
      sequence_t _elements;
  };



}

#endif

/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
