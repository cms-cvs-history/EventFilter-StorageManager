// $Id: FragmentMonitorCollection.h,v 1.1.2.1 2009/02/04 13:27:08 mommsen Exp $

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
   * $Revision: 1.1.2.1 $
   * $Date: 2009/02/04 13:27:08 $
   */
  
  class FragmentMonitorCollection : public MonitorCollection
  {
  private:

    MonitoredQuantity allFragmentSizes;
    MonitoredQuantity eventFragmentSizes;
    MonitoredQuantity dqmEventFragmentSizes;


  public:

    FragmentMonitorCollection(xdaq::Application*);

    void addAllFragmentSizeSample(double size)
    {
      allFragmentSizes.addSample(size);
    }

    void addEventFragmentSizeSample(double size)
    {
      eventFragmentSizes.addSample(size);
    }

    void addDQMEventFragmentSizeSample(double size)
    {
      dqmEventFragmentSizes.addSample(size);
    }


  private:

    virtual void do_calculateStatistics();
    
    virtual xercesc::DOMElement* do_addDOMElement(xercesc::DOMElement*);
    
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
