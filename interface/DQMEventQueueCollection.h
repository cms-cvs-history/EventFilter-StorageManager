// $Id: DQMEventQueueCollection.h,v 1.3.14.1 2011/01/21 15:51:20 mommsen Exp $
/// @file: DQMEventQueueCollection.h 

#ifndef EventFilter_StorageManager_DQMEventQueueCollection_h
#define EventFilter_StorageManager_DQMEventQueueCollection_h

#include "boost/shared_ptr.hpp"
#include "EventFilter/StorageManager/interface/DQMEventRecord.h"
#include "EventFilter/StorageManager/interface/QueueCollection.h"

namespace stor {

  /**
   * A collection of ConcurrentQueue<DQMEventRecord>.
   *
   * $Author: mommsen $
   * $Revision: 1.3.14.1 $
   * $Date: 2011/01/21 15:51:20 $
   */

  typedef QueueCollection<DQMEventRecord::GroupRecord> DQMEventQueueCollection;
  typedef boost::shared_ptr<DQMEventQueueCollection> DQMEventQueueCollectionPtr;

} // namespace stor

#endif // EventFilter_StorageManager_DQMEventQueueCollection_h 



// emacs configuration
// Local Variables: -
// mode: c++ -
// c-basic-offset: 2 -
// indent-tabs-mode: nil -
// End: -
