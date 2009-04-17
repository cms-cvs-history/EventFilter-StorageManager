// $Id: EventQueueCollection.h,v 1.1.2.10 2009/04/08 22:32:34 paterno Exp $

#ifndef StorageManager_EventQueueCollection_h
#define StorageManager_EventQueueCollection_h

#include "EventFilter/StorageManager/interface/I2OChain.h"
#include "EventFilter/StorageManager/interface/QueueCollection.h"

namespace stor {

  /**
   * A collection of ConcurrentQueue<I2OChain>.
   *
   * $Author: paterno $
   * $Revision: 1.1.2.10 $
   * $Date: 2009/04/08 22:32:34 $
   */

  typedef QueueCollection<I2OChain> EventQueueCollection;
  
} // namespace stor

#endif // StorageManager_EventQueueCollection_h 



// emacs configuration
// Local Variables: -
// mode: c++ -
// c-basic-offset: 2 -
// indent-tabs-mode: nil -
// End: -
