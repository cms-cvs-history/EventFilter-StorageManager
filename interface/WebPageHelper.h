// $Id: WebPageHelper.h,v 1.1.2.7 2009/03/02 18:08:21 biery Exp $

#ifndef StorageManager_WebPageHelper_h
#define StorageManager_WebPageHelper_h

#include <string>

#include "boost/shared_ptr.hpp"
#include "boost/thread/mutex.hpp"

#include "toolbox/mem/Pool.h"
#include "xdaq/ApplicationDescriptor.h"
#include "xgi/Output.h"

#include "EventFilter/StorageManager/interface/StatisticsReporter.h"
#include "EventFilter/StorageManager/interface/FragmentMonitorCollection.h"
#include "EventFilter/StorageManager/interface/RunMonitorCollection.h"
#include "EventFilter/StorageManager/interface/XHTMLMaker.h"

namespace stor {

  /**
   * Helper class to handle web page requests
   *
   * $Author: biery $
   * $Revision: 1.1.2.7 $
   * $Date: 2009/03/02 18:08:21 $
   */
  
  class WebPageHelper
  {
  public:

    /**
     * Generates the default monitoring webpage for the SM
     */
    static void defaultWebPage
    (
      xgi::Output*, 
      const std::string stateName,
      const boost::shared_ptr<StatisticsReporter>&,
      toolbox::mem::Pool*,
      const int nLogicalDisk,
      const std::string filePath,
      xdaq::ApplicationDescriptor*
    );

    /**
     * Returns the number of instances for the given process name
     */
    static int getProcessCount(std::string processName);


  private:

    /**
     * Returns the webpage body with the standard header as XHTML node
     */
    static XHTMLMaker::Node* createWebPageBody(XHTMLMaker&,
                                               const std::string stateName,
                                               xdaq::ApplicationDescriptor*);

    /**
     * Adds the links for the other SM webpages
     */
    static void addDOMforSMLinks(XHTMLMaker&, XHTMLMaker::Node *parent,
                                 xdaq::ApplicationDescriptor*);

    /**
     * Adds the resource table to the parent DOM element
     */
    static void addDOMforResourceUsage
    (
      XHTMLMaker&,
      XHTMLMaker::Node *parent,
      toolbox::mem::Pool*,
      const int nLogicalDisk,
      const std::string filePath
    );

    /**
     * Adds fragment monitoring statistics to the parent DOM element
     */
    static void addDOMforFragmentMonitor(XHTMLMaker& maker,
                                         XHTMLMaker::Node *parent,
                                         FragmentMonitorCollection const& fmc);

    /**
     * Adds run monitoring statistics to the parent DOM element
     */
    static void addDOMforRunMonitor(XHTMLMaker& maker,
                                    XHTMLMaker::Node *parent,
                                    RunMonitorCollection const& rmc);

  private:

    static boost::mutex _xhtmlMakerMutex;

  };

} // namespace stor

#endif // StorageManager_WebPageHelper_h 


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
