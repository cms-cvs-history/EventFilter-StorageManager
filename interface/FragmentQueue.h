// $Id: FragmentQueue.h,v 1.1.2.2 2009/01/30 10:49:40 mommsen Exp $

#ifndef EventFilter_StorageManager_FragmentQueue_h
#define EventFilter_StorageManager_FragmentQueue_h

#include "EventFilter/StorageManager/interface/ConcurrentQueue.h"
#include "EventFilter/StorageManager/interface/I2OChain.h"

namespace stor {

  /**
   * Queue holding I2OChains of event fragments 
   *
   * $Author: mommsen $
   * $Revision: 1.1.2.2 $
   * $Date: 2009/01/30 10:49:40 $
   */

  typedef ConcurrentQueue<I2OChain> FragmentQueue;  
  
} // namespace stor

#endif // StorageManager_FragmentQueue_h 


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
