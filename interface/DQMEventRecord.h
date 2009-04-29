// $Id: DQMEventRecord.h,v 1.1.2.5 2009/04/27 16:55:41 mommsen Exp $

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
   * $Revision: 1.1.2.5 $
   * $Date: 2009/04/27 16:55:41 $
   */

  class DQMEventRecord : public DQMInstance
  {

  public:

    struct GroupRecord
    {
      struct Entry
      {
        std::vector<unsigned char> buffer;
        std::vector<QueueID> dqmConsumers;
      };
      
      GroupRecord() :
      _entry(new Entry) {};
      
      /**
       * Get the list of DQM event consumers this
       * DQM event group should be served to.
       */
      std::vector<QueueID> getEventConsumerTags() const
       { return _entry->dqmConsumers; }

      /**
       * Returns the DQM event message view for this group
       */
      DQMEventMsgView getDQMEventMsgView()
      { return DQMEventMsgView(&_entry->buffer[0]); }

      /**
       * Returns true if there is a DQM event message view available
       */
      bool isValid() const
      { return ( !_entry->buffer.empty() ); }

      /**
       * Returns the size of the stored event msg view in bytes
       */
      size_t size() const
      { return _entry->buffer.size(); }

      // We use here a shared_ptr to avoid copying the whole
      // buffer each time the event record is handed on
      boost::shared_ptr<Entry> _entry;
      
    };
    
    
  public:

    DQMEventRecord
    (
      const DQMKey,
      const DQMProcessingParams
    );

    /**
     * Set the list of DQM event consumers this
     * DQM event should be served to.
     */
    void setEventConsumerTags(std::vector<QueueID> dqmConsumers)
    { _dqmConsumers = dqmConsumers; }

    /**
     * Adds the DQMEventMsgView. Collates the histograms with the existing
     * DQMEventMsgView if there is one.
     */
    void addDQMEventView(DQMEventMsgView const&);

    /**
     * Populates the dqmEventView with the requested group and returns the group
     */
    GroupRecord populateAndGetGroup(const std::string groupName);


  private:

    const DQMProcessingParams _dqmParams;

    std::vector<QueueID> _dqmConsumers;
    std::string _releaseTag;

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
