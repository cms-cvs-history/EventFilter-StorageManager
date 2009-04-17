// $Id: DQMEventQueueCollection.h,v 1.1.2.1 2009/04/17 10:41:59 mommsen Exp $

#ifndef StorageManager_DQMEventQueueCollection_h
#define StorageManager_DQMEventQueueCollection_h

#include "EventFilter/StorageManager/interface/DQMEventRecord.h"
#include "EventFilter/StorageManager/interface/QueueCollection.h"

namespace stor {

  /**
   * A collection of ConcurrentQueue<DQMEventRecord>.
   *
   * $Author: mommsen $
   * $Revision: 1.1.2.1 $
   * $Date: 2009/04/17 10:41:59 $
   */

  typedef QueueCollection<DQMEventRecord> DQMEventQueueCollection;
  
} // namespace stor

#endif // StorageManager_DQMEventQueueCollection_h 



// emacs configuration
// Local Variables: -
// mode: c++ -
// c-basic-offset: 2 -
// indent-tabs-mode: nil -
// End: -
