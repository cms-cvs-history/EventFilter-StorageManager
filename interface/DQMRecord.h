// $Id: DQMRecord.h,v 1.1.2.18 2009/04/16 12:54:13 mommsen Exp $

#ifndef StorageManager_DQMRecord_h
#define StorageManager_DQMRecord_h

#include <vector>

#include "boost/shared_ptr.hpp"

#include "EventFilter/StorageManager/interface/QueueID.h"

#include "IOPool/Streamer/interface/DQMEventMessage.h"


namespace stor {

  /**
   * Container for shared resources.
   *
   * $Author: mommsen $
   * $Revision: 1.1.2.18 $
   * $Date: 2009/04/16 12:54:13 $
   */

  struct DQMRecord
  {
    std::vector<QueueID> routes;
    boost::shared_ptr<DQMEventMsgView> dqmEventView;

    std::vector<QueueID> getEventConsumerTags() const
    { return routes; }
  };

} // namespace stor

#endif // StorageManager_DQMRecord_h 


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
