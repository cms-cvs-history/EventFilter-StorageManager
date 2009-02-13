// $Id: FragmentMonitorCollection.h,v 1.1.2.7 2009/02/12 11:24:57 mommsen Exp $

#ifndef StorageManager_FragmentMonitorCollection_h
#define StorageManager_FragmentMonitorCollection_h

#include "xdata/UnsignedInteger32.h"
#include "xdata/Double.h"

#include "EventFilter/StorageManager/interface/MonitorCollection.h"


namespace stor {

  /**
   * A collection of MonitoredQuantities related to fragments
   *
   * $Author: mommsen $
   * $Revision: 1.1.2.7 $
   * $Date: 2009/02/12 11:24:57 $
   */
  
  class FragmentMonitorCollection : public MonitorCollection
  {
  private:

    MonitoredQuantity allFragmentSizes;
    MonitoredQuantity eventFragmentSizes;
    MonitoredQuantity dqmEventFragmentSizes;

    MonitoredQuantity allFragmentBandwidth;
    MonitoredQuantity eventFragmentBandwidth;
    MonitoredQuantity dqmEventFragmentBandwidth;


  public:

    explicit FragmentMonitorCollection(xdaq::Application*);

    void addEventFragmentSample(const double bytecount);

    void addDQMEventFragmentSample(const double bytecount);

    const MonitoredQuantity& getAllFragmentSizeMQ() const {
      return allFragmentSizes;
    }
    MonitoredQuantity& getAllFragmentSizeMQ() {
      return allFragmentSizes;
    }

    const MonitoredQuantity& getEventFragmentSizeMQ() const {
      return eventFragmentSizes;
    }
    MonitoredQuantity& getEventFragmentSizeMQ() {
      return eventFragmentSizes;
    }

    const MonitoredQuantity& getDQMEventFragmentSizeMQ() const {
      return dqmEventFragmentSizes;
    }
    MonitoredQuantity& getDQMEventFragmentSizeMQ() {
      return dqmEventFragmentSizes;
    }

    const MonitoredQuantity& getAllFragmentBandwidthMQ() const {
      return allFragmentBandwidth;
    }
    MonitoredQuantity& getAllFragmentBandwidthMQ() {
      return allFragmentBandwidth;
    }

    const MonitoredQuantity& getEventFragmentBandwidthMQ() const {
      return eventFragmentBandwidth;
    }
    MonitoredQuantity& getEventFragmentBandwidthMQ() {
      return eventFragmentBandwidth;
    }

    const MonitoredQuantity& getDQMEventFragmentBandwidthMQ() const {
      return dqmEventFragmentBandwidth;
    }
    MonitoredQuantity& getDQMEventFragmentBandwidthMQ() {
      return dqmEventFragmentBandwidth;
    }


  private:

    virtual void do_calculateStatistics();
    
    virtual void do_addDOMElement(xercesc::DOMElement*) const;
    
    virtual void do_updateInfoSpace();


    // InfoSpace items which were defined in the old SM
    xdata::Double _duration;                  // Duration of run in seconds
    xdata::UnsignedInteger32 _receivedFrames; // Total I2O frames received
    xdata::UnsignedInteger32 _totalSamples;   // Total number of samples used for measurement
                                              // (same as receivedFrames)
    xdata::UnsignedInteger32 _dqmRecords;     // Total number of DQM records (frames) received

    xdata::Double _meanBandwidth;    // Total average bandwidth in MB/s
    xdata::Double _meanRate;         // Total avarage number of frames/s
    xdata::Double _meanLatency;      // Total average latency in micro-seconds/frame
    xdata::Double _receivedVolume;   // Total received data in MB

    xdata::UnsignedInteger32 _receivedPeriod4Stats;  // Time period per recent measurements
    xdata::UnsignedInteger32 _receivedSamples4Stats; // Number of recent samples used for measurement
    xdata::Double _instantBandwidth; // Recent bandwidth in MB/s
    xdata::Double _instantRate;      // Recent number of frames/s
    xdata::Double _instantLatency;   // Recent latency in micro-seconds/frame
    xdata::Double _maxBandwidth;     // Recent maximum bandwidth in MB/s
    xdata::Double _minBandwidth;     // Recent minimum bandwidth in MB/s

    // Why are these put into infospace if none of the DQM related measurements are?
    xdata::UnsignedInteger32 _receivedDQMPeriod4Stats;  // Number of recent samples used for DQM measurement
    xdata::UnsignedInteger32 _receivedDQMSamples4Stats; // Time period per recent DQMmeasurements

  };
  
} // namespace stor

#endif // StorageManager_FragmentMonitorCollection_h 


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
