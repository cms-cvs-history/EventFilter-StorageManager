// $Id: DQMEventStore.h,v 1.1.2.8 2009/02/20 20:54:52 biery Exp $

#ifndef StorageManager_DQMEventStore_h
#define StorageManager_DQMEventStore_h

#include <map>

#include "IOPool/Streamer/interface/HLTInfo.h"

#include "EventFilter/StorageManager/interface/DQMRecord.h"
#include "EventFilter/StorageManager/interface/I2OChain.h"


namespace stor {
  
  /**
   * Stores and collates DQM events
   *
   * $Author: biery $
   * $Revision: 1.1.2.8 $
   * $Date: 2009/02/20 20:54:52 $
   */
  
  class DQMEventStore
  {
  public:
    
    DQMEventStore() {};

    /**
     * Adds the DQM event found in the I2OChain to
     * the store. If a matching DQMRecord is found,
     * the histograms are added unless collateDQM is false.
     */
    void addDQMEvent(I2OChain const&);


    /**
     * Returns true if there is a complete DQMRecord
     * ready to be served to consumers. In this case
     * DQMRecord& holds this record.
     */
    bool getCompletedDQMRecordIfAvailable(DQMRecord&);


    /**
     * Clears all DQMRecords hold by the DQM store
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

    typedef std::map<int, int> fragmentMap;
    fragmentMap _store;
    
    
  };
  
} // namespace stor

#endif // StorageManager_DQMEventStore_h 


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
