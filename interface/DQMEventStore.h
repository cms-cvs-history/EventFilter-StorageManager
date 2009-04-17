// $Id: DQMEventStore.h,v 1.1.2.1 2009/04/17 10:41:59 mommsen Exp $

#ifndef StorageManager_DQMEventStore_h
#define StorageManager_DQMEventStore_h

#include <map>

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
   * $Revision: 1.1.2.1 $
   * $Date: 2009/04/17 10:41:59 $
   */
  
  class DQMEventStore
  {
  public:
    
    explicit DQMEventStore(DQMProcessingParams const&);

    /**
     * Adds the DQM event found in the I2OChain to
     * the store. If a matching DQMEventRecord is found,
     * the histograms are added unless collateDQM is false.
     */
    void addDQMEvent(I2OChain&);


    /**
     * Returns true if there is a complete DQMEventRecord
     * ready to be served to consumers. In this case
     * DQMEventRecord& holds this record.
     */
    bool getCompletedDQMEventRecordIfAvailable(DQMEventRecord&);


    /**
     * Clears all DQMEventRecords hold by the DQM store
     */
    void clear()
    { _store.clear(); }


    /**
     * Checks if the DQM store is empty
     */
    bool empty()
    { return _store.empty(); }

    
  private:

    //Prevent copying of the DQMEventStore
    DQMEventStore(DQMEventStore const&);
    DQMEventStore& operator=(DQMEventStore const&);

    DQMEventRecord makeDQMEventRecord(I2OChain const&);

    boost::shared_ptr<DQMEventMsgView> getDQMEventView(I2OChain const&);


    const DQMProcessingParams& _dqmParams;

    typedef std::map<DQMKey, DQMEventRecord> DQMEventRecordMap;
    DQMEventRecordMap _store;
    
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
