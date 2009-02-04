// $Id$

#ifndef StorageManager_MonitorCollection_h
#define StorageManager_MonitorCollection_h

#include <xercesc/dom/DOMElement.hpp>

#include "EventFilter/StorageManager/interface/MonitoredQuantity.h"


namespace stor {

  /**
   * An abstract collection of MonitoredQuantities
   *
   * $Author:$
   * $Revision:$
   * $Date:$
   */
  
  class MonitorCollection
  {
  public:

    // A virtual destructor here leads to a missing symbol
    // virtual ~MonitorCollection() = 0;

    /**
     * Calculates the statistics for all quantities
     */
    void calculateStatistics();

    /**
     * Updates the infospace
     */
    void updateInfoSpace();

    /**
     * Adds a child node to the parent node
     */
    xercesc::DOMElement* addDOMElement(xercesc::DOMElement*);


  protected:

    virtual void do_calculateStatistics() = 0;
    
    virtual xercesc::DOMElement* do_addDOMElement(xercesc::DOMElement*) = 0;
    
    virtual void do_updateInfoSpace() = 0;

  };
  
} // namespace stor

#endif // StorageManager_MonitorCollection_h 


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
