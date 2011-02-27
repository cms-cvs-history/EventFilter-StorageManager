// $Id: DQMEventStore.h,v 1.8.8.2 2011/02/23 09:27:07 mommsen Exp $
/// @file: DQMEventStore.h 

#ifndef EventFilter_StorageManager_DQMEventStore_h
#define EventFilter_StorageManager_DQMEventStore_h

#include <map>
#include <queue>

#include "boost/shared_ptr.hpp"

#include "IOPool/Streamer/interface/HLTInfo.h"

#include "EventFilter/StorageManager/interface/Configuration.h"
#include "EventFilter/StorageManager/interface/DQMTopLevelFolder.h"
#include "EventFilter/StorageManager/interface/DQMKey.h"
#include "EventFilter/StorageManager/interface/InitMsgCollection.h"
#include "EventFilter/StorageManager/interface/SharedResources.h"

namespace smproxy {
  struct DataRetrieverMonitorCollection;
}

namespace stor {
  
  class DQMEventMonitorCollection;


  /**
   * Stores and collates DQM events
   * Note that this code is not thread safe as it uses a
   * class wide temporary buffer to convert I2OChains
   * into DQMEventMsgViews.
   *
   * $Author: mommsen $
   * $Revision: 1.8.8.2 $
   * $Date: 2011/02/23 09:27:07 $
   */

  template<class EventType, class ConnectionType>  
  class DQMEventStore
  {
  public:
    
    DQMEventStore
    (
      DQMEventMonitorCollection&,
      ConnectionType*,
      size_t (ConnectionType::*getExpectedUpdatesCount)() const
    );

    ~DQMEventStore();

    /**
     * Set the DQMProcessingParams to be used.
     * This clears everything in the store.
     */
    void setParameters(DQMProcessingParams const&);

    /**
     * Adds the DQM event found in EventType to the store.
     * If a matching DQMEventRecord is found,
     * the histograms are added unless collateDQM is false.
     */
    void addDQMEvent(EventType const&);

    /**
     * Returns true if there is a complete group
     * ready to be served to consumers. In this case
     * DQMEventRecord::GroupRecord holds this record.
     */
    bool getCompletedTopLevelFolderIfAvailable(DQMTopLevelFolder::Record&);

    /**
     * Queue completed top level folders, then clear the DQM event store
     */
    void purge();

    /**
     * Clears all data hold by the DQM event store
     */
    void clear();

    /**
     * Checks if the DQM event store is empty
     */
    bool empty()
    { return ( _store.empty() && _topLevelFoldersReadyToServe.empty() ); }

    
  private:

    //Prevent copying of the DQMEventStore
    DQMEventStore(DQMEventStore const&);
    DQMEventStore& operator=(DQMEventStore const&);

    void addDQMEventToStore(EventType const&);

    void addDQMEventToReadyToServe(EventType const&);

    void addReadyTopLevelFoldersToReadyToServe();

    DQMTopLevelFolderPtr makeDQMTopLevelFolder(EventType const&);

    DQMEventMsgView getDQMEventView(EventType const&);

    bool getNextReadyTopLevelFolder(DQMTopLevelFolderPtr&);

    void addTopLevelFolderToReadyToServe(const DQMTopLevelFolderPtr);

    DQMProcessingParams _dqmParams;
    DQMEventMonitorCollection& _dqmEventMonColl;
    ConnectionType* _connectionType;
    size_t (ConnectionType::*_getExpectedUpdatesCount)() const;

    typedef std::map<DQMKey, DQMTopLevelFolderPtr> DQMTopLevelFolderMap;
    DQMTopLevelFolderMap _store;
    std::queue<DQMTopLevelFolder::Record> _topLevelFoldersReadyToServe;
        
    std::vector<unsigned char> _tempEventArea;

  };
  
} // namespace stor

#endif // EventFilter_StorageManager_DQMEventStore_h 


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
