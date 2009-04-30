// $Id: WebPageHelper.h,v 1.1.2.15 2009/04/28 18:30:16 biery Exp $

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
#include "EventFilter/StorageManager/interface/Utils.h"
#include "EventFilter/StorageManager/interface/XHTMLMaker.h"

namespace stor {

  /**
   * Helper class to handle web page requests
   *
   * $Author: biery $
   * $Revision: 1.1.2.15 $
   * $Date: 2009/04/28 18:30:16 $
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


    /**
       Generates consumer statistics page
    */
    void consumerStatistics( xgi::Output*,
                             const SharedResourcesPtr );

    /**
       Generates the data sender web page for all resource brokers
    */
    void resourceBrokerOverview( xgi::Output*,
                                 const SharedResourcesPtr );

    /**
       Generates the data sender web page for a specific resource broker
    */
    void resourceBrokerDetail( xgi::Output*,
                               const SharedResourcesPtr,
                               long long );

  private:

    /**
      Get base url
    */
    std::string baseURL() const;

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
      const MonitoredQuantity::DataSetType
    );

    /**
     * Add statistics for received fragments
     */
    void addFragmentStats
    (
      XHTMLMaker& maker,
      XHTMLMaker::Node *table,
      FragmentMonitorCollection::FragmentStats const&,
      const MonitoredQuantity::DataSetType dataSet
    );

    /**
     * Add header with integration duration
     */
    void addDurationToTableHead
    (
      XHTMLMaker& maker,
      XHTMLMaker::Node *tableRow,
      const utils::duration_t
    );
    
    /**
     * Add a table row for number of fragment frames received
     */
    void addRowForFramesReceived
    (
      XHTMLMaker& maker,
      XHTMLMaker::Node *table,
      FragmentMonitorCollection::FragmentStats const&,
      const MonitoredQuantity::DataSetType dataSet
    );

    /**
     * Add a table row for fragment bandwidth
     */
    void addRowForBandwidth
    (
      XHTMLMaker& maker,
      XHTMLMaker::Node *table,
      FragmentMonitorCollection::FragmentStats const&,
      const MonitoredQuantity::DataSetType dataSet
    );

    /**
     * Add a table row for fragment rate
     */
    void addRowForRate
    (
      XHTMLMaker& maker,
      XHTMLMaker::Node *table,
      FragmentMonitorCollection::FragmentStats const&,
      const MonitoredQuantity::DataSetType dataSet
    );

    /**
     * Add a table row for fragment latency
     */
    void addRowForLatency
    (
      XHTMLMaker& maker,
      XHTMLMaker::Node *table,
      FragmentMonitorCollection::FragmentStats const&,
      const MonitoredQuantity::DataSetType dataSet
    );

    /**
     * Add a table row for total fragment volume received
     */
    void addRowForTotalVolume
    (
      XHTMLMaker& maker,
      XHTMLMaker::Node *table,
      FragmentMonitorCollection::FragmentStats const&,
      const MonitoredQuantity::DataSetType dataSet
    );

    /**
     * Add a table row for maximum fragment bandwidth
     */
    void addRowForMaxBandwidth
    (
      XHTMLMaker& maker,
      XHTMLMaker::Node *table,
      FragmentMonitorCollection::FragmentStats const&,
      const MonitoredQuantity::DataSetType dataSet
    );

    /**
     * Add a table row for minimum fragment bandwidth
     */
    void addRowForMinBandwidth
    (
      XHTMLMaker& maker,
      XHTMLMaker::Node *table,
      FragmentMonitorCollection::FragmentStats const&,
      const MonitoredQuantity::DataSetType dataSet
    );

    /**
     * Adds top-level output module statistics to the parent DOM element
     */
    void addOutputModuleStatistics
    (
      XHTMLMaker& maker,
      XHTMLMaker::Node *parent,
      DataSenderMonitorCollection const&
    );

    /**
     * Adds output module statistics from the specified resource
     * broker to the parent DOM element
     */
    void addOutputModuleStatistics
    (
      XHTMLMaker& maker,
      XHTMLMaker::Node *parent,
      long long localRBID,
      DataSenderMonitorCollection const&
    );

    /**
     * Adds output module statistics to the parent DOM element
     */
    void addOutputModuleStatistics
    (
      XHTMLMaker& maker,
      XHTMLMaker::Node *parent,
      DataSenderMonitorCollection::OutputModuleResultsList const&
    );

    /**
     * Adds the list of data senders (resource brokers) to the
     * parent DOM element
     */
    void addResourceBrokerList
    (
      XHTMLMaker& maker,
      XHTMLMaker::Node *parent,
      DataSenderMonitorCollection const&
    );

    /**
     * Adds information about a specific resource broker to the
     * parent DOM element
     */
    void addResourceBrokerDetails
    (
      XHTMLMaker& maker,
      XHTMLMaker::Node *parent,
      long long localRBID,
      DataSenderMonitorCollection const&
    );


  private:

    //Prevent copying of the WebPageHelper
    WebPageHelper(WebPageHelper const&);
    WebPageHelper& operator=(WebPageHelper const&);

    static boost::mutex _xhtmlMakerMutex;
    xdaq::ApplicationDescriptor* _appDescriptor;
    const std::string _smVersion;

    XHTMLMaker::AttrMap _tableAttr;
    XHTMLMaker::AttrMap _tableLabelAttr;
    XHTMLMaker::AttrMap _tableValueAttr;
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
