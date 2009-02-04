// $Id: MonitorCollection.h,v 1.1.2.1 2009/02/04 13:27:08 mommsen Exp $

#ifndef StorageManager_MonitorCollection_h
#define StorageManager_MonitorCollection_h

#include <xercesc/dom/DOMElement.hpp>

#include "EventFilter/StorageManager/interface/MonitoredQuantity.h"


namespace stor {

  /**
   * An abstract collection of MonitoredQuantities
   *
   * $Author: mommsen $
   * $Revision: 1.1.2.1 $
   * $Date: 2009/02/04 13:27:08 $
   */
  
  class MonitorCollection
  {
  public:

    // A pure virtual destructor results in a missing symbol
    virtual ~MonitorCollection() {};

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
