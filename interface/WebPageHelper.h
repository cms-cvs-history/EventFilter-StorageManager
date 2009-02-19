// $Id: WebPageHelper.h,v 1.1.2.5 2009/02/17 14:58:55 mommsen Exp $

#ifndef StorageManager_WebPageHelper_h
#define StorageManager_WebPageHelper_h

#include <string>

#include "boost/shared_ptr.hpp"

#include "toolbox/mem/Pool.h"
#include "xdaq/ApplicationDescriptor.h"
#include "xgi/Output.h"

#include "EventFilter/StorageManager/interface/StatisticsReporter.h"
#include "EventFilter/StorageManager/interface/XHTMLMaker.h"

namespace stor {

  /**
   * Helper class to handle web page requests
   *
   * $Author: mommsen $
   * $Revision: 1.1.2.5 $
   * $Date: 2009/02/17 14:58:55 $
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
      const boost::shared_ptr<StatisticsReporter>&,
      toolbox::mem::Pool*,
      const int nLogicalDisk,
      const std::string filePath
    );

    /**
     * Returns the number of instances for the given process name
     */
    int getProcessCount(std::string processName) const;


  private:

    /**
     * Returns the webpage body with the standard header as XHTML node
     */
    XHTMLMaker::Node* createWebPageBody(XHTMLMaker&, const std::string stateName);

    /**
     * Adds the links for the other SM webpages
     */
    void addDOMforSMLinks(XHTMLMaker&, XHTMLMaker::Node *parent);
    
    /**
     * Adds the resource table to the parent DOM element
     */
    void addDOMforResourceUsage
    (
      XHTMLMaker&,
      XHTMLMaker::Node *parent,
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