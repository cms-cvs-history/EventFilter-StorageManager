// $Id: FragmentQueue.h,v 1.3 2009/07/20 13:06:10 mommsen Exp $
/// @file: FragmentQueue.h 

#ifndef EventFilter_StorageManager_FragmentQueue_h
#define EventFilter_StorageManager_FragmentQueue_h

#include "boost/shared_ptr.hpp"
#include "EventFilter/StorageManager/interface/ConcurrentQueue.h"
#include "EventFilter/StorageManager/interface/I2OChain.h"

namespace stor {

  /**
   * Queue holding I2OChains of event fragments 
   *
   * $Author: mommsen $
   * $Revision: 1.3 $
   * $Date: 2009/07/20 13:06:10 $
   */

  typedef ConcurrentQueue<I2OChain> FragmentQueue;
  typedef boost::shared_ptr<FragmentQueue> FragmentQueuePtr;
  
} // namespace stor

#endif // EventFilter_StorageManager_FragmentQueue_h 


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
