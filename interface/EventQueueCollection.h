// $Id: EventQueueCollection.h,v 1.3.14.2 2011/01/24 12:18:39 mommsen Exp $
/// @file: EventQueueCollection.h 

#ifndef EventFilter_StorageManager_EventQueueCollection_h
#define EventFilter_StorageManager_EventQueueCollection_h

#include "boost/shared_ptr.hpp"
#include "EventFilter/StorageManager/interface/I2OChain.h"
#include "EventFilter/StorageManager/interface/QueueCollection.h"

namespace stor {

  /**
   * A collection of ConcurrentQueue<I2OChain>.
   *
   * $Author: mommsen $
   * $Revision: 1.3.14.2 $
   * $Date: 2011/01/24 12:18:39 $
   */

  typedef QueueCollection<I2OChain> EventQueueCollection;
  typedef boost::shared_ptr<EventQueueCollection> EventQueueCollectionPtr;
  
} // namespace stor

#endif // EventFilter_StorageManager_EventQueueCollection_h 



// emacs configuration
// Local Variables: -
// mode: c++ -
// c-basic-offset: 2 -
// indent-tabs-mode: nil -
// End: -
