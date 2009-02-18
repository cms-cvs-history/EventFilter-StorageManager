// $Id: RunMonitorCollection.h,v 1.1.2.4 2009/02/16 15:52:25 mommsen Exp $

#ifndef StorageManager_RunMonitorCollection_h
#define StorageManager_RunMonitorCollection_h

#include "xdata/UnsignedInteger32.h"

#include "EventFilter/StorageManager/interface/MonitorCollection.h"


namespace stor {

  /**
   * A collection of MonitoredQuantities related to events received
   * in the current run
   *
   * $Author: mommsen $
   * $Revision: 1.1.2.4 $
   * $Date: 2009/02/16 15:52:25 $
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

    //Prevent copying of the RunMonitorCollection
    RunMonitorCollection(RunMonitorCollection const&);
    RunMonitorCollection& operator=(RunMonitorCollection const&);

    virtual void do_calculateStatistics();
    
    virtual void do_addDOMElement(XHTMLMaker&, XHTMLMaker::Node*) const;
    
    virtual void do_updateInfoSpace();


    // InfoSpace items which were defined in the old SM
    xdata::UnsignedInteger32 _runNumber;           // The current run number
    xdata::UnsignedInteger32 _receivedEvents;      // Total number of received events
    xdata::UnsignedInteger32 _receivedErrorEvents; // Total number of received error events

  };
  
} // namespace stor

#endif // StorageManager_RunMonitorCollection_h 


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
