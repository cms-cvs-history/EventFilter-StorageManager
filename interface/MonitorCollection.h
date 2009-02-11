// $Id: MonitorCollection.h,v 1.1.2.4 2009/02/05 10:09:49 mommsen Exp $

#ifndef StorageManager_MonitorCollection_h
#define StorageManager_MonitorCollection_h

#include <list>
#include <vector>
#include <utility>
#include <string>
#include <xercesc/dom/DOMElement.hpp>

#include "xdaq/Application.h"
#include "xdata/InfoSpace.h"
#include "xdata/Serializable.h"

#include "EventFilter/StorageManager/interface/MonitoredQuantity.h"


namespace stor {

  /**
   * An abstract collection of MonitoredQuantities
   *
   * $Author: mommsen $
   * $Revision: 1.1.2.4 $
   * $Date: 2009/02/05 10:09:49 $
   */
  
  class MonitorCollection
  {
  public:

    MonitorCollection
    (
      xdaq::Application*,
      const std::string infoSpaceName
    );

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
    void addDOMElement(xercesc::DOMElement*);


  protected:

    virtual void do_calculateStatistics() = 0;
    
    virtual void do_addDOMElement(xercesc::DOMElement*) = 0;
    
    virtual void do_updateInfoSpace() = 0;


    // Stuff dealing with info space
    void putItemsIntoInfoSpace();

    xdata::InfoSpace *_infoSpace;

    typedef std::vector< std::pair<std::string, xdata::Serializable*> > infoSpaceItems_t;
    infoSpaceItems_t _infoSpaceItems;

    std::list<std::string> _infoSpaceItemNames;
  };
  
} // namespace stor

#endif // StorageManager_MonitorCollection_h 


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -