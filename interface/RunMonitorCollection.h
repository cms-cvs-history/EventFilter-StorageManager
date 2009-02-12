// $Id: RunMonitorCollection.h,v 1.1.2.7 2009/02/12 11:24:57 mommsen Exp $

#ifndef StorageManager_RunMonitorCollection_h
#define StorageManager_RunMonitorCollection_h

#include "xdata/UnsignedInteger32.h"
#include "xdata/Double.h"

#include "EventFilter/StorageManager/interface/MonitorCollection.h"


namespace stor {

  /**
   * A collection of MonitoredQuantities related to events received
   * in the current run
   *
   * $Author: mommsen $
   * $Revision: 1.1.2.7 $
   * $Date: 2009/02/12 11:24:57 $
   */
  
  class RunMonitorCollection : public MonitorCollection
  {
  private:

    MonitoredQuantity eventIDsReceived;
    MonitoredQuantity errorEventIDsReceived;
    MonitoredQuantity runNumbersSeen;  // Does this make sense?
    MonitoredQuantity lumiSectionsSeen;


  public:

    explicit RunMonitorCollection(xdaq::Application*);

    const MonitoredQuantity& getEventIDsReceivedMQ() const {
      return eventIDsReceived;
    }
    MonitoredQuantity& getEventIDsReceivedMQ() {
      return eventIDsReceived;
    }

    const MonitoredQuantity& getErrorEventIDsReceivedMQ() const {
      return errorEventIDsReceived;
    }
    MonitoredQuantity& getErrorEventIDsReceivedMQ() {
      return errorEventIDsReceived;
    }

    const MonitoredQuantity& getRunNumbersSeenMQ() const {
      return runNumbersSeen;
    }
    MonitoredQuantity& getRunNumbersSeenMQ() {
      return runNumbersSeen;
    }

    const MonitoredQuantity& getLumiSectionsSeenMQ() const {
      return lumiSectionsSeen;
    }
    MonitoredQuantity& getLumiSectionsSeenMQ() {
      return lumiSectionsSeen;
    }


  private:

    virtual void do_calculateStatistics();
    
    virtual void do_addDOMElement(xercesc::DOMElement*) const;
    
    virtual void do_updateInfoSpace();


    //InfoSpace items
    xdata::UnsignedInteger32 _runNumber;           // The current run number

  };
  
} // namespace stor

#endif // StorageManager_RunMonitorCollection_h 


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
