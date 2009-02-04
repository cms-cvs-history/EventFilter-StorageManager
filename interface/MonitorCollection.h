// $Id: MonitorCollection.h,v 1.1.2.2 2009/02/04 13:33:18 mommsen Exp $

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
   * $Revision: 1.1.2.2 $
   * $Date: 2009/02/04 13:33:18 $
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
    xercesc::DOMElement* addDOMElement(xercesc::DOMElement*);


  protected:

    virtual void do_calculateStatistics() = 0;
    
    virtual xercesc::DOMElement* do_addDOMElement(xercesc::DOMElement*) = 0;
    
    virtual void do_updateInfoSpace() = 0;


    // Stuff dealing with info space
    void putItemsIntoInfoSpace();

    const std::string _infoSpaceName;
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
