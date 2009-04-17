// $Id: DQMEventQueueCollection.h,v 1.1.2.10 2009/04/08 22:32:34 paterno Exp $

#ifndef StorageManager_DQMEventQueueCollection_h
#define StorageManager_DQMEventQueueCollection_h

#include "EventFilter/StorageManager/interface/DQMRecord.h"
#include "EventFilter/StorageManager/interface/QueueCollection.h"

namespace stor {

  /**
   * A collection of ConcurrentQueue<DQMRecordPtr>.
   *
   * $Author: paterno $
   * $Revision: 1.1.2.10 $
   * $Date: 2009/04/08 22:32:34 $
   */

  typedef QueueCollection<DQMRecord> DQMEventQueueCollection;
  
} // namespace stor

#endif // StorageManager_DQMEventQueueCollection_h 



// emacs configuration
// Local Variables: -
// mode: c++ -
// c-basic-offset: 2 -
// indent-tabs-mode: nil -
// End: -
