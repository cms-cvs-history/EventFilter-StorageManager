// $Id: EventConsumerQueue.h,v 1.1.2.4 2009/03/02 17:39:31 paterno Exp $

#ifndef StorageManager_EventConsumerQueue_h
#define StorageManager_EventConsumerQueue_h

#include "EventFilter/StorageManager/interface/ConcurrentQueue.h"
#include "EventFilter/StorageManager/interface/I2OChain.h"

namespace stor {

  /**
   * Queue holding I2OChains of complete events waiting to be served
   * over HTTP to an event consumer
   *
   * $Author: paterno $
   * $Revision: 1.1.2.4 $
   * $Date: 2009/03/02 17:39:31 $
   */

  typedef ConcurrentQueue<I2OChain> EventConsumerQueue;
  
  
} // namespace stor

#endif // StorageManager_EventConsumerQueue_h 


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
