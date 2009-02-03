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
  typedef boost::shared_ptr<boost::statechart::event_base> event_ptr;
  typedef std::list<event_ptr> CommandQueue;

  // To put something ONTO the queue, use:
  //      void push_front(T const&)

  // To pop something off the queue, use 
  //      T const& pop_back();

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

  // Please do not use any of the rest of the interface of std::list;
  // the real CommandQueue will not support the whole interface of
  // list.

}

#endif
