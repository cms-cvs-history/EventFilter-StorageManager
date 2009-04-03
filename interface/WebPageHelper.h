// $Id: WebPageHelper.h,v 1.1.2.8 2009/04/03 10:59:00 mommsen Exp $

#ifndef StorageManager_WebPageHelper_h
#define StorageManager_WebPageHelper_h

#include <string>

#include "boost/thread/mutex.hpp"

#include "toolbox/mem/Pool.h"
#include "xdaq/ApplicationDescriptor.h"
#include "xgi/Output.h"

#include "EventFilter/StorageManager/interface/FilesMonitorCollection.h"
#include "EventFilter/StorageManager/interface/FragmentMonitorCollection.h"
#include "EventFilter/StorageManager/interface/RunMonitorCollection.h"
#include "EventFilter/StorageManager/interface/SharedResources.h"
#include "EventFilter/StorageManager/interface/StatisticsReporter.h"
#include "EventFilter/StorageManager/interface/XHTMLMaker.h"

namespace stor {

  /**
   * Helper class to handle web page requests
   *
   * $Author: mommsen $
   * $Revision: 1.1.2.8 $
   * $Date: 2009/04/03 10:59:00 $
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
      const SharedResourcesPtr,
      xdaq::ApplicationDescriptor*,
      toolbox::mem::Pool*
    );

    /**
     * Generates the files monitoring webpage for the SM
     */
    static void filesWebPage
    (
      xgi::Output*,
      const StatisticsReporterPtr,
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
      const DiskWritingParams
    );

    /**
     * Adds fragment monitoring statistics to the parent DOM element
     */
    static void addDOMforFragmentMonitor(XHTMLMaker& maker,
                                         XHTMLMaker::Node *parent,
                                         FragmentMonitorCollection const&);

    /**
     * Adds run monitoring statistics to the parent DOM element
     */
    static void addDOMforRunMonitor(XHTMLMaker& maker,
                                    XHTMLMaker::Node *parent,
                                    RunMonitorCollection const&);

    /**
     * Adds files statistics to the parent DOM element
     */
    static void addDOMforFiles(XHTMLMaker& maker,
                               XHTMLMaker::Node *parent,
                               FilesMonitorCollection const&);


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
