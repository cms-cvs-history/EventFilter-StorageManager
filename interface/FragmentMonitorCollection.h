// $Id$

#ifndef StorageManager_FragmentMonitorCollection_h
#define StorageManager_FragmentMonitorCollection_h

#include "EventFilter/StorageManager/interface/MonitorCollection.h"


namespace stor {

  /**
   * A collection of MonitoredQuantities related to fragments
   *
   * $Author:$
   * $Revision:$
   * $Date:$
   */
  
  class FragmentMonitorCollection : public MonitorCollection
  {
  private:

    MonitoredQuantity allFragmentSizes;
    MonitoredQuantity eventFragmentSizes;
    MonitoredQuantity dqmEventFragmentSizes;


  public:

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

  };
  
} // namespace stor

#endif // StorageManager_FragmentMonitorCollection_h 


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
