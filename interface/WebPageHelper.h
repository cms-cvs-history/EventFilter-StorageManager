// $Id: WebPageHelper.h,v 1.1.2.1 2009/02/12 11:22:40 mommsen Exp $

#ifndef StorageManager_WebPageHelper_h
#define StorageManager_WebPageHelper_h

#include <string>

#include "toolbox/mem/Pool.h"
#include "xdaq/ApplicationDescriptor.h"
#include "xgi/Output.h"

#include "EventFilter/StorageManager/interface/FragmentMonitorCollection.h"
#include "EventFilter/StorageManager/interface/RunMonitorCollection.h"
#include "EventFilter/StorageManager/interface/XHTMLMaker.h"

namespace stor {

  /**
   * Helper class to handle web page requests
   *
   * $Author: mommsen $
   * $Revision: 1.1.2.1 $
   * $Date: 2009/02/12 11:22:40 $
   */
  
  class WebPageHelper
  {
  public:
    
    explicit WebPageHelper(xdaq::ApplicationDescriptor*);

    /**
     * Generates the default monitoring webpage for the SM
     */
    void defaultWebPage
    (
      xgi::Output*, 
      const std::string stateName,
      const RunMonitorCollection&,
      const FragmentMonitorCollection&,
      toolbox::mem::Pool*,
      const int nLogicalDisk,
      const std::string filePath
    );


  private:

    /**
     * Returns the webpage body with the standard header as XHTML node
     */
    XHTMLMaker::Node* createWebPageBody(const std::string stateName);
    
    /**
     * Adds the resource table to the parent DOM element
     */
    void addDOMforResourceUsage
    (
      xercesc::DOMElement *parent,
      toolbox::mem::Pool*,
      const int nLogicalDisk,
      const std::string filePath
    );

  
  private:

    xdaq::ApplicationDescriptor *_appDescriptor;
    
  };
  
} // namespace stor

#endif // StorageManager_WebPageHelper_h 


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
