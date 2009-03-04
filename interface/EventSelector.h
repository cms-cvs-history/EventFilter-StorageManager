// $Id: EventSelector.h,v 1.1.2.1 2009/01/30 10:49:40 mommsen Exp $

#ifndef StorageManager_EventSelector_h
#define StorageManager_EventSelector_h

#include "EventFilter/StorageManager/interface/I2OChain.h"
#include "EventFilter/StorageManager/interface/Types.h"


namespace stor {

  /**
   * Adds QueueIDs to the event according to its header information
   *
   * $Author: mommsen $
   * $Revision: 1.1.2.1 $
   * $Date: 2009/01/30 10:49:40 $
   */
  
  class EventSelector
  {
  public:
    
    EventSelector();
    
    ~EventSelector();
    
    /*
     * Looks into the I2O and event header to decide into which
     * queues and streams the event should be put.
     */
    void markEvent(I2OChain&);


  private:
    
  };
  
} // namespace stor

#endif // StorageManager_EventSelector_h 


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
