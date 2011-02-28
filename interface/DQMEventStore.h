// $Id: DQMEventStore.h,v 1.8.8.3 2011/02/27 13:55:52 mommsen Exp $
/// @file: DQMEventStore.h 

#ifndef EventFilter_StorageManager_DQMEventStore_h
#define EventFilter_StorageManager_DQMEventStore_h

#include <map>
#include <queue>

#include "boost/shared_ptr.hpp"
#include "boost/thread/mutex.hpp"

#include "toolbox/lang/Class.h"
#include "toolbox/task/WaitingWorkLoop.h"
#include "xcept/Exception.h"
#include "xdaq/ApplicationDescriptor.h"

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
   * $Revision: 1.8.8.3 $
   * $Date: 2011/02/27 13:55:52 $
   */

  template<class EventType, class ConnectionType, class StateMachineType>  
  class DQMEventStore : public toolbox::lang::Class
  {
  public:
    
    DQMEventStore
    (
      xdaq::ApplicationDescriptor*,
      DQMEventQueueCollectionPtr,
      DQMEventMonitorCollection&,
      ConnectionType*,
      size_t (ConnectionType::*getExpectedUpdatesCount)() const,
      StateMachineType*,
      void (StateMachineType::*moveToFailedState)(xcept::Exception&)
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
     * Process completed top level folders, then clear the DQM event store
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
    { return _store.empty(); }

    
  private:

    //Prevent copying of the DQMEventStore
    DQMEventStore(DQMEventStore const&);
    DQMEventStore& operator=(DQMEventStore const&);

    void addDQMEventToStore(EventType const&);

    void addDQMEventToReadyToServe(EventType const&);

    DQMTopLevelFolderPtr makeDQMTopLevelFolder(EventType const&);

    DQMEventMsgView getDQMEventView(EventType const&);

    bool getNextReadyTopLevelFolder(DQMTopLevelFolderPtr&);

    void startWorkLoop();

    bool processCompletedTopLevelFolders(toolbox::task::WorkLoop*);

    bool handleNextCompletedTopLevelFolder();

    xdaq::ApplicationDescriptor* _appDescriptor;
    DQMProcessingParams _dqmParams;
    DQMEventQueueCollectionPtr _dqmEventQueueCollection;
    DQMEventMonitorCollection& _dqmEventMonColl;
    ConnectionType* _connectionType;
    size_t (ConnectionType::*_getExpectedUpdatesCount)() const;
    StateMachineType* _stateMachineType;
    void (StateMachineType::*_moveToFailedState)(xcept::Exception&);

    typedef std::map<DQMKey, DQMTopLevelFolderPtr> DQMTopLevelFolderMap;
    DQMTopLevelFolderMap _store;
    mutable boost::mutex _storeMutex;

    toolbox::task::WorkLoop* _completedFolderWL;
    toolbox::task::ActionSignature* _completedFolderAction;
    bool _processCompletedTopLevelFolders;

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
