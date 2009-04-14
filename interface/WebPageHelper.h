// $Id: WebPageHelper.h,v 1.1.2.11 2009/04/09 14:00:04 mommsen Exp $

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
#include "EventFilter/StorageManager/interface/StreamsMonitorCollection.h"
#include "EventFilter/StorageManager/interface/XHTMLMaker.h"

namespace stor {

  /**
   * Helper class to handle web page requests
   *
   * $Author: mommsen $
   * $Revision: 1.1.2.11 $
   * $Date: 2009/04/09 14:00:04 $
   */
  
  class WebPageHelper
  {
  public:

    WebPageHelper
    (
      xdaq::ApplicationDescriptor*,
      const std::string SMversion
    );


    /**
     * Generates the default monitoring webpage
     */
    void defaultWebPage
    (
      xgi::Output*, 
      const SharedResourcesPtr,
      toolbox::mem::Pool*
    );

    /**
     * Generates the output streams monitoring webpage
     */
    void storedDataWebPage
    (
      xgi::Output*,
      const SharedResourcesPtr
    );

    /**
     * Generates the files monitoring webpage
     */
    void filesWebPage
    (
      xgi::Output*,
      const SharedResourcesPtr
    );

    /**
     * Returns the number of instances for the given process name
     */
    int getProcessCount(std::string processName);


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
      DiskWritingParams const&
    );

    /**
     * Adds fragment monitoring statistics to the parent DOM element
     */
    void addDOMforFragmentMonitor
    (
      XHTMLMaker& maker,
      XHTMLMaker::Node *parent,
      FragmentMonitorCollection const&
    );

    /**
     * Adds run monitoring statistics to the parent DOM element
     */
    void addDOMforRunMonitor
    (
      XHTMLMaker& maker,
      XHTMLMaker::Node *parent,
      RunMonitorCollection const&
    );

    /**
     * Adds stored data statistics to the parent DOM element
     */
    void addDOMforStoredData
    (
      XHTMLMaker& maker,
      XHTMLMaker::Node *parent,
      StreamsMonitorCollection const&
    );

    /**
     * Adds the SM config string to the parent DOM element
     */
    void addDOMforConfigString
    (
      XHTMLMaker& maker,
      XHTMLMaker::Node *parent,
      DiskWritingParams const&
    );

    /**
     * Adds files statistics to the parent DOM element
     */
    void addDOMforFiles
    (
      XHTMLMaker& maker,
      XHTMLMaker::Node *parent,
      FilesMonitorCollection const&
    );

    /**
     * List stream records statistics
     */
    void listStreamRecordsStats
    (
      XHTMLMaker& maker,
      XHTMLMaker::Node *table,
      StreamsMonitorCollection const&,
      MonitoredQuantity::DataSetType
    );

  private:

    //Prevent copying of the WebPageHelper
    WebPageHelper(WebPageHelper const&);
    WebPageHelper& operator=(WebPageHelper const&);

    static boost::mutex _xhtmlMakerMutex;
    xdaq::ApplicationDescriptor* _appDescriptor;
    const std::string _smVersion;

    XHTMLMaker::AttrMap _tableAttr;
    XHTMLMaker::AttrMap _specialRowAttr;

  };

} // namespace stor

#endif // StorageManager_WebPageHelper_h 


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
