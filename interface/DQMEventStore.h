// $Id: DQMEventStore.h,v 1.8.8.1 2011/01/24 12:18:39 mommsen Exp $
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


namespace stor {
  
  class DQMEventMonitorCollection;
  class I2OChain;


  /**
   * Stores and collates DQM events
   * Note that this code is not thread safe as it uses a
   * class wide temporary buffer to convert I2OChains
   * into DQMEventMsgViews.
   *
   * $Author: mommsen $
   * $Revision: 1.8.8.1 $
   * $Date: 2011/01/24 12:18:39 $
   */
  
  class DQMEventStore
  {
  public:
    
    explicit DQMEventStore(SharedResourcesPtr);

    ~DQMEventStore();

    /**
     * Set the DQMProcessingParams to be used.
     * This clears everything in the store.
     */
    void setParameters(DQMProcessingParams const&);

    /**
     * Adds the DQM event found in the I2OChain to
     * the store. If a matching DQMEventRecord is found,
     * the histograms are added unless collateDQM is false.
     */
    void addDQMEvent(I2OChain const&);

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

    void addDQMEventToStore(I2OChain const&);

    void addDQMEventToReadyToServe(I2OChain const&);

    void addReadyTopLevelFoldersToReadyToServe();

    DQMTopLevelFolderPtr makeDQMTopLevelFolder(I2OChain const&);

    DQMEventMsgView getDQMEventView(I2OChain const&);

    bool getNextReadyTopLevelFolder(DQMTopLevelFolderPtr&);

    void addTopLevelFolderToReadyToServe(const DQMTopLevelFolderPtr);


    DQMProcessingParams _dqmParams;
    DQMEventMonitorCollection& _dqmEventMonColl;
    InitMsgCollectionPtr _initMsgColl;

    typedef std::map<DQMKey, DQMTopLevelFolderPtr> DQMTopLevelFolderMap;
    DQMTopLevelFolderMap _store;
    std::queue<DQMTopLevelFolder::Record> _topLevelFoldersReadyToServe;
    
  };
  
} // namespace stor

#endif // EventFilter_StorageManager_DQMEventStore_h 


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
