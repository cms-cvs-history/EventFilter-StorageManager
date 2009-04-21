// $Id: DQMEventRecord.h,v 1.1.2.1 2009/04/17 17:28:24 mommsen Exp $

#ifndef StorageManager_DQMEventRecord_h
#define StorageManager_DQMEventRecord_h

#include <vector>

#include "boost/shared_ptr.hpp"

#include "EventFilter/StorageManager/interface/Configuration.h"
#include "EventFilter/StorageManager/interface/DQMInstance.h"
#include "EventFilter/StorageManager/interface/DQMKey.h"
#include "EventFilter/StorageManager/interface/QueueID.h"

#include "IOPool/Streamer/interface/DQMEventMessage.h"


namespace stor {

  /**
   * Class holding information for one DQM event
   *
   * $Author: mommsen $
   * $Revision: 1.1.2.1 $
   * $Date: 2009/04/17 17:28:24 $
   */

  class DQMEventRecord : public DQMInstance
  {

  public:

    struct Entry
    {
      std::vector<QueueID> dqmConsumers;
      boost::shared_ptr<DQMEventMsgView> dqmEventView;

      std::vector<QueueID> getEventConsumerTags() const
      { return dqmConsumers; }
    };

  public:

    DQMEventRecord
    (
      DQMKey const&,
      DQMProcessingParams const&
    );

    /**
     * Set the list of DQM event consumers this
     * DQM event should be served to.
     */
    void setEventConsumerTags(std::vector<QueueID> dqmConsumers)
    { _entry.dqmConsumers = dqmConsumers; }

    /**
     * Adds the DQMEventMsgView. Collates the histograms with the existing
     * DQMEventMsgView if there is one.
     */
    void addDQMEventView(boost::shared_ptr<DQMEventMsgView>);

    /**
     * Returns the entry
     */
    DQMEventRecord::Entry getEntry()
    { return _entry; }


  private:

    Entry _entry;

  };

  typedef boost::shared_ptr<DQMEventRecord> DQMEventRecordPtr;

} // namespace stor

#endif // StorageManager_DQMEventRecord_h 


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
