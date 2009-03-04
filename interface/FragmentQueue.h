// $Id: FragmentQueue.h,v 1.1.2.3 2009/02/19 22:29:03 paterno Exp $

#ifndef EventFilter_StorageManager_FragmentQueue_h
#define EventFilter_StorageManager_FragmentQueue_h

#include "EventFilter/StorageManager/interface/ConcurrentQueue.h"
#include "EventFilter/StorageManager/interface/I2OChain.h"

namespace stor {

  /**
   * Queue holding I2OChains of event fragments 
   *
   * $Author: paterno $
   * $Revision: 1.1.2.3 $
   * $Date: 2009/02/19 22:29:03 $
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
