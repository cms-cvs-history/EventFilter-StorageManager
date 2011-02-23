// $Id: DQMTopLevelFolder.h,v 1.11.4.1 2011/01/24 12:18:39 mommsen Exp $
/// @file: DQMTopLevelFolder.h 

#ifndef EventFilter_StorageManager_DQMTopLevelFolder_h
#define EventFilter_StorageManager_DQMTopLevelFolder_h

#include <vector>

#include "boost/shared_ptr.hpp"

#include "EventFilter/StorageManager/interface/Configuration.h"
#include "EventFilter/StorageManager/interface/DQMFolder.h"
#include "EventFilter/StorageManager/interface/DQMKey.h"
#include "EventFilter/StorageManager/interface/I2OChain.h"
#include "EventFilter/StorageManager/interface/QueueID.h"

#include "IOPool/Streamer/interface/DQMEventMessage.h"


namespace stor {

  class DQMEventMonitorCollection;


  /**
   * Class holding information for one DQM event
   *
   * $Author: mommsen $
   * $Revision: 1.11.4.1 $
   * $Date: 2011/01/24 12:18:39 $
   */

  class DQMTopLevelFolder
  {

  public:

    struct Record
    {
      struct Entry
      {
        std::vector<unsigned char> buffer;
        std::vector<QueueID> dqmConsumers;
      };
      
      Record() :
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
       * Returns true if there is no DQM event message view available
       */
      inline bool empty() const
      { return ( _entry->buffer.empty() ); }

      /**
       * Returns the memory usage of the stored event msg view in bytes
       */
      inline size_t memoryUsed() const
      { return _entry->buffer.size() + _entry->dqmConsumers.size()*sizeof(QueueID); }

      /**
       * Returns the size of the stored event msg view in bytes
       */
      inline unsigned long totalDataSize() const
      { return _entry->buffer.size(); }

      // We use here a shared_ptr to avoid copying the whole
      // buffer each time the event record is handed on
      boost::shared_ptr<Entry> _entry;
      
    };
    
    
  public:

    DQMTopLevelFolder
    (
      const I2OChain& dqmEvent,
      const DQMProcessingParams&,
      DQMEventMonitorCollection&,
      const unsigned int expectedUpdates
    );

    ~DQMTopLevelFolder();

    /**
     * Adds the DQMEventMsgView. Collates the histograms with the existing
     * DQMEventMsgView if there is one.
     */
    void addDQMEvent(const I2OChain& dqmEvent);

    /**
     * Returns true if this top level folder is ready to be served.
     * This is either the case if all expected updates have been received
     * or when the last update was more than dqmParams.readyTimeDQM ago.
     */
    bool isReady(const utils::time_point_t& now) const;

    /**
     * Populate the record with the currently available data.
     * Return false if no data is available.
     */
    bool getRecord(Record&);


  private:

    void addEvent(std::auto_ptr<DQMEvent::TObjectTable>);
    size_t populateTable(DQMEvent::TObjectTable&) const;

    const DQMProcessingParams _dqmParams;
    DQMEventMonitorCollection& _dqmEventMonColl;

    std::vector<QueueID> _dqmConsumers;

    typedef boost::shared_ptr<DQMFolder> DQMFolderPtr;
    typedef std::map<std::string, DQMFolderPtr> DQMFoldersMap;
    DQMFoldersMap _dqmFolders;

    const DQMKey _dqmKey;
    const unsigned int _expectedUpdates;
    unsigned int _nUpdates;
    utils::time_point_t _lastUpdate;
    unsigned int _sentEvents;
    std::string _releaseTag;
    
    std::vector<unsigned char> _tempEventArea;
    
  };

  typedef boost::shared_ptr<DQMTopLevelFolder> DQMTopLevelFolderPtr;

} // namespace stor

#endif // EventFilter_StorageManager_DQMTopLevelFolder_h 


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
