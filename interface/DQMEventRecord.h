// $Id: DQMEventRecord.h,v 1.1.2.1 2009/04/17 10:41:59 mommsen Exp $

#ifndef StorageManager_DQMEventRecord_h
#define StorageManager_DQMEventRecord_h

#include <vector>

#include "boost/shared_ptr.hpp"

#include "EventFilter/StorageManager/interface/QueueID.h"

#include "IOPool/Streamer/interface/DQMEventMessage.h"


namespace stor {

  /**
   * Class holding information for one DQM event
   *
   * $Author: mommsen $
   * $Revision: 1.1.2.1 $
   * $Date: 2009/04/17 10:41:59 $
   */

  class DQMEventRecord
  {

  public:

    /**
     * Set the list of DQM event consumers this
     * DQM event should be served to.
     */
    void setEventConsumerTags(std::vector<QueueID> dqmConsumers)
    { _entry->dqmConsumers = dqmConsumers; }

    /**
     * Returns the list of DQM event consumers this
     * DQM event is tagged.
     */
    std::vector<QueueID> getEventConsumerTags() const
    { return _entry->dqmConsumers; }

    /**
     * Adds the DQMEventMsgView. Collates the histograms with the existing
     * DQMEventMsgView if there is one.
     */
    void addDQMEventView(boost::shared_ptr<DQMEventMsgView>);


  private:
    
    struct DQMEventRecordEntry
    {
      std::vector<QueueID> dqmConsumers;
      boost::shared_ptr<DQMEventMsgView> dqmEventView;
    };

    boost::shared_ptr<DQMEventRecordEntry> _entry;

  };

} // namespace stor

#endif // StorageManager_DQMEventRecord_h 


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
