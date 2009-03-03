// $Id: SyncQueue.h,v 1.1.2.1 2009/01/30 20:35:14 paterno Exp $

/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2000-2004, CERN.			                 *
 * All rights reserved.                                                  *
 * Authors: J. Gutleber and L. Orsini					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#ifndef EventFilter_StorageManager_toolbox_SyncQueue_h
#define EventFilter_StorageManager_toolbox_SyncQueue_h


#include <algorithm>
#include <list>
#include <string>
#include <iostream>
#include "toolbox/BSem.h"
#include "toolbox/Condition.h"
#include "toolbox/exception/QueueFull.h"
#include "toolbox/exception/Timeout.h"

//! A thread safe, synchronized queue
/*! This class provides a queue abstraction to be
  used for producer/consumer problems amongst two
  or more threads. Note that, if multiple threads act
  as receivers, an element in the queue is consumed by
  at most one thread.
  There exists currently no blocking write, that means,
  if an upper limit for the queue size has been set and
  the queue is full, an exception will be raised.
*/
   
//namespace toolbox {   

namespace
{
  /**
     class ScopedLock implements the ScopedLock pattern, using the
     BSem semaphore type. This provides exception safety for code
     needing to lock the semaphore.
  */
  class ScopedLock
  {
  public:
    explicit ScopedLock(toolbox::BSem& sem) : semaphore_(sem) { semaphore_.take(); }
    ~ScopedLock() { semaphore_.give(); }
  private:
    toolbox::BSem& semaphore_;
  };
}

namespace stor {   
   
  template <class T>
  class SyncQueue
  {
  public:
	
    //! Creates an unbounded synchronized queue
    SyncQueue () : sema_(toolbox::BSem::FULL)
    {
      maxElements_ = UINT_MAX;
    }
		
    //! Creates a synchronized queue with maxElements
    SyncQueue (unsigned int maxElements) : sema_(toolbox::BSem::FULL)
    {
      maxElements_ = maxElements;
    }
		
    //! Limits the number of maximum elements in the queue
    void setMaxElements (unsigned int maxElements)
    {
      maxElements_ = maxElements;
    }
		
    //! push an element into the queue. Throw is queue is full
    void push (const T element) throw (toolbox::exception::QueueFull)
    {
      sema_.take();
      unsigned int size = queue_.size();
      if ( size < maxElements_ )
	{
	  queue_.push_back (element);
	  // If the queue was empty before
	  // signal a potential waiting thread
	  // that data have arrived.
	  if (size == 0) cond_.signal();
	  sema_.give();
	} 
      else 
	{
	  sema_.give();
	  XCEPT_RAISE( toolbox::exception::QueueFull,"synchronized queue full");
	}
    }
		
    //! pop an element from the queue. Block if queue is empty.
    T pop ()
    {
      for (;;)
	{
	  sema_.take();
	  if (queue_.size() > 0)
	    {
	      T value = queue_.front();
	      queue_.pop_front();
	      sema_.give();
	      return value;
	    } 
	  else 
	    {
	      sema_.give();
	    }	cond_.wait();
	}
    }
		
    //! pop an element from the queue. Block until timeout.
    T pop (time_t sec, suseconds_t usec) throw (toolbox::exception::Timeout)
    {
      for (;;)
	{                          
	  sema_.take();
                               
	  if (queue_.size() > 0)
	    {
	      T value = queue_.front();
	      queue_.pop_front();
	      sema_.give();
	      return value;
	    } 
	  else 
	    {
	      sema_.give();
	      // w.b. : going to wait with timeout :
	      try 
		{
		  cond_.timedwait(sec,usec);
		} 
	      catch (toolbox::exception::Timeout& e) 
		{
		  XCEPT_RETHROW(toolbox::exception::Timeout, "synchronized queue timeout", e);
		}
	    }                                
	}
    }
		
    unsigned int size ()
    {
      unsigned int retVal = 0;
      sema_.take();
      retVal = queue_.size();
      sema_.give();
      return retVal;
    }


    /**
       Member template for_each provides a thread-safe means to
       iterate over the contents of the queue, invoking the given
       function once for each element of the collection. It uses
       std::for_each to perform the iteration.

     */
    template <class FUN>
    FUN for_each(FUN f)
    {
      ScopedLock lock(sema_);
      return std::for_each(queue_.begin(), queue_.end(), f);
    }

    /**
       clear() clears the queue.
    */
    void clear()
    {
      ScopedLock lock(sema_);
      queue_.clear();
    }
		
  protected:
	
    std::list<T> queue_;
    toolbox::BSem 	sema_;
    toolbox::Condition cond_;
    unsigned int maxElements_;
  };

}


#endif
/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
