// $Id: DQMEventMonitorCollection.h,v 1.1.2.16 2009/04/14 12:50:30 mommsen Exp $

#ifndef StorageManager_DQMEventMonitorCollection_h
#define StorageManager_DQMEventMonitorCollection_h

#include "xdata/Double.h"
#include "xdata/UnsignedInteger32.h"

#include "EventFilter/StorageManager/interface/MonitorCollection.h"


namespace stor {

  /**
   * A collection of MonitoredQuantities related to fragments
   *
   * $Author: mommsen $
   * $Revision: 1.1.2.16 $
   * $Date: 2009/04/14 12:50:30 $
   */
  
  class DQMEventMonitorCollection : public MonitorCollection
  {
  private:

    MonitoredQuantity _dqmEventSizes;
    MonitoredQuantity _servedDQMEventSizes;
    MonitoredQuantity _writtenDQMEventSizes;

    MonitoredQuantity _dqmEventBandwidth;
    MonitoredQuantity _servedDQMEventBandwidth;
    MonitoredQuantity _writtenDQMEventBandwidth;

    MonitoredQuantity _numberOfUpdates;


  public:

    struct DQMEventStats
    {
      MonitoredQuantity::Stats dqmEventSizeStats;             //MB
      MonitoredQuantity::Stats servedDQMEventSizeStats;       //MB
      MonitoredQuantity::Stats writtenDQMEventSizeStats;      //MB
      
      MonitoredQuantity::Stats dqmEventBandwidthStats;        //MB/s
      MonitoredQuantity::Stats servedDQMEventBandwidthStats;  //MB/s
      MonitoredQuantity::Stats writtenDQMEventBandwidthStats; //MB/s

      MonitoredQuantity::Stats numberOfUpdatesStats; // number of received updates per DQMKey
    };

    explicit DQMEventMonitorCollection(xdaq::Application*);

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

    const MonitoredQuantity& getNumberOfUpdatesMQ() const {
      return _numberOfUpdates;
    }
    MonitoredQuantity& getNumberOfUpdatesMQ() {
      return _numberOfUpdates;
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
    
    virtual void do_updateInfoSpace();
    
    virtual void do_reset();

  };
  
} // namespace stor

#endif // StorageManager_DQMEventMonitorCollection_h 


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
