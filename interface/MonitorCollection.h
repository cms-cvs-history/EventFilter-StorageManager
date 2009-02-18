// $Id: MonitorCollection.h,v 1.1.2.8 2009/02/16 15:52:25 mommsen Exp $

#ifndef StorageManager_MonitorCollection_h
#define StorageManager_MonitorCollection_h

#include <list>
#include <vector>
#include <utility>
#include <string>

#include "xdaq/Application.h"
#include "xdata/InfoSpace.h"
#include "xdata/Serializable.h"

#include "EventFilter/StorageManager/interface/MonitoredQuantity.h"
#include "EventFilter/StorageManager/interface/XHTMLMaker.h"


namespace stor {

  /**
   * An abstract collection of MonitoredQuantities
   *
   * $Author: mommsen $
   * $Revision: 1.1.2.8 $
   * $Date: 2009/02/16 15:52:25 $
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
     * Calculates the statistics and updates the info space
     * for all monitored quantities
     */
    void update();

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
    void addDOMElement(XHTMLMaker&, XHTMLMaker::Node*) const;


  protected:

    virtual void do_calculateStatistics() = 0;
    
    virtual void do_addDOMElement(XHTMLMaker&, XHTMLMaker::Node*) const = 0;
    
    virtual void do_updateInfoSpace() = 0;


    // Stuff dealing with info space
    void putItemsIntoInfoSpace();

    xdata::InfoSpace *_infoSpace;

    typedef std::vector< std::pair<std::string, xdata::Serializable*> > infoSpaceItems_t;
    infoSpaceItems_t _infoSpaceItems;

    std::list<std::string> _infoSpaceItemNames;

  private:

    //Prevent copying of the MonitorCollection
    MonitorCollection(MonitorCollection const&);
    MonitorCollection& operator=(MonitorCollection const&);

  };
  
} // namespace stor

#endif // StorageManager_MonitorCollection_h 


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
