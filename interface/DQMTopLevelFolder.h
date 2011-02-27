// $Id: DQMTopLevelFolder.h,v 1.1.2.2 2011/02/24 13:37:40 mommsen Exp $
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
   * $Revision: 1.1.2.2 $
   * $Date: 2011/02/24 13:37:40 $
   */

  class DQMTopLevelFolder
  {

  public:

    class Record
    {
    private:

      struct Entry
      {
        std::vector<unsigned char> buffer;
        QueueIDs dqmConsumers;
      };

    public:
      
      Record() :
      _entry(new Entry) {};

      /**
       * Clear any data
       */
      inline void clear()
      { _entry->buffer.clear(); _entry->dqmConsumers.clear(); }

      /**
       * Return a reference to the buffer providing space for
       * the specified size in bytes.
       */
      inline void* getBuffer(size_t size) const
      { _entry->buffer.resize(size); return &(_entry->buffer[0]); }

      /**
       * Set the list of DQM event consumer this
       * top level folder should be served to.
       */
      inline void tagForEventConsumers(const QueueIDs& ids)
      { _entry->dqmConsumers = ids; }
      
      /**
       * Get the list of DQM event consumers this
       * top level folder should be served to.
       */
      inline QueueIDs getEventConsumerTags() const
       { return _entry->dqmConsumers; }

      /**
       * Returns the DQM event message view for this group
       */
      inline DQMEventMsgView getDQMEventMsgView()
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


    private:

      // We use here a shared_ptr to avoid copying the whole
      // buffer each time the event record is handed on
      boost::shared_ptr<Entry> _entry;
      
    };
    
    
  public:

    DQMTopLevelFolder
    (
      const DQMKey&,
      const QueueIDs&,
      const DQMProcessingParams&,
      DQMEventMonitorCollection&,
      const unsigned int expectedUpdates
    );

    ~DQMTopLevelFolder();

    /**
     * Adds the DQMEventMsgView, but does not take ownership of the underlying
     * data buffer. Collates the histograms with the existing
     * DQMEventMsgView if there is one.
     */
    void addDQMEvent(const DQMEventMsgView&);

    /**
     * Adds the DQM event message contained in the I2OChain.
     * It copies the data from the I2OChain in its own buffer space.
     * Collates the histograms with the existing
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

    const DQMKey _dqmKey;
    const QueueIDs _dqmConsumers;
    const DQMProcessingParams _dqmParams;
    DQMEventMonitorCollection& _dqmEventMonColl;
    const unsigned int _expectedUpdates;

    unsigned int _nUpdates;
    utils::time_point_t _lastUpdate;
    unsigned int _sentEvents;
    std::string _releaseTag;

    typedef boost::shared_ptr<DQMFolder> DQMFolderPtr;
    typedef std::map<std::string, DQMFolderPtr> DQMFoldersMap;
    DQMFoldersMap _dqmFolders;
    
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
