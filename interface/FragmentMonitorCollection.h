// $Id: FragmentMonitorCollection.h,v 1.1.2.5 2009/02/06 09:58:18 mommsen Exp $

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
   * $Revision: 1.1.2.5 $
   * $Date: 2009/02/06 09:58:18 $
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
    
    virtual void do_addDOMElement(xercesc::DOMElement*);
    
    virtual void do_updateInfoSpace();


    //InfoSpace items
    xdata::UnsignedInteger32 _receivedFrames;      // Rate of received I2O frames
    xdata::UnsignedInteger32 _receivedFramesSize;  // Total size of received I2O frames
    xdata::Double _receivedFramesBandwidth;        // Recent input bandwidth of I2O frames

  };
  
} // namespace stor

#endif // StorageManager_FragmentMonitorCollection_h 


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
