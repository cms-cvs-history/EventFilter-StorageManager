// $Id: DQMEventMonitorCollection.h,v 1.7.8.2 2011/02/04 13:57:45 mommsen Exp $
/// @file: DQMEventMonitorCollection.h 

#ifndef EventFilter_StorageManager_DQMEventMonitorCollection_h
#define EventFilter_StorageManager_DQMEventMonitorCollection_h

#include "xdata/Double.h"
#include "xdata/UnsignedInteger32.h"

#include "EventFilter/StorageManager/interface/MonitorCollection.h"


namespace stor {

  /**
   * A collection of MonitoredQuantities related to fragments
   *
   * $Author: mommsen $
   * $Revision: 1.7.8.2 $
   * $Date: 2011/02/04 13:57:45 $
   */
  
  class DQMEventMonitorCollection : public MonitorCollection
  {
  private:

    MonitoredQuantity _droppedDQMEventCounts;

    MonitoredQuantity _dqmEventSizes;
    MonitoredQuantity _servedDQMEventSizes;
    MonitoredQuantity _writtenDQMEventSizes;

    MonitoredQuantity _dqmEventBandwidth;
    MonitoredQuantity _servedDQMEventBandwidth;
    MonitoredQuantity _writtenDQMEventBandwidth;

    MonitoredQuantity _numberOfTopLevelFolders;
    MonitoredQuantity _numberOfUpdates;
    MonitoredQuantity _numberOfWrittenTopLevelFolders;


  public:

    struct DQMEventStats
    {
      MonitoredQuantity::Stats droppedDQMEventCountsStats;  //number of events
      
      MonitoredQuantity::Stats dqmEventSizeStats;             //MB
      MonitoredQuantity::Stats servedDQMEventSizeStats;       //MB
      MonitoredQuantity::Stats writtenDQMEventSizeStats;      //MB
      
      MonitoredQuantity::Stats dqmEventBandwidthStats;        //MB/s
      MonitoredQuantity::Stats servedDQMEventBandwidthStats;  //MB/s
      MonitoredQuantity::Stats writtenDQMEventBandwidthStats; //MB/s

      MonitoredQuantity::Stats numberOfTopLevelFoldersStats;  //number of top level folders
      MonitoredQuantity::Stats numberOfUpdatesStats;          //number of received updates per DQMKey
      MonitoredQuantity::Stats numberOfWrittenTopLevelFoldersStats; //number of top level folders written to disk
    };

    explicit DQMEventMonitorCollection(const utils::duration_t& updateInterval);

    const MonitoredQuantity& getDroppedDQMEventCountsMQ() const {
      return _droppedDQMEventCounts;
    }
    MonitoredQuantity& getDroppedDQMEventCountsMQ() {
      return _droppedDQMEventCounts;
    }

    const MonitoredQuantity& getDQMEventSizeMQ() const {
      return _dqmEventSizes;
    }
    MonitoredQuantity& getDQMEventSizeMQ() {
      return _dqmEventSizes;
    }

    const MonitoredQuantity& getServedDQMEventSizeMQ() const {
      return _servedDQMEventSizes;
    }
    MonitoredQuantity& getServedDQMEventSizeMQ() {
      return _servedDQMEventSizes;
    }

    const MonitoredQuantity& getWrittenDQMEventSizeMQ() const {
      return _writtenDQMEventSizes;
    }
    MonitoredQuantity& getWrittenDQMEventSizeMQ() {
      return _writtenDQMEventSizes;
    }

    const MonitoredQuantity& getDQMEventBandwidthMQ() const {
      return _dqmEventBandwidth;
    }
    MonitoredQuantity& getDQMEventBandwidthMQ() {
      return _dqmEventBandwidth;
    }

    const MonitoredQuantity& getServedDQMEventBandwidthMQ() const {
      return _servedDQMEventBandwidth;
    }
    MonitoredQuantity& getServedDQMEventBandwidthMQ() {
      return _servedDQMEventBandwidth;
    }

    const MonitoredQuantity& getWrittenDQMEventBandwidthMQ() const {
      return _writtenDQMEventBandwidth;
    }
    MonitoredQuantity& getWrittenDQMEventBandwidthMQ() {
      return _writtenDQMEventBandwidth;
    }

    const MonitoredQuantity& getNumberOfTopLevelFoldersMQ() const {
      return _numberOfTopLevelFolders;
    }
    MonitoredQuantity& getNumberOfTopLevelFoldersMQ() {
      return _numberOfTopLevelFolders;
    }

    const MonitoredQuantity& getNumberOfUpdatesMQ() const {
      return _numberOfUpdates;
    }
    MonitoredQuantity& getNumberOfUpdatesMQ() {
      return _numberOfUpdates;
    }

    const MonitoredQuantity& getNumberOfWrittenTopLevelFoldersMQ() const {
      return _numberOfWrittenTopLevelFolders;
    }
    MonitoredQuantity& getNumberOfWrittenTopLevelFoldersMQ() {
      return _numberOfWrittenTopLevelFolders;
    }

   /**
    * Write all our collected statistics into the given Stats struct.
    */
    void getStats(DQMEventStats& stats) const;


  private:

    //Prevent copying of the DQMEventMonitorCollection
    DQMEventMonitorCollection(DQMEventMonitorCollection const&);
    DQMEventMonitorCollection& operator=(DQMEventMonitorCollection const&);

    virtual void do_calculateStatistics();
    virtual void do_reset();
    virtual void do_appendInfoSpaceItems(InfoSpaceItems&);
    virtual void do_updateInfoSpaceItems();

    xdata::Double _dqmFoldersPerEP;
    xdata::UnsignedInteger32 _processedDQMEvents;
    xdata::UnsignedInteger32 _droppedDQMEvents;
  };
  
} // namespace stor

#endif // EventFilter_StorageManager_DQMEventMonitorCollection_h 


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
