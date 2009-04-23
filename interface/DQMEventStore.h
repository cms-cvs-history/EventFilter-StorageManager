// $Id: DQMEventStore.h,v 1.1.2.4 2009/04/22 15:35:01 mommsen Exp $

#ifndef StorageManager_DQMEventStore_h
#define StorageManager_DQMEventStore_h

#include <map>
#include <stack>

#include "boost/shared_ptr.hpp"

#include "IOPool/Streamer/interface/HLTInfo.h"

#include "EventFilter/StorageManager/interface/Configuration.h"
#include "EventFilter/StorageManager/interface/DQMKey.h"
#include "EventFilter/StorageManager/interface/DQMEventRecord.h"
#include "EventFilter/StorageManager/interface/I2OChain.h"


namespace stor {
  
  /**
   * Stores and collates DQM events
   *
   * $Author: mommsen $
   * $Revision: 1.1.2.4 $
   * $Date: 2009/04/22 15:35:01 $
   */
  
  class DQMEventStore
  {
  public:
    
    explicit DQMEventStore(const DQMProcessingParams);

    /**
     * Adds the DQM event found in the I2OChain to
     * the store. If a matching DQMEventRecord is found,
     * the histograms are added unless collateDQM is false.
     */
    void addDQMEvent(I2OChain&);


    /**
     * Returns true if there is a complete DQMEventRecord
     * ready to be served to consumers. In this case
     * DQMEventRecord::Entry holds this record.
     */
    bool getCompletedDQMEventRecordIfAvailable(DQMEventRecord::Entry&);

    /**
     * Writes and purges all DQMEventRecords hold by the store
     */
    void writeAndPurgeAllDQMInstances();

    /**
     * Clears all DQMEventRecords hold by the DQM store
     */
    void clear()
    { _store.clear(); } // need to clear stack


    /**
     * Checks if the DQM event store is empty
     */
    bool empty()
    { return ( _store.empty() && _recordsReadyToServe.empty() ); }

    
  private:

    //Prevent copying of the DQMEventStore
    DQMEventStore(DQMEventStore const&);
    DQMEventStore& operator=(DQMEventStore const&);

    void addDQMEventToStore(I2OChain const&);

    void addDQMEventToReadyToServe(I2OChain const&);

    void addNextAvailableDQMGroupToReadyToServe(const std::string groupName);

    DQMEventRecordPtr makeDQMEventRecord(I2OChain const&);

    boost::shared_ptr<DQMEventMsgView> getDQMEventView(I2OChain const&);

    DQMEventRecordPtr getNewestReadyDQMEventRecord(const std::string groupName);

    void writeAndPurgeStaleDQMInstances();


    const DQMProcessingParams _dqmParams;

    typedef std::map<DQMKey, DQMEventRecordPtr> DQMEventRecordMap;
    DQMEventRecordMap _store;
    // Always serve the freshest records
    std::stack<DQMEventRecord::Entry> _recordsReadyToServe;
    
   std::vector<unsigned char> _tempEventArea;
    
  };
  
} // namespace stor

#endif // StorageManager_DQMEventStore_h 


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
