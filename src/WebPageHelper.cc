// $Id: WebPageHelper.cc,v 1.1.2.52 2009/05/29 12:39:29 dshpakov Exp $

#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <sys/statfs.h>

#include "boost/lexical_cast.hpp"

#include "EventFilter/StorageManager/interface/ConsumerMonitorCollection.h"
#include "EventFilter/StorageManager/interface/MonitoredQuantity.h"
#include "EventFilter/StorageManager/interface/RegistrationCollection.h"
#include "EventFilter/StorageManager/interface/WebPageHelper.h"
#include "EventFilter/StorageManager/interface/XHTMLMonitor.h"

using namespace stor;


boost::mutex WebPageHelper::_xhtmlMakerMutex;

WebPageHelper::WebPageHelper
(
  xdaq::ApplicationDescriptor* appDesc,
  const std::string SMversion
) :
_appDescriptor(appDesc),
_smVersion(SMversion)
{
  // set application icon for hyperdaq
  appDesc->setAttribute("icon", "/evf/images/smicon.jpg");

  _tableAttr[ "frame" ] = "void";
  _tableAttr[ "rules" ] = "group";
  _tableAttr[ "class" ] = "states";
  _tableAttr[ "cellspacing" ] = "0";
  _tableAttr[ "cellpadding" ] = "2";
  _tableAttr[ "width" ] = "100%";
  _tableAttr[ "valign" ] = "top";

  _rowAttr[ "valign" ] = "top";
  
  _specialRowAttr = _rowAttr;
  _specialRowAttr[ "class" ] = "special";

  _tableLabelAttr[ "align" ] = "left";

  _tableValueAttr[ "align" ] = "right";
}


void WebPageHelper::defaultWebPage
(
  xgi::Output *out, 
  const SharedResourcesPtr sharedResources
)
{
  boost::mutex::scoped_lock lock(_xhtmlMakerMutex);
  XHTMLMonitor theMonitor;
  XHTMLMaker maker;
  
  StatisticsReporterPtr statReporter = sharedResources->_statisticsReporter;

  // Create the body with the standard header
  XHTMLMaker::Node* body = createWebPageBody(maker, statReporter);

  // Show host name:
  XHTMLMaker::Node* hn_table = maker.addNode( "table", body );
  XHTMLMaker::Node* hn_tbody = maker.addNode( "tbody", hn_table );
  XHTMLMaker::Node* hn_tr = maker.addNode( "tr", hn_tbody );
  XHTMLMaker::Node* hn_td = maker.addNode( "td", hn_tr );
  std::string hname( "Running on host: " );
  hname += sharedResources->_configuration->getDiskWritingParams()._hostName;
  maker.addText( hn_td, hname );

  //TODO: Failed printout

  // Run and event summary
  addDOMforRunMonitor(maker, body, statReporter->getRunMonitorCollection());
  
  // Resource usage
  addDOMforResourceUsage(maker, body, 
    statReporter->getResourceMonitorCollection());
  
  // Add the received data statistics table
  addDOMforFragmentMonitor(maker, body,
                           statReporter->getFragmentMonitorCollection());

  addDOMforSMLinks(maker, body);
  
  // Dump the webpage to the output stream
  maker.out(*out);
}


void WebPageHelper::storedDataWebPage
(
  xgi::Output *out,
  const SharedResourcesPtr sharedResources
)
{
  boost::mutex::scoped_lock lock(_xhtmlMakerMutex);
  XHTMLMonitor theMonitor;
  XHTMLMaker maker;
  
  StatisticsReporterPtr statReporter = sharedResources->_statisticsReporter;
  
  // Create the body with the standard header
  XHTMLMaker::Node* body = createWebPageBody(maker, statReporter); 

  addDOMforStoredData(maker, body, statReporter->getStreamsMonitorCollection());

  maker.addNode("hr", body);

  addDOMforConfigString(maker, body, sharedResources->_configuration->getDiskWritingParams());  
  
  addDOMforSMLinks(maker, body);
  
   // Dump the webpage to the output stream
  maker.out(*out);
}


void WebPageHelper::filesWebPage
(
  xgi::Output *out,
  const SharedResourcesPtr sharedResources
)
{
  boost::mutex::scoped_lock lock(_xhtmlMakerMutex);
  XHTMLMonitor theMonitor;
  XHTMLMaker maker;
  
  StatisticsReporterPtr statReporter = sharedResources->_statisticsReporter;
  
  // Create the body with the standard header
  XHTMLMaker::Node* body = createWebPageBody(maker, statReporter);

  addDOMforFiles(maker, body, statReporter->getFilesMonitorCollection());  

  addDOMforSMLinks(maker, body);
  
   // Dump the webpage to the output stream
  maker.out(*out);
}


//////////////////////////////
//// Consumer statistics: ////
//////////////////////////////
void WebPageHelper::consumerStatistics( xgi::Output* out,
                                        const SharedResourcesPtr resPtr )
{

  // Get lock, initialize maker:
  boost::mutex::scoped_lock lock( _xhtmlMakerMutex );
  XHTMLMonitor theMonitor;
  XHTMLMaker maker;

  // Make header:
  XHTMLMaker::Node* body = createWebPageBody( maker, resPtr->_statisticsReporter );

 {
  // Title:
  XHTMLMaker::AttrMap title_attr;
  title_attr[ "style" ] = "text-align:center;font-weight:bold";
  XHTMLMaker::Node* title = maker.addNode( "p", body, title_attr );
  maker.addText( title, "Consumer Statistics" );

  //
  //// Consumer summary table: ////
  //

  XHTMLMaker::AttrMap table_attr;
  table_attr[ "cellspacing" ] = "5";
  table_attr[ "border" ] = "1";
  XHTMLMaker::Node* cs_table = maker.addNode( "table", body, table_attr );
  XHTMLMaker::Node* cs_tbody = maker.addNode( "tbody", cs_table );
  XHTMLMaker::Node* cs_top_row = maker.addNode( "tr", cs_tbody, _rowAttr );

  // Cell titles:
  XHTMLMaker::Node* cs_th_id = maker.addNode( "th", cs_top_row );
  maker.addText( cs_th_id, "ID" );
  XHTMLMaker::Node* cs_th_name = maker.addNode( "th", cs_top_row );
  maker.addText( cs_th_name, "Name" );
  XHTMLMaker::Node* cs_th_status = maker.addNode( "th", cs_top_row );
  maker.addText( cs_th_status, "Status" );
  XHTMLMaker::Node* cs_th_hlt = maker.addNode( "th", cs_top_row );
  maker.addText( cs_th_hlt, "HLT Output Module" );
  XHTMLMaker::Node* cs_th_filters = maker.addNode( "th", cs_top_row );
  maker.addText( cs_th_filters, "Filters" );
  XHTMLMaker::Node* cs_th_policy = maker.addNode( "th", cs_top_row );
  maker.addText( cs_th_policy, "Enquing Policy" );
  XHTMLMaker::Node* cs_th_queued = maker.addNode( "th", cs_top_row );
  maker.addText( cs_th_queued, "Events Queued" );
  XHTMLMaker::Node* cs_th_served = maker.addNode( "th", cs_top_row );
  maker.addText( cs_th_served, "Events Served" );
  XHTMLMaker::Node* cs_th_served_rate = maker.addNode( "th", cs_top_row );
  maker.addText( cs_th_served_rate, "Served Event Rate" );

  boost::shared_ptr<RegistrationCollection> rc = resPtr->_registrationCollection;
  RegistrationCollection::ConsumerRegistrations regs;
  rc->getEventConsumers( regs );

  boost::shared_ptr<ConsumerMonitorCollection> cc =
    resPtr->_statisticsReporter->getEventConsumerMonitorCollection();

  // Loop over consumers:
  for( RegistrationCollection::ConsumerRegistrations::const_iterator it = regs.begin();
       it != regs.end(); ++it )
    {

      // Row:
      XHTMLMaker::Node* cs_tr = maker.addNode( "tr", cs_tbody, _rowAttr );

      // ID:
      std::ostringstream cid_oss;
      cid_oss << (*it)->consumerId();
      XHTMLMaker::Node* cs_td_id = maker.addNode( "td", cs_tr );
      maker.addText( cs_td_id, cid_oss.str() );

      // Name:
      XHTMLMaker::Node* cs_td_name = maker.addNode( "td", cs_tr );
      if ( (*it)->isProxyServer() )
        maker.addText( cs_td_name, "Proxy Server" );
      else
        maker.addText( cs_td_name, (*it)->consumerName() );

      // Status:
      XHTMLMaker::AttrMap status_attr;
      std::string status_message = "";
      if( (*it)->isStale() )
        {
          status_attr[ "style" ] = "color:brown";
          status_message = "Stale";
        }
      else
        {
          status_attr[ "style" ] = "color:green";
          status_message = "Active";
        }
      XHTMLMaker::Node* cs_td_status = maker.addNode( "td", cs_tr, status_attr );
      maker.addText( cs_td_status, status_message );

      // HLT output module:
      XHTMLMaker::Node* cs_td_hlt = maker.addNode( "td", cs_tr );
      maker.addText( cs_td_hlt, (*it)->selHLTOut() );

      // Filter list:
      std::string fl_str;
      const EventConsumerRegistrationInfo::FilterList fl = (*it)->selEvents();
      for( EventConsumerRegistrationInfo::FilterList::const_iterator lit = fl.begin();
           lit != fl.end(); ++lit )
        {
          if( lit != fl.begin() )
            {
              fl_str += "&nbsp;&nbsp;";
            }
          fl_str += *lit;
        }
      XHTMLMaker::Node* cs_td_filters = maker.addNode( "td", cs_tr );
      maker.addText( cs_td_filters, fl_str );

      // Policy:
      std::ostringstream policy_oss;
      policy_oss << (*it)->queuePolicy();
      XHTMLMaker::Node* cs_td_policy = maker.addNode( "td", cs_tr );
      maker.addText( cs_td_policy, policy_oss.str() );

      // Events queued:
      std::ostringstream eq_oss;
      MonitoredQuantity::Stats eq_stats;
      bool eq_found = cc->getQueued( (*it)->queueId(), eq_stats );
      if( eq_found )
        {
          eq_oss << eq_stats.getSampleCount();
        }
      else
        {
          eq_oss << "Not found";
        }
      XHTMLMaker::Node* cs_td_eq = maker.addNode( "td", cs_tr );
      maker.addText( cs_td_eq, eq_oss.str() );

      // Number and rate of served events:
      std::ostringstream es_oss;
      std::ostringstream rate_oss;
      MonitoredQuantity::Stats es_stats;
      bool es_found = cc->getServed( (*it)->queueId(), es_stats );
      if( es_found )
        {
          es_oss << es_stats.getSampleCount();
          rate_oss << es_stats.getSampleRate();
        }
      else
        {
          es_oss << "Not found";
          rate_oss << "Not found";
        }
      XHTMLMaker::Node* cs_td_es = maker.addNode( "td", cs_tr );
      maker.addText( cs_td_es, es_oss.str() );
      XHTMLMaker::Node* cs_td_rate = maker.addNode( "td", cs_tr );
      maker.addText( cs_td_rate, rate_oss.str() );

    }
 }

 {
  // Title:
  XHTMLMaker::AttrMap title_attr;
  title_attr[ "style" ] = "text-align:center;font-weight:bold";
  XHTMLMaker::Node* title = maker.addNode( "p", body, title_attr );
  maker.addText( title, "DQM Consumer Statistics" );

  //
  //// Consumer summary table: ////
  //

  XHTMLMaker::AttrMap table_attr;
  table_attr[ "cellspacing" ] = "5";
  table_attr[ "border" ] = "1";
  XHTMLMaker::Node* cs_table = maker.addNode( "table", body, table_attr );
  XHTMLMaker::Node* cs_tbody = maker.addNode( "tbody", cs_table );
  XHTMLMaker::Node* cs_top_row = maker.addNode( "tr", cs_tbody, _rowAttr );

  // Cell titles:
  XHTMLMaker::Node* cs_th_id = maker.addNode( "th", cs_top_row );
  maker.addText( cs_th_id, "ID" );
  XHTMLMaker::Node* cs_th_name = maker.addNode( "th", cs_top_row );
  maker.addText( cs_th_name, "Name" );
  XHTMLMaker::Node* cs_th_status = maker.addNode( "th", cs_top_row );
  maker.addText( cs_th_status, "Status" );
  XHTMLMaker::Node* cs_th_hlt = maker.addNode( "th", cs_top_row );
  maker.addText( cs_th_hlt, "Top Level Folder" );
  XHTMLMaker::Node* cs_th_policy = maker.addNode( "th", cs_top_row );
  maker.addText( cs_th_policy, "Enquing Policy" );
  XHTMLMaker::Node* cs_th_queued = maker.addNode( "th", cs_top_row );
  maker.addText( cs_th_queued, "Events Queued" );
  XHTMLMaker::Node* cs_th_served = maker.addNode( "th", cs_top_row );
  maker.addText( cs_th_served, "Events Served" );

  boost::shared_ptr<RegistrationCollection> rc = resPtr->_registrationCollection;
  RegistrationCollection::DQMConsumerRegistrations regs;
  rc->getDQMEventConsumers( regs );

  boost::shared_ptr<ConsumerMonitorCollection> cc =
    resPtr->_statisticsReporter->getDQMConsumerMonitorCollection();

  // Loop over consumers:
  for( RegistrationCollection::DQMConsumerRegistrations::const_iterator it = regs.begin();
       it != regs.end(); ++it )
    {

      // Row:
      XHTMLMaker::Node* cs_tr = maker.addNode( "tr", cs_tbody, _rowAttr );

      // ID:
      std::ostringstream cid_oss;
      cid_oss << (*it)->consumerId();
      XHTMLMaker::Node* cs_td_id = maker.addNode( "td", cs_tr );
      maker.addText( cs_td_id, cid_oss.str() );

      // Name:
      XHTMLMaker::Node* cs_td_name = maker.addNode( "td", cs_tr );
      if ( (*it)->isProxyServer() )
        maker.addText( cs_td_name, "Proxy Server" );
      else
        maker.addText( cs_td_name, (*it)->consumerName() );

      // Status:
      XHTMLMaker::AttrMap status_attr;
      std::string status_message = "";
      if( (*it)->isStale() )
        {
          status_attr[ "style" ] = "color:brown";
          status_message = "Stale";
        }
      else
        {
          status_attr[ "style" ] = "color:green";
          status_message = "Active";
        }
      XHTMLMaker::Node* cs_td_status = maker.addNode( "td", cs_tr, status_attr );
      maker.addText( cs_td_status, status_message );

      // Top level folder:
      XHTMLMaker::Node* cs_td_fld = maker.addNode( "td", cs_tr );
      maker.addText( cs_td_fld, (*it)->topLevelFolderName() );

      // Policy:
      std::ostringstream policy_oss;
      policy_oss << (*it)->queuePolicy();
      XHTMLMaker::Node* cs_td_policy = maker.addNode( "td", cs_tr );
      maker.addText( cs_td_policy, policy_oss.str() );

      // Events queued:
      std::ostringstream eq_oss;
      MonitoredQuantity::Stats eq_stats;
      bool eq_found = cc->getQueued( (*it)->queueId(), eq_stats );
      if( eq_found )
        {
          eq_oss << eq_stats.getSampleCount();
        }
      else
        {
          eq_oss << "Not found";
        }
      XHTMLMaker::Node* cs_td_eq = maker.addNode( "td", cs_tr );
      maker.addText( cs_td_eq, eq_oss.str() );

      // Number and rate of served events:
      std::ostringstream es_oss;
      std::ostringstream rate_oss;
      MonitoredQuantity::Stats es_stats;
      bool es_found = cc->getServed( (*it)->queueId(), es_stats );
      if( es_found )
        {
          es_oss << es_stats.getSampleCount();
          rate_oss << es_stats.getSampleRate();
        }
      else
        {
          es_oss << "Not found";
          rate_oss << "Not found";
        }
      XHTMLMaker::Node* cs_td_es = maker.addNode( "td", cs_tr );
      maker.addText( cs_td_es, es_oss.str() );

    }
 }

  // Write it:
  maker.out( *out );

}


void WebPageHelper::resourceBrokerOverview
(
  xgi::Output *out,
  const SharedResourcesPtr sharedResources
)
{
  boost::mutex::scoped_lock lock(_xhtmlMakerMutex);
  XHTMLMonitor theMonitor;
  XHTMLMaker maker;
  
  StatisticsReporterPtr statReporter = sharedResources->_statisticsReporter;
  
  // Create the body with the standard header
  XHTMLMaker::Node* body = createWebPageBody(maker, statReporter);

  addOutputModuleTables(maker, body,
                        statReporter->getDataSenderMonitorCollection());  

  maker.addNode("hr", body);

  addResourceBrokerList(maker, body,
                        statReporter->getDataSenderMonitorCollection());

  addDOMforSMLinks(maker, body);
  
   // Dump the webpage to the output stream
  maker.out(*out);
}


void WebPageHelper::resourceBrokerDetail
(
  xgi::Output *out,
  const SharedResourcesPtr sharedResources,
  long long uniqueRBID
)
{
  boost::mutex::scoped_lock lock(_xhtmlMakerMutex);
  XHTMLMonitor theMonitor;
  XHTMLMaker maker;
  
  StatisticsReporterPtr statReporter = sharedResources->_statisticsReporter;
  
  // Create the body with the standard header
  XHTMLMaker::Node* body = createWebPageBody(maker, statReporter);

  addResourceBrokerDetails(maker, body, uniqueRBID,
                           statReporter->getDataSenderMonitorCollection());  

  addOutputModuleStatistics(maker, body, uniqueRBID,
                            statReporter->getDataSenderMonitorCollection());  

  addFilterUnitList(maker, body, uniqueRBID,
                    statReporter->getDataSenderMonitorCollection());  

  addDOMforSMLinks(maker, body);
  
   // Dump the webpage to the output stream
  maker.out(*out);
}


void WebPageHelper::dqmEventWebPage
(
  xgi::Output *out,
  const SharedResourcesPtr sharedResources
)
{
  boost::mutex::scoped_lock lock(_xhtmlMakerMutex);
  XHTMLMonitor theMonitor;
  XHTMLMaker maker;
  
  StatisticsReporterPtr statReporter = sharedResources->_statisticsReporter;
  
  // Create the body with the standard header
  XHTMLMaker::Node* body = createWebPageBody(maker, statReporter);

  addDOMforProcessedDQMEvents(maker, body, statReporter->getDQMEventMonitorCollection());  
  addDOMforDQMEventStatistics(maker, body, statReporter->getDQMEventMonitorCollection());  

  addDOMforSMLinks(maker, body);
  
   // Dump the webpage to the output stream
  maker.out(*out);
}


void WebPageHelper::throughputWebPage
(
  xgi::Output *out,
  const SharedResourcesPtr sharedResources
)
{
  boost::mutex::scoped_lock lock(_xhtmlMakerMutex);
  XHTMLMonitor theMonitor;
  XHTMLMaker maker;

  StatisticsReporterPtr statReporter = sharedResources->_statisticsReporter;

  // Create the body with the standard header
  XHTMLMaker::Node* body = createWebPageBody(maker, statReporter);

  addDOMforThroughputStatistics(maker, body, statReporter->getThroughputMonitorCollection());  

  addDOMforSMLinks(maker, body);

  // Dump the webpage to the output stream
  maker.out(*out);
}


///////////////////////
//// Get base URL: ////
///////////////////////
std::string WebPageHelper::baseURL() const
{
  return _appDescriptor->getContextDescriptor()->getURL() + "/" + _appDescriptor->getURN();
}


XHTMLMaker::Node* WebPageHelper::createWebPageBody
(
  XHTMLMaker& maker,
  const StatisticsReporterPtr statReporter
)
{
  std::ostringstream title;
  title << _appDescriptor->getClassName()
    << " instance " << _appDescriptor->getInstance();
  XHTMLMaker::Node* body = maker.start(title.str());
  
  std::ostringstream stylesheetLink;
  stylesheetLink << "/" << _appDescriptor->getURN()
    << "/styles.css";
  XHTMLMaker::AttrMap stylesheetAttr;
  stylesheetAttr[ "rel" ] = "stylesheet";
  stylesheetAttr[ "type" ] = "text/css";
  stylesheetAttr[ "href" ] = stylesheetLink.str();
  maker.addNode("link", maker.getHead(), stylesheetAttr);
  
  XHTMLMaker::AttrMap tableAttr;
  tableAttr[ "border" ] = "0";
  tableAttr[ "cellspacing" ] = "7";
  tableAttr[ "width" ] = "100%";
  XHTMLMaker::Node* table = maker.addNode("table", body, tableAttr);
  
  XHTMLMaker::Node* tableRow = maker.addNode("tr", table, _rowAttr);
  
  XHTMLMaker::AttrMap tableDivAttr;
  tableDivAttr[ "align" ] = "left";
  tableDivAttr[ "width" ] = "64";
  XHTMLMaker::Node* tableDiv = maker.addNode("td", tableRow, tableDivAttr);

  XHTMLMaker::AttrMap smLinkAttr;
  smLinkAttr[ "href" ] = _appDescriptor->getContextDescriptor()->getURL()
    + "/" + _appDescriptor->getURN();
  XHTMLMaker::Node* smLink = maker.addNode("a", tableDiv, smLinkAttr);
  
  XHTMLMaker::AttrMap smImgAttr;
  smImgAttr[ "align" ] = "middle";
  smImgAttr[ "src" ] = "/evf/images/smicon.jpg"; // $XDAQ_DOCUMENT_ROOT is prepended to this path
  smImgAttr[ "alt" ] = "main";
  smImgAttr[ "width" ] = "64";
  smImgAttr[ "height" ] = "64";
  smImgAttr[ "border" ] = "0";
  maker.addNode("img", smLink, smImgAttr);

  tableDiv = maker.addNode("td", tableRow);
  tableAttr[ "cellspacing" ] = "1";
  XHTMLMaker::Node* instanceTable = maker.addNode("table", tableDiv, tableAttr);
  XHTMLMaker::Node* instanceTableRow = maker.addNode("tr", instanceTable, _rowAttr);
  tableDivAttr[ "width" ] = "60%";
  XHTMLMaker::Node* instanceTableDiv = maker.addNode("td", instanceTableRow, tableDivAttr);
  XHTMLMaker::AttrMap fontAttr;
  fontAttr[ "size" ] = "+2";
  XHTMLMaker::Node* header = maker.addNode("font", instanceTableDiv, fontAttr);
  header = maker.addNode("b", header);
  maker.addText(header, title.str());
  
  tableDivAttr[ "width" ] = "40%";
  instanceTableDiv = maker.addNode("td", instanceTableRow, tableDivAttr);
  header = maker.addNode("font", instanceTableDiv, fontAttr);
  header = maker.addNode("b", header);
  maker.addText(header, 
    statReporter->getStateMachineMonitorCollection().externallyVisibleState());

  instanceTableRow = maker.addNode("tr", instanceTable, _rowAttr);
  instanceTableDiv = maker.addNode("td", instanceTableRow);
  fontAttr[ "size" ] = "-3";
  XHTMLMaker::Node* version = maker.addNode("font", instanceTableDiv, fontAttr);
  maker.addText(version, _smVersion);
  instanceTableDiv = maker.addNode("td", instanceTableRow);
  fontAttr[ "size" ] = "-1";
  XHTMLMaker::Node* innerState = maker.addNode("font", instanceTableDiv, fontAttr);
  maker.addText(innerState, 
    statReporter->getStateMachineMonitorCollection().innerStateName());

  tableDivAttr[ "align" ] = "right";
  tableDivAttr[ "width" ] = "64";
  tableDiv = maker.addNode("td", tableRow, tableDivAttr);
  
  XHTMLMaker::AttrMap xdaqLinkAttr;
  xdaqLinkAttr[ "href" ] = "/urn:xdaq-application:lid=3";
  XHTMLMaker::Node* xdaqLink = maker.addNode("a", tableDiv, xdaqLinkAttr);
  
  XHTMLMaker::AttrMap xdaqImgAttr;
  xdaqImgAttr[ "align" ] = "middle";
  xdaqImgAttr[ "src" ] = "/hyperdaq/images/HyperDAQ.jpg"; // $XDAQ_DOCUMENT_ROOT is prepended to this path
  xdaqImgAttr[ "alt" ] = "HyperDAQ";
  xdaqImgAttr[ "width" ] = "64";
  xdaqImgAttr[ "height" ] = "64";
  xdaqImgAttr[ "border" ] = "0";
  maker.addNode("img", xdaqLink, xdaqImgAttr);

  maker.addNode("hr", body);
  
  return body;
}


void WebPageHelper::addDOMforSMLinks
(
  XHTMLMaker& maker,
  XHTMLMaker::Node *parent
)
{
  std::string url = _appDescriptor->getContextDescriptor()->getURL()
    + "/" + _appDescriptor->getURN();

  XHTMLMaker::AttrMap linkAttr;
  XHTMLMaker::Node *link;

  maker.addNode("hr", parent);

  linkAttr[ "href" ] = url + "/storedData";
  link = maker.addNode("a", parent, linkAttr);
  maker.addText(link, "Stored data web page");

  maker.addNode("hr", parent);

  linkAttr[ "href" ] = url + "/rbsenderlist";
  link = maker.addNode("a", parent, linkAttr);
  maker.addText(link, "RB Sender list web page");

  maker.addNode("hr", parent);

  linkAttr[ "href" ] = url + "/fileStatistics";
  link = maker.addNode("a", parent, linkAttr);
  maker.addText(link, "File Statistics web page");

  maker.addNode("hr", parent);

  linkAttr[ "href" ] = url + "/consumerStatistics";
  link = maker.addNode("a", parent, linkAttr);
  maker.addText(link, "Consumer Statistics");

  maker.addNode("hr", parent);

  linkAttr[ "href" ] = url + "/dqmEventStatistics";
  link = maker.addNode("a", parent, linkAttr);
  maker.addText(link, "DQM event processor statistics");

  maker.addNode("hr", parent);

  linkAttr[ "href" ] = url + "/throughputStatistics";
  link = maker.addNode("a", parent, linkAttr);
  maker.addText(link, "Throughput statistics");

  maker.addNode("hr", parent);

}


void WebPageHelper::addDOMforResourceUsage
(
  XHTMLMaker& maker,
  XHTMLMaker::Node *parent,
  ResourceMonitorCollection const& rmc
)
{
  ResourceMonitorCollection::Stats stats;
  rmc.getStats(stats);

  XHTMLMaker::AttrMap colspanAttr;
  colspanAttr[ "colspan" ] = "2";

  XHTMLMaker::AttrMap halfWidthAttr;
  halfWidthAttr[ "width" ] = "50%";
  
  XHTMLMaker::Node* table = maker.addNode("table", parent, _tableAttr);
  XHTMLMaker::Node* tableRow = maker.addNode("tr", table, _rowAttr);
  XHTMLMaker::Node* tableDiv = maker.addNode("th", tableRow, colspanAttr);
  maker.addText(tableDiv, "Resource Usage");
  
  tableRow = maker.addNode("tr", table, _rowAttr);

  tableDiv = maker.addNode("td", tableRow, halfWidthAttr);
  addTableForResourceUsages(maker, tableDiv, stats);

  tableDiv = maker.addNode("td", tableRow, halfWidthAttr);
  addTableForDiskUsages(maker, tableDiv, stats);
}


void WebPageHelper::addTableForResourceUsages
(
  XHTMLMaker& maker,
  XHTMLMaker::Node *parent,
  ResourceMonitorCollection::Stats const& stats
)
{
  XHTMLMaker::Node* table = maker.addNode("table", parent, _tableAttr);
  
  addRowsForMemoryUsage(maker, table, stats);
  addRowsForWorkers(maker, table, stats);
}

   
void WebPageHelper::addRowsForMemoryUsage
(
  XHTMLMaker& maker,
  XHTMLMaker::Node *table,
  ResourceMonitorCollection::Stats const& stats
)
{
  XHTMLMaker::AttrMap colspanAttr;
  colspanAttr[ "colspan" ] = "2";

  XHTMLMaker::AttrMap tableLabelAttr = _tableLabelAttr;
  tableLabelAttr[ "width" ] = "54%";

  XHTMLMaker::AttrMap tableValueAttr = _tableValueAttr;
  tableValueAttr[ "width" ] = "46%";

  // Memory pool usage
  XHTMLMaker::Node* tableRow = maker.addNode("tr", table, _rowAttr);
  XHTMLMaker::Node* tableDiv;
  if ( stats.poolUsageStats.getSampleCount() > 0 )
  {
    tableDiv = maker.addNode("td", tableRow, tableLabelAttr);
    maker.addText(tableDiv, "Memory pool used (bytes)");
    tableDiv = maker.addNode("td", tableRow, tableValueAttr);
    maker.addText(tableDiv, stats.poolUsageStats.getLastSampleValue(), 0);
  }
  else
  {
    tableDiv = maker.addNode("td", tableRow, colspanAttr);
    maker.addText(tableDiv, "Memory pool pointer not yet available");
  }
}

 
void WebPageHelper::addRowsForWorkers
(
  XHTMLMaker& maker,
  XHTMLMaker::Node *table,
  ResourceMonitorCollection::Stats const& stats
)
{
  XHTMLMaker::AttrMap tableLabelAttr = _tableLabelAttr;
  tableLabelAttr[ "width" ] = "54%";

  XHTMLMaker::AttrMap tableValueAttr = _tableValueAttr;
  tableValueAttr[ "width" ] = "46%";

  // # copy worker
  XHTMLMaker::Node* tableRow = maker.addNode("tr", table, _rowAttr);
  XHTMLMaker::Node* tableDiv = maker.addNode("td", tableRow, tableLabelAttr);
  maker.addText(tableDiv, "# CopyWorker");
  tableDiv = maker.addNode("td", tableRow, tableValueAttr);
  maker.addText(tableDiv, stats.numberOfCopyWorkersStats.getLastSampleValue(), 0);
  
  // # inject worker
  tableRow = maker.addNode("tr", table, _rowAttr);
  tableDiv = maker.addNode("td", tableRow, tableLabelAttr);
  maker.addText(tableDiv, "# InjectWorker");
  tableDiv = maker.addNode("td", tableRow, tableValueAttr);
  maker.addText(tableDiv, stats.numberOfInjectWorkersStats.getLastSampleValue(), 0);
}


void WebPageHelper::addTableForDiskUsages
(
  XHTMLMaker& maker,
  XHTMLMaker::Node *parent,
  ResourceMonitorCollection::Stats const& stats
)
{
  XHTMLMaker::AttrMap colspanAttr;
  colspanAttr[ "colspan" ] = "2";
  
  XHTMLMaker::AttrMap tableLabelAttr = _tableLabelAttr;
  tableLabelAttr[ "width" ] = "54%";

  XHTMLMaker::AttrMap tableValueAttr = _tableValueAttr;
  tableValueAttr[ "width" ] = "46%";
  
  XHTMLMaker::AttrMap warningAttr = _rowAttr;

  XHTMLMaker::Node* table = maker.addNode("table", parent, _tableAttr);
  XHTMLMaker::Node* tableRow = maker.addNode("tr", table, _rowAttr);
  XHTMLMaker::Node* tableDiv = maker.addNode("td", tableRow, colspanAttr);
  maker.addText(tableDiv, "Disk space usage");


  for (ResourceMonitorCollection::DiskUsageStatsPtrList::const_iterator
         it = stats.diskUsageStatsList.begin(),
         itEnd = stats.diskUsageStatsList.end();
       it != itEnd;
       ++it)
  {
    warningAttr[ "bgcolor" ] = (*it)->warningColor;
    tableRow = maker.addNode("tr", table, warningAttr);
    tableDiv = maker.addNode("td", tableRow, tableLabelAttr);
    maker.addText(tableDiv, (*it)->pathName);
    tableDiv = maker.addNode("td", tableRow, tableValueAttr);
    {
      std::ostringstream tmpString;
      tmpString << std::fixed << std::setprecision(0) <<
        (*it)->relDiskUsageStats.getLastSampleValue() << "% (" <<
        (*it)->absDiskUsageStats.getLastSampleValue() << " of " << 
        (*it)->diskSize << " GB)";
      maker.addText(tableDiv, tmpString.str());
    }
  }
}


void WebPageHelper::addDOMforFragmentMonitor
(
  XHTMLMaker& maker,
  XHTMLMaker::Node *parent,
  FragmentMonitorCollection const& fmc
)
{
  FragmentMonitorCollection::FragmentStats stats;
  fmc.getStats(stats);

  XHTMLMaker::AttrMap colspanAttr;
  colspanAttr[ "colspan" ] = "4";

  XHTMLMaker::Node* table = maker.addNode("table", parent, _tableAttr);

  // Received Data Statistics header
  XHTMLMaker::Node* tableRow = maker.addNode("tr", table, _rowAttr);
  XHTMLMaker::Node* tableDiv = maker.addNode("th", tableRow, colspanAttr);
  maker.addText(tableDiv, "Received I2O Frames");

  // Parameter/Value header
  tableRow = maker.addNode("tr", table, _rowAttr);
  tableDiv = maker.addNode("th", tableRow);
  maker.addText(tableDiv, "Parameter");
  tableDiv = maker.addNode("th", tableRow);
  maker.addText(tableDiv, "Total");
  tableDiv = maker.addNode("th", tableRow);
  maker.addText(tableDiv, "Events");
  tableDiv = maker.addNode("th", tableRow);
  maker.addText(tableDiv, "DQM histos");

  addFragmentStats(maker, table, stats,  MonitoredQuantity::FULL);

  addFragmentStats(maker, table, stats,  MonitoredQuantity::RECENT);
}

void WebPageHelper::addFragmentStats
(
  XHTMLMaker& maker,
  XHTMLMaker::Node *table,
  FragmentMonitorCollection::FragmentStats const& stats,
  const MonitoredQuantity::DataSetType dataSet
)
{
  // Mean performance header
  XHTMLMaker::Node* tableRow = maker.addNode("tr", table, _rowAttr);
  XHTMLMaker::Node* tableDiv = maker.addNode("th", tableRow);
  if ( dataSet == MonitoredQuantity::FULL )
    maker.addText(tableDiv, "Performance for full run");
  else
    maker.addText(tableDiv, "Recent performance for last");

  addDurationToTableHead(maker, tableRow,
    stats.allFragmentSizeStats.getDuration(dataSet));
  addDurationToTableHead(maker, tableRow,
    stats.eventFragmentSizeStats.getDuration(dataSet));
  addDurationToTableHead(maker, tableRow,
    stats.dqmEventFragmentSizeStats.getDuration(dataSet));

  addRowForFramesReceived(maker, table, stats, dataSet);
  addRowForBandwidth(maker, table, stats, dataSet);
  addRowForRate(maker, table, stats, dataSet);
  addRowForLatency(maker, table, stats, dataSet);
  if ( dataSet == MonitoredQuantity::FULL )
  {
    addRowForTotalVolume(maker, table, stats, dataSet);
  }
  else
  {
    addRowForMaxBandwidth(maker, table, stats, dataSet);
    addRowForMinBandwidth(maker, table, stats, dataSet);
  }
}


void WebPageHelper::addDurationToTableHead
(
  XHTMLMaker& maker,
  XHTMLMaker::Node *tableRow,
  const utils::duration_t duration
)
{
  XHTMLMaker::AttrMap tableValueWidth;
  tableValueWidth[ "width" ] = "23%";

  XHTMLMaker::Node* tableDiv = maker.addNode("th", tableRow, tableValueWidth);
  std::ostringstream tmpString;
  tmpString << std::fixed << std::setprecision(0) <<
      duration << " s";
  maker.addText(tableDiv, tmpString.str());
}


void WebPageHelper::addRowForFramesReceived
(
  XHTMLMaker& maker,
  XHTMLMaker::Node *table,
  FragmentMonitorCollection::FragmentStats const& stats,
  const MonitoredQuantity::DataSetType dataSet
)
{
  XHTMLMaker::Node* tableRow = maker.addNode("tr", table, _rowAttr);
  XHTMLMaker::Node* tableDiv = maker.addNode("td", tableRow);
  maker.addText(tableDiv, "Frames Received");
  tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
  maker.addText(tableDiv, stats.allFragmentSizeStats.getSampleCount(dataSet), 0);
  tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
  maker.addText(tableDiv, stats.eventFragmentSizeStats.getSampleCount(dataSet), 0);
  tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
  maker.addText(tableDiv, stats.dqmEventFragmentSizeStats.getSampleCount(dataSet), 0);
}


void WebPageHelper::addRowForBandwidth
(
  XHTMLMaker& maker,
  XHTMLMaker::Node *table,
  FragmentMonitorCollection::FragmentStats const& stats,
  const MonitoredQuantity::DataSetType dataSet
)
{
  XHTMLMaker::Node* tableRow = maker.addNode("tr", table, _rowAttr);
  XHTMLMaker::Node* tableDiv = maker.addNode("td", tableRow);
  maker.addText(tableDiv, "Bandwidth (MB/s)");
  tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
  maker.addText(tableDiv, stats.allFragmentSizeStats.getValueRate(dataSet));
  tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
  maker.addText(tableDiv, stats.eventFragmentSizeStats.getValueRate(dataSet));
  tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
  maker.addText(tableDiv, stats.dqmEventFragmentSizeStats.getValueRate(dataSet));
}


void WebPageHelper::addRowForRate
(
  XHTMLMaker& maker,
  XHTMLMaker::Node *table,
  FragmentMonitorCollection::FragmentStats const& stats,
  const MonitoredQuantity::DataSetType dataSet
)
{
  XHTMLMaker::Node* tableRow = maker.addNode("tr", table, _rowAttr);
  XHTMLMaker::Node* tableDiv = maker.addNode("td", tableRow);
  maker.addText(tableDiv, "Rate (frames/s)");
  tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
  maker.addText(tableDiv, stats.allFragmentSizeStats.getSampleRate(dataSet));
  tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
  maker.addText(tableDiv, stats.eventFragmentSizeStats.getSampleRate(dataSet));
  tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
  maker.addText(tableDiv, stats.dqmEventFragmentSizeStats.getSampleRate(dataSet));
}


void WebPageHelper::addRowForLatency
(
  XHTMLMaker& maker,
  XHTMLMaker::Node *table,
  FragmentMonitorCollection::FragmentStats const& stats,
  const MonitoredQuantity::DataSetType dataSet
)
{
  XHTMLMaker::Node* tableRow = maker.addNode("tr", table, _rowAttr);
  XHTMLMaker::Node* tableDiv = maker.addNode("td", tableRow);
  maker.addText(tableDiv, "Latency (us/frame)");
  tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
  maker.addText(tableDiv, stats.allFragmentSizeStats.getSampleLatency(dataSet));
  tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
  maker.addText(tableDiv, stats.eventFragmentSizeStats.getSampleLatency(dataSet));
  tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
  maker.addText(tableDiv, stats.dqmEventFragmentSizeStats.getSampleLatency(dataSet));
}


void WebPageHelper::addRowForTotalVolume
(
  XHTMLMaker& maker,
  XHTMLMaker::Node *table,
  FragmentMonitorCollection::FragmentStats const& stats,
  const MonitoredQuantity::DataSetType dataSet
)
{
  XHTMLMaker::Node* tableRow = maker.addNode("tr", table, _rowAttr);
  XHTMLMaker::Node* tableDiv = maker.addNode("td", tableRow);
  maker.addText(tableDiv, "Total volume received (MB)");
  tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
  maker.addText(tableDiv, stats.allFragmentSizeStats.getValueSum(dataSet), 3);
  tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
  maker.addText(tableDiv, stats.eventFragmentSizeStats.getValueSum(dataSet), 3);
  tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
  maker.addText(tableDiv, stats.dqmEventFragmentSizeStats.getValueSum(dataSet), 3);
}


void WebPageHelper::addRowForMaxBandwidth
(
  XHTMLMaker& maker,
  XHTMLMaker::Node *table,
  FragmentMonitorCollection::FragmentStats const& stats,
  const MonitoredQuantity::DataSetType dataSet
)
{
  XHTMLMaker::Node* tableRow = maker.addNode("tr", table, _rowAttr);
  XHTMLMaker::Node* tableDiv = maker.addNode("td", tableRow);
  maker.addText(tableDiv, "Maximum Bandwidth (MB/s)");
  tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
  maker.addText(tableDiv, stats.allFragmentBandwidthStats.getValueMax(dataSet));
  tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
  maker.addText(tableDiv, stats.eventFragmentBandwidthStats.getValueMax(dataSet));
  tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
  maker.addText(tableDiv, stats.dqmEventFragmentBandwidthStats.getValueMax(dataSet));
}


void WebPageHelper::addRowForMinBandwidth
(
  XHTMLMaker& maker,
  XHTMLMaker::Node *table,
  FragmentMonitorCollection::FragmentStats const& stats,
  const MonitoredQuantity::DataSetType dataSet
)
{
  XHTMLMaker::Node* tableRow = maker.addNode("tr", table, _rowAttr);
  XHTMLMaker::Node* tableDiv = maker.addNode("td", tableRow);
  maker.addText(tableDiv, "Minimum Bandwidth (MB/s)");
  tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
  maker.addText(tableDiv, stats.allFragmentBandwidthStats.getValueMin(dataSet));
  tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
  maker.addText(tableDiv, stats.eventFragmentBandwidthStats.getValueMin(dataSet));
  tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
  maker.addText(tableDiv, stats.dqmEventFragmentBandwidthStats.getValueMin(dataSet));
}


void WebPageHelper::addDOMforRunMonitor
(
  XHTMLMaker& maker,
  XHTMLMaker::Node *parent,
  RunMonitorCollection const& rmc
)
{
  MonitoredQuantity::Stats eventIDsReceivedStats;
  rmc.getEventIDsReceivedMQ().getStats(eventIDsReceivedStats);
  MonitoredQuantity::Stats errorEventIDsReceivedStats;
  rmc.getErrorEventIDsReceivedMQ().getStats(errorEventIDsReceivedStats);
  MonitoredQuantity::Stats runNumbersSeenStats;
  rmc.getRunNumbersSeenMQ().getStats(runNumbersSeenStats);
  MonitoredQuantity::Stats lumiSectionsSeenStats;
  rmc.getLumiSectionsSeenMQ().getStats(lumiSectionsSeenStats);

  XHTMLMaker::AttrMap colspanAttr;
  colspanAttr[ "colspan" ] = "4";
  
  XHTMLMaker::AttrMap tableLabelAttr = _tableLabelAttr;
  tableLabelAttr[ "width" ] = "27%";

  XHTMLMaker::AttrMap tableValueAttr = _tableValueAttr;
  tableValueAttr[ "width" ] = "23%";

  XHTMLMaker::Node* table = maker.addNode("table", parent, _tableAttr);

  XHTMLMaker::Node* tableRow = maker.addNode("tr", table, _rowAttr);
  XHTMLMaker::Node* tableDiv = maker.addNode("th", tableRow, colspanAttr);
  maker.addText(tableDiv, "Storage Manager Statistics");

  // Run number and lumi section
  tableRow = maker.addNode("tr", table, _rowAttr);
  tableDiv = maker.addNode("td", tableRow, tableLabelAttr);
  maker.addText(tableDiv, "Run number");
  tableDiv = maker.addNode("td", tableRow, tableValueAttr);
  maker.addText(tableDiv, runNumbersSeenStats.getLastSampleValue(), 0);
  tableDiv = maker.addNode("td", tableRow, tableLabelAttr);
  maker.addText(tableDiv, "Lumi section");
  tableDiv = maker.addNode("td", tableRow, tableValueAttr);
  maker.addText(tableDiv, lumiSectionsSeenStats.getLastSampleValue(), 0);

  // Total events received
  tableRow = maker.addNode("tr", table, _specialRowAttr);
  tableDiv = maker.addNode("td", tableRow, tableLabelAttr);
  maker.addText(tableDiv, "Events received (non-unique)");
  tableDiv = maker.addNode("td", tableRow, tableValueAttr);
  maker.addText(tableDiv, eventIDsReceivedStats.getSampleCount(), 0);
  tableDiv = maker.addNode("td", tableRow, tableLabelAttr);
  maker.addText(tableDiv, "Error events received");
  tableDiv = maker.addNode("td", tableRow, tableValueAttr);
  maker.addText(tableDiv, errorEventIDsReceivedStats.getSampleCount(), 0);

  // Last event IDs
  tableRow = maker.addNode("tr", table, _rowAttr);
  tableDiv = maker.addNode("td", tableRow, tableLabelAttr);
  maker.addText(tableDiv, "Last event ID");
  tableDiv = maker.addNode("td", tableRow, tableValueAttr);
  maker.addText(tableDiv, eventIDsReceivedStats.getLastSampleValue(), 0);
  tableDiv = maker.addNode("td", tableRow, tableLabelAttr);
  maker.addText(tableDiv, "Last error event ID");
  tableDiv = maker.addNode("td", tableRow, tableValueAttr);
  maker.addText(tableDiv, errorEventIDsReceivedStats.getLastSampleValue(), 0);

}


void WebPageHelper::addDOMforStoredData
(
  XHTMLMaker& maker,
  XHTMLMaker::Node *parent,
  StreamsMonitorCollection const& smc
)
{
  MonitoredQuantity::Stats allStreamsVolumeStats;
  smc.getAllStreamsVolumeMQ().getStats(allStreamsVolumeStats);

  XHTMLMaker::AttrMap tableValueWidthAttr;
  tableValueWidthAttr[ "width" ] = "11%";

  XHTMLMaker::AttrMap rowspanAttr = tableValueWidthAttr;
  rowspanAttr[ "rowspan" ] = "2";
  rowspanAttr[ "valign" ] = "top";
  
  XHTMLMaker::AttrMap colspanAttr;
  colspanAttr[ "colspan" ] = "8";

  XHTMLMaker::AttrMap bandwidthColspanAttr;
  bandwidthColspanAttr[ "colspan" ] = "4";

  XHTMLMaker::Node* table = maker.addNode("table", parent, _tableAttr);

  XHTMLMaker::Node* tableRow = maker.addNode("tr", table, _rowAttr);
  XHTMLMaker::Node* tableDiv = maker.addNode("th", tableRow, colspanAttr);
  maker.addText(tableDiv, "Stored Data Statistics");

  // Header
  tableRow = maker.addNode("tr", table, _specialRowAttr);
  tableDiv = maker.addNode("th", tableRow, rowspanAttr);
  maker.addText(tableDiv, "Stream");
  tableDiv = maker.addNode("th", tableRow, rowspanAttr);
  maker.addText(tableDiv, "Files");
  tableDiv = maker.addNode("th", tableRow, rowspanAttr);
  maker.addText(tableDiv, "Events");
  tableDiv = maker.addNode("th", tableRow, rowspanAttr);
  maker.addText(tableDiv, "Events/s");
  tableDiv = maker.addNode("th", tableRow, rowspanAttr);
  maker.addText(tableDiv, "Volume (MB)");
  tableDiv = maker.addNode("th", tableRow, bandwidthColspanAttr);
  maker.addText(tableDiv, "Bandwidth (MB/s)");

  tableRow = maker.addNode("tr", table, _specialRowAttr);
  tableDiv = maker.addNode("th", tableRow, tableValueWidthAttr);
  maker.addText(tableDiv, "average");
  tableDiv = maker.addNode("th", tableRow, tableValueWidthAttr);
  maker.addText(tableDiv, "min");
  tableDiv = maker.addNode("th", tableRow, tableValueWidthAttr);
  maker.addText(tableDiv, "max");
  
  if (smc.getStreamRecordsMQ().size() == 0)
  {
    tableRow = maker.addNode("tr", table, _rowAttr);
    tableDiv = maker.addNode("td", tableRow, colspanAttr);
    maker.addText(tableDiv, "no streams available yet");
    return;
  }
  // Mean performance
  tableRow = maker.addNode("tr", table, _rowAttr);
  tableDiv = maker.addNode("th", tableRow, colspanAttr);
  {
    std::ostringstream tmpString;
    tmpString << "Mean performance for " << std::fixed << std::setprecision(0) <<
      allStreamsVolumeStats.getDuration() << " s";
    maker.addText(tableDiv, tmpString.str());
  }
  listStreamRecordsStats(maker, table, smc, MonitoredQuantity::FULL);
  
  
  // Recent performance
  tableRow = maker.addNode("tr", table, _rowAttr);
  tableDiv = maker.addNode("th", tableRow, colspanAttr);
  {
    std::ostringstream tmpString;
    tmpString << "Recent performance for the last " << std::fixed << std::setprecision(0) <<
      allStreamsVolumeStats.getDuration(MonitoredQuantity::RECENT) << " s";
    maker.addText(tableDiv, tmpString.str());
  }
  listStreamRecordsStats(maker, table, smc, MonitoredQuantity::RECENT);
}


void WebPageHelper::addDOMforConfigString
(
  XHTMLMaker& maker,
  XHTMLMaker::Node *parent,
  DiskWritingParams const& dwParams
)
{
  XHTMLMaker::Node* table = maker.addNode("table", parent);
  XHTMLMaker::Node* tableRow = maker.addNode("tr", table, _specialRowAttr);
  XHTMLMaker::Node* tableDiv = maker.addNode("th", tableRow);
  maker.addText(tableDiv, "SM Configuration");
  tableRow = maker.addNode("tr", table, _rowAttr);
  tableDiv = maker.addNode("td", tableRow);
  XHTMLMaker::AttrMap textareaAttr;
  textareaAttr[ "rows" ] = "10";
  textareaAttr[ "cols" ] = "100";
  textareaAttr[ "scroll" ] = "yes";
  textareaAttr[ "readonly" ];
  textareaAttr[ "title" ] = "SM config";
  XHTMLMaker::Node* textarea = maker.addNode("textarea", tableDiv, textareaAttr);
  maker.addText(textarea, dwParams._streamConfiguration);
}  


void WebPageHelper::listStreamRecordsStats
(
  XHTMLMaker& maker,
  XHTMLMaker::Node *table,
  StreamsMonitorCollection const& smc,
  const MonitoredQuantity::DataSetType dataSet
)
{
  StreamsMonitorCollection::StreamRecordList const& streamRecords =
    smc.getStreamRecordsMQ();
  MonitoredQuantity::Stats allStreamsFileCountStats;
  smc.getAllStreamsFileCountMQ().getStats(allStreamsFileCountStats);
  MonitoredQuantity::Stats allStreamsVolumeStats;
  smc.getAllStreamsVolumeMQ().getStats(allStreamsVolumeStats);
  MonitoredQuantity::Stats allStreamsBandwidthStats;
  smc.getAllStreamsBandwidthMQ().getStats(allStreamsBandwidthStats);
 
  XHTMLMaker::Node *tableRow, *tableDiv;

  XHTMLMaker::AttrMap tableValueAttr = _tableValueAttr;
  tableValueAttr[ "width" ] = "11%";
 

  for (
    StreamsMonitorCollection::StreamRecordList::const_iterator 
      it = streamRecords.begin(), itEnd = streamRecords.end();
    it != itEnd;
    ++it
  ) 
  {
    MonitoredQuantity::Stats streamFileCountStats;
    (*it)->fileCount.getStats(streamFileCountStats);
    MonitoredQuantity::Stats streamVolumeStats;
    (*it)->volume.getStats(streamVolumeStats);
    MonitoredQuantity::Stats streamBandwidthStats;
    (*it)->bandwidth.getStats(streamBandwidthStats);
    
    
    tableRow = maker.addNode("tr", table, _rowAttr);
    tableDiv = maker.addNode("td", tableRow);
    maker.addText(tableDiv, (*it)->streamName);
    tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
    maker.addText(tableDiv, streamFileCountStats.getSampleCount(dataSet), 0);
    tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
    maker.addText(tableDiv, streamVolumeStats.getSampleCount(dataSet), 0);
    tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
    maker.addText(tableDiv, streamVolumeStats.getSampleRate(dataSet), 1);
    tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
    maker.addText(tableDiv, streamVolumeStats.getValueSum(dataSet), 1);
    tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
    maker.addText(tableDiv, streamBandwidthStats.getValueRate(dataSet));
    tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
    maker.addText(tableDiv, streamBandwidthStats.getValueMin(dataSet));
    tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
    maker.addText(tableDiv, streamBandwidthStats.getValueMax(dataSet));
  }
  
  tableRow = maker.addNode("tr", table, _specialRowAttr);
  tableDiv = maker.addNode("td", tableRow);
  maker.addText(tableDiv, "Total");
  tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
  maker.addText(tableDiv, allStreamsFileCountStats.getSampleCount(dataSet), 0);
  tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
  maker.addText(tableDiv, allStreamsVolumeStats.getSampleCount(dataSet), 0);
  tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
  maker.addText(tableDiv, allStreamsVolumeStats.getSampleRate(dataSet), 1);
  tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
  maker.addText(tableDiv, allStreamsVolumeStats.getValueSum(dataSet), 1);
  tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
  maker.addText(tableDiv, allStreamsBandwidthStats.getValueRate(dataSet));
  tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
  maker.addText(tableDiv, allStreamsBandwidthStats.getValueMin(dataSet));
  tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
  maker.addText(tableDiv, allStreamsBandwidthStats.getValueMax(dataSet));
 
}


void WebPageHelper::addDOMforFiles(XHTMLMaker& maker,
                                   XHTMLMaker::Node *parent,
                                   FilesMonitorCollection const& fmc)
{
  FilesMonitorCollection::FileRecordList const& fileRecords =
    fmc.getFileRecordsMQ();

  XHTMLMaker::AttrMap colspanAttr;
  colspanAttr[ "colspan" ] = "5";
  
  XHTMLMaker::AttrMap tableLabelAttr = _tableLabelAttr;
  tableLabelAttr[ "align" ] = "center";

  XHTMLMaker::AttrMap tableValueWidthAttr;
  tableValueWidthAttr[ "width" ] = "11%";

  XHTMLMaker::AttrMap tableCounterWidthAttr;
  tableCounterWidthAttr[ "width" ] = "5%";

  XHTMLMaker::Node* table = maker.addNode("table", parent, _tableAttr);

  XHTMLMaker::Node* tableRow = maker.addNode("tr", table, _rowAttr);
  XHTMLMaker::Node* tableDiv = maker.addNode("th", tableRow, colspanAttr);
  maker.addText(tableDiv, "File Statistics (most recent first)");

  // Header
  tableRow = maker.addNode("tr", table, _specialRowAttr);
  tableDiv = maker.addNode("th", tableRow, tableCounterWidthAttr);
  maker.addText(tableDiv, "#");
  tableDiv = maker.addNode("th", tableRow);
  maker.addText(tableDiv, "Pathname");
  tableDiv = maker.addNode("th", tableRow, tableValueWidthAttr);
  maker.addText(tableDiv, "Events");
  tableDiv = maker.addNode("th", tableRow, tableValueWidthAttr);
  maker.addText(tableDiv, "Size (Bytes)");
  tableDiv = maker.addNode("th", tableRow, tableValueWidthAttr);
  maker.addText(tableDiv, "Closing reason");

  // File list
  if (fileRecords.size() == 0)
  {
    tableRow = maker.addNode("tr", table, _rowAttr);
    tableDiv = maker.addNode("td", tableRow, colspanAttr);
    maker.addText(tableDiv, "no files available yet");
    return;
  }

  for (
    FilesMonitorCollection::FileRecordList::const_reverse_iterator 
      it = fileRecords.rbegin(), itEnd = fileRecords.rend();
    it != itEnd;
    ++it
  ) 
  {
    tableRow = maker.addNode("tr", table, _rowAttr);
    tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
    maker.addText(tableDiv, (*it)->entryCounter, 0);
    tableDiv = maker.addNode("td", tableRow);
    maker.addText(tableDiv, (*it)->completeFileName());
    tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
    maker.addText(tableDiv, (*it)->eventCount, 0);
    tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
    maker.addText(tableDiv, (*it)->fileSize, 0);
    tableDiv = maker.addNode("td", tableRow, tableLabelAttr);
    maker.addText(tableDiv, (*it)->closingReason());
  }
}


void WebPageHelper::addDOMforProcessedDQMEvents(XHTMLMaker& maker,
                                                XHTMLMaker::Node *parent,
                                                DQMEventMonitorCollection const& dmc)
{
  DQMEventMonitorCollection::DQMEventStats stats;
  dmc.getStats(stats);

  XHTMLMaker::AttrMap colspanAttr;
  colspanAttr[ "colspan" ] = "4";

  XHTMLMaker::Node* table = maker.addNode("table", parent, _tableAttr);

  // Received Data Statistics header
  XHTMLMaker::Node* tableRow = maker.addNode("tr", table, _rowAttr);
  XHTMLMaker::Node* tableDiv = maker.addNode("th", tableRow, colspanAttr);
  maker.addText(tableDiv, "Processed DQM events");

  // Parameter/Value header
  tableRow = maker.addNode("tr", table, _rowAttr);
  tableDiv = maker.addNode("th", tableRow);
  maker.addText(tableDiv, "Parameter");
  tableDiv = maker.addNode("th", tableRow);
  maker.addText(tableDiv, "Received");
  tableDiv = maker.addNode("th", tableRow);
  maker.addText(tableDiv, "Served to consumers");
  tableDiv = maker.addNode("th", tableRow);
  maker.addText(tableDiv, "Written to disk");

  addDQMEventStats(maker, table, stats,  MonitoredQuantity::FULL);

  addDQMEventStats(maker, table, stats,  MonitoredQuantity::RECENT);
}


void WebPageHelper::addDOMforDQMEventStatistics(XHTMLMaker& maker,
                                                XHTMLMaker::Node *parent,
                                                DQMEventMonitorCollection const& dmc)
{
  DQMEventMonitorCollection::DQMEventStats stats;
  dmc.getStats(stats);

  XHTMLMaker::AttrMap colspanAttr;
  colspanAttr[ "colspan" ] = "3";

  XHTMLMaker::Node* table = maker.addNode("table", parent, _tableAttr);

  // Received Data Statistics header
  XHTMLMaker::Node* tableRow = maker.addNode("tr", table, _rowAttr);
  XHTMLMaker::Node* tableDiv = maker.addNode("th", tableRow, colspanAttr);
  maker.addText(tableDiv, "DQM Event Statistics");

  // Parameter/Value header
  tableRow = maker.addNode("tr", table, _rowAttr);
  tableDiv = maker.addNode("th", tableRow);
  maker.addText(tableDiv, "Parameter");
  {
    tableDiv = maker.addNode("th", tableRow);
    std::ostringstream tmpString;
    tmpString << "Full run (" <<
      std::fixed << std::setprecision(0) <<
      stats.dqmEventSizeStats.getDuration(MonitoredQuantity::FULL) <<
      " s)";
    maker.addText(tableDiv, tmpString.str());
  }
  {
    tableDiv = maker.addNode("th", tableRow);
    std::ostringstream tmpString;
    tmpString << "Recent (" <<
      std::fixed << std::setprecision(0) <<
      stats.dqmEventSizeStats.getDuration(MonitoredQuantity::RECENT) <<
      " s)";
    maker.addText(tableDiv, tmpString.str());
  }


  // DQM events received 
  tableRow = maker.addNode("tr", table, _rowAttr);
  tableDiv = maker.addNode("td", tableRow, _tableLabelAttr);
  maker.addText(tableDiv, "DQM events received");
  tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
  maker.addText(tableDiv, stats.dqmEventSizeStats.getSampleCount(MonitoredQuantity::FULL));
  tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
  maker.addText(tableDiv, stats.dqmEventSizeStats.getSampleCount(MonitoredQuantity::RECENT));

  // Average updates/group
  tableRow = maker.addNode("tr", table, _rowAttr);
  tableDiv = maker.addNode("td", tableRow, _tableLabelAttr);
  maker.addText(tableDiv, "Updates/group (average)");
  tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
  maker.addText(tableDiv, stats.numberOfUpdatesStats.getValueAverage(MonitoredQuantity::FULL));
  tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
  maker.addText(tableDiv, stats.numberOfUpdatesStats.getValueAverage(MonitoredQuantity::RECENT));

  // Min updates/group
  tableRow = maker.addNode("tr", table, _rowAttr);
  tableDiv = maker.addNode("td", tableRow, _tableLabelAttr);
  maker.addText(tableDiv, "Updates/group (min)");
  tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
  maker.addText(tableDiv, stats.numberOfUpdatesStats.getValueMin(MonitoredQuantity::FULL));
  tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
  maker.addText(tableDiv, stats.numberOfUpdatesStats.getValueMin(MonitoredQuantity::RECENT));

  // Max updates/group
  tableRow = maker.addNode("tr", table, _rowAttr);
  tableDiv = maker.addNode("td", tableRow, _tableLabelAttr);
  maker.addText(tableDiv, "Updates/group (max)");
  tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
  maker.addText(tableDiv, stats.numberOfUpdatesStats.getValueMax(MonitoredQuantity::FULL));
  tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
  maker.addText(tableDiv, stats.numberOfUpdatesStats.getValueMax(MonitoredQuantity::RECENT));

  // RMS updates/group
  tableRow = maker.addNode("tr", table, _rowAttr);
  tableDiv = maker.addNode("td", tableRow, _tableLabelAttr);
  maker.addText(tableDiv, "Updates/group (RMS)");
  tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
  maker.addText(tableDiv, stats.numberOfUpdatesStats.getValueRMS(MonitoredQuantity::FULL));
  tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
  maker.addText(tableDiv, stats.numberOfUpdatesStats.getValueRMS(MonitoredQuantity::RECENT));
}


void WebPageHelper::addDOMforThroughputStatistics(XHTMLMaker& maker,
                                                  XHTMLMaker::Node *parent,
                                                  ThroughputMonitorCollection const& tmc)
{
  double busyPercentage, dataRate;

  MonitoredQuantity::Stats fqEntryCountMQ, fragSizeMQ, fpIdleMQ;
  MonitoredQuantity::Stats sqEntryCountMQ, eventSizeMQ, dwIdleMQ, diskWriteMQ;
  MonitoredQuantity::Stats dqEntryCountMQ, dqmEventSizeMQ, dqmIdleMQ;
  tmc.getFragmentQueueEntryCountMQ().getStats(fqEntryCountMQ);
  tmc.getPoppedFragmentSizeMQ().getStats(fragSizeMQ);
  tmc.getFragmentProcessorIdleMQ().getStats(fpIdleMQ);
  tmc.getStreamQueueEntryCountMQ().getStats(sqEntryCountMQ);
  tmc.getPoppedEventSizeMQ().getStats(eventSizeMQ);
  tmc.getDiskWriterIdleMQ().getStats(dwIdleMQ);
  tmc.getDiskWriteMQ().getStats(diskWriteMQ);
  tmc.getDQMEventQueueEntryCountMQ().getStats(dqEntryCountMQ);
  tmc.getPoppedDQMEventSizeMQ().getStats(dqmEventSizeMQ);
  tmc.getDQMEventProcessorIdleMQ().getStats(dqmIdleMQ);
  int binCount = tmc.getBinCount();

  XHTMLMaker::AttrMap colspanAttr;
  colspanAttr[ "colspan" ] = "16";

  XHTMLMaker::AttrMap tableLabelAttr = _tableLabelAttr;
  tableLabelAttr[ "align" ] = "center";

  XHTMLMaker::AttrMap tableAverageAttr = _tableValueAttr;
  tableAverageAttr[ "style" ] = "background-color: yellow;";

  XHTMLMaker::Node* table = maker.addNode("table", parent, _tableAttr);

  XHTMLMaker::Node* tableRow = maker.addNode("tr", table, _rowAttr);
  XHTMLMaker::Node* tableDiv = maker.addNode("th", tableRow, colspanAttr);
  maker.addText(tableDiv, "Throughput Statistics");

  // Header
  tableRow = maker.addNode("tr", table, _specialRowAttr);
  tableDiv = maker.addNode("th", tableRow);
  maker.addText(tableDiv, "Relative Time (sec)");
  tableDiv = maker.addNode("th", tableRow);
  maker.addText(tableDiv, "Bin Size (sec)");
  tableDiv = maker.addNode("th", tableRow, tableLabelAttr);
  maker.addText(tableDiv, "Instantaneous Number of Fragments in Fragment Queue");
  tableDiv = maker.addNode("th", tableRow, tableLabelAttr);
  maker.addText(tableDiv, "Number of Fragments Popped from Fragment Queue");
  tableDiv = maker.addNode("th", tableRow, tableLabelAttr);
  maker.addText(tableDiv, "Data Rate Popped from Fragment Queue (MB/sec)");
  tableDiv = maker.addNode("th", tableRow, tableLabelAttr);
  maker.addText(tableDiv, "Fragment Processor Thread Busy Percentage");
  tableDiv = maker.addNode("th", tableRow, tableLabelAttr);
  maker.addText(tableDiv, "Instantaneous Number of Events in Stream Queue");
  tableDiv = maker.addNode("th", tableRow, tableLabelAttr);
  maker.addText(tableDiv, "Number of Events Popped from Stream Queue");
  tableDiv = maker.addNode("th", tableRow, tableLabelAttr);
  maker.addText(tableDiv, "Data Rate Popped from Stream Queue (MB/sec)");
  tableDiv = maker.addNode("th", tableRow, tableLabelAttr);
  maker.addText(tableDiv, "Disk Writer Thread Busy Percentage");
  tableDiv = maker.addNode("th", tableRow, tableLabelAttr);
  maker.addText(tableDiv, "Number of Events Written to Disk");
  tableDiv = maker.addNode("th", tableRow, tableLabelAttr);
  maker.addText(tableDiv, "Data  Rate to Disk (MB/sec)");
  tableDiv = maker.addNode("th", tableRow, tableLabelAttr);
  maker.addText(tableDiv, "Instantaneous Number of DQMEvents in DQMEvent Queue");
  tableDiv = maker.addNode("th", tableRow, tableLabelAttr);
  maker.addText(tableDiv, "Number of DQMEvents Popped from DQMEvent Queue");
  tableDiv = maker.addNode("th", tableRow, tableLabelAttr);
  maker.addText(tableDiv, "Data Rate Popped from DQMEvent Queue (MB/sec)");
  tableDiv = maker.addNode("th", tableRow, tableLabelAttr);
  maker.addText(tableDiv, "DQMEvent Processor Thread Busy Percentage");

  // smooth out the idle times so that the busy times that we display
  // are not garbled by slight differences in the binning inside the
  // monitored quanitity and the reporting of the idle time.
  // NOTE that this "smoothing" does *not* reduce the normal variations
  // is idle or busy times, when events are flowing normally.  Instead,
  // it handles the cases in which the idle time is so large that it gets
  // reported in the wrong bin.
  int index = binCount - 1;
  while (index >= 0)
  {
    index = smoothIdleTimes(fpIdleMQ.recentBinnedValueSums,
                            fpIdleMQ.recentBinnedDurations,
                            index, index);
  }
  index = binCount - 1;
  while (index >= 0)
  {
    index = smoothIdleTimes(dwIdleMQ.recentBinnedValueSums,
                            dwIdleMQ.recentBinnedDurations,
                            index, index);
  }
  index = binCount - 1;
  while (index >= 0)
  {
    index = smoothIdleTimes(dqmIdleMQ.recentBinnedValueSums,
                            dqmIdleMQ.recentBinnedDurations,
                            index, index);
  }

  // calculate the sum of the data in each column so we can show averages
  double sums[15];
  for (int idx = 0; idx < 15; ++idx) {sums[idx] = 0.0;}

  // add individual rows for the bins
  double relativeTime = fqEntryCountMQ.recentDuration;
  for (int idx = (binCount - 1); idx >= 0; --idx)
  {
    tableRow = maker.addNode("tr", table, _rowAttr);

    // relative time
    relativeTime -= fqEntryCountMQ.recentBinnedDurations[idx];
    tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
    maker.addText(tableDiv, relativeTime, 2);

    // bin size
    tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
    maker.addText(tableDiv, fqEntryCountMQ.recentBinnedDurations[idx], 2);
    sums[0] += fqEntryCountMQ.recentBinnedDurations[idx];

    // number of fragments in fragment queue
    tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
    maker.addText(tableDiv, fqEntryCountMQ.recentBinnedValueSums[idx], 0);
    sums[1] += fqEntryCountMQ.recentBinnedValueSums[idx];

    // number of fragments popped from fragment queue
    tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
    maker.addText(tableDiv, fragSizeMQ.recentBinnedSampleCounts[idx], 0);
    sums[2] += fragSizeMQ.recentBinnedSampleCounts[idx];

    // data rate popped from fragment queue
    dataRate = 0.0;
    if (fragSizeMQ.recentBinnedDurations[idx] > 0.0)
    {
      dataRate = (fragSizeMQ.recentBinnedValueSums[idx] / 1048576.0) /
        fragSizeMQ.recentBinnedDurations[idx];
    }
    tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
    maker.addText(tableDiv, dataRate, 1);
    sums[3] += dataRate;

    // fragment processor thread busy percentage
    busyPercentage = 0.0;
    if (fpIdleMQ.recentBinnedSampleCounts[idx] > 0 &&
        (fpIdleMQ.recentBinnedValueSums[idx] <=
         fpIdleMQ.recentBinnedDurations[idx]))
    {
      busyPercentage = 100.0 * (1.0 - (fpIdleMQ.recentBinnedValueSums[idx] /
                                       fpIdleMQ.recentBinnedDurations[idx]));
      busyPercentage += 0.5;
    }
    tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
    maker.addText(tableDiv, busyPercentage, 0);
    sums[4] += busyPercentage;

    // number of events in stream queue
    tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
    maker.addText(tableDiv, sqEntryCountMQ.recentBinnedValueSums[idx], 0);
    sums[5] += sqEntryCountMQ.recentBinnedValueSums[idx];

    // number of events popped from stream queue
    tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
    maker.addText(tableDiv, eventSizeMQ.recentBinnedSampleCounts[idx], 0);
    sums[6] += eventSizeMQ.recentBinnedSampleCounts[idx];

    // data rate popped from stream queue
    dataRate = 0.0;
    if (eventSizeMQ.recentBinnedDurations[idx] > 0.0)
    {
      dataRate = (eventSizeMQ.recentBinnedValueSums[idx] / 1048576.0) /
        eventSizeMQ.recentBinnedDurations[idx];
    }
    tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
    maker.addText(tableDiv, dataRate, 1);
    sums[7] += dataRate;

    // disk writer thread busy percentage
    busyPercentage = 0.0;
    if (dwIdleMQ.recentBinnedSampleCounts[idx] > 0 &&
        (dwIdleMQ.recentBinnedValueSums[idx] <=
         dwIdleMQ.recentBinnedDurations[idx]))
    {
      busyPercentage = 100.0 * (1.0 - (dwIdleMQ.recentBinnedValueSums[idx] /
                                       dwIdleMQ.recentBinnedDurations[idx]));
      busyPercentage += 0.5;
    }
    tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
    maker.addText(tableDiv, busyPercentage, 0);
    sums[8] += busyPercentage;

    // number of events written to disk
    tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
    maker.addText(tableDiv, diskWriteMQ.recentBinnedSampleCounts[idx], 0);
    sums[9] += diskWriteMQ.recentBinnedSampleCounts[idx];

    // date rate written to disk
    dataRate = 0.0;
    if (diskWriteMQ.recentBinnedDurations[idx] > 0.0)
    {
      dataRate = (diskWriteMQ.recentBinnedValueSums[idx] / 1048576.0) /
        diskWriteMQ.recentBinnedDurations[idx];
    }
    tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
    maker.addText(tableDiv, dataRate, 1);
    sums[10] += dataRate;

    // number of dqm events in DQMEvent queue
    tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
    maker.addText(tableDiv, dqEntryCountMQ.recentBinnedValueSums[idx], 0);
    sums[11] += dqEntryCountMQ.recentBinnedValueSums[idx];

    // number of dqm events popped from DQMEvent queue
    tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
    maker.addText(tableDiv, dqmEventSizeMQ.recentBinnedSampleCounts[idx], 0);
    sums[12] += dqmEventSizeMQ.recentBinnedSampleCounts[idx];

    // data rate popped from DQMEvent queue
    dataRate = 0.0;
    if (dqmEventSizeMQ.recentBinnedDurations[idx] > 0.0)
    {
      dataRate = (dqmEventSizeMQ.recentBinnedValueSums[idx] / 1048576.0) /
        dqmEventSizeMQ.recentBinnedDurations[idx];
    }
    tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
    maker.addText(tableDiv, dataRate, 1);
    sums[13] += dataRate;

    // DQMEvent processor thread busy percentage
    busyPercentage = 0.0;
    if (dqmIdleMQ.recentBinnedSampleCounts[idx] > 0 &&
        (dqmIdleMQ.recentBinnedValueSums[idx] <=
         dqmIdleMQ.recentBinnedDurations[idx]))
    {
      busyPercentage = 100.0 * (1.0 - (dqmIdleMQ.recentBinnedValueSums[idx] /
                                       dqmIdleMQ.recentBinnedDurations[idx]));
      busyPercentage += 0.5;
    }
    tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
    maker.addText(tableDiv, busyPercentage, 0);
    sums[14] += busyPercentage;
  }

  // calculate the averages
  double averages[15];
  for (int idx = 0; idx < 15; ++idx)
  {
    if (binCount > 0)
    {
      averages[idx] = sums[idx] / binCount;
    }
    else
    {
      averages[idx] = 0.0;
    }
  }

  // display the averages
  {
    tableRow = maker.addNode("tr", table, _rowAttr);

    // relative time
    tableDiv = maker.addNode("td", tableRow, tableAverageAttr);
    maker.addText(tableDiv, "Avg");

    // bin size
    tableDiv = maker.addNode("td", tableRow, tableAverageAttr);
    maker.addText(tableDiv, averages[0], 2);

    // number of fragments in fragment queue
    tableDiv = maker.addNode("td", tableRow, tableAverageAttr);
    maker.addText(tableDiv, averages[1], 1);

    // number of fragments popped from fragment queue
    tableDiv = maker.addNode("td", tableRow, tableAverageAttr);
    maker.addText(tableDiv, averages[2], 1);

    // data rate popped from fragment queue
    tableDiv = maker.addNode("td", tableRow, tableAverageAttr);
    maker.addText(tableDiv, averages[3], 1);

    // fragment processor thread busy percentage
    tableDiv = maker.addNode("td", tableRow, tableAverageAttr);
    maker.addText(tableDiv, averages[4], 1);

    // number of events in stream queue
    tableDiv = maker.addNode("td", tableRow, tableAverageAttr);
    maker.addText(tableDiv, averages[5], 1);

    // number of events popped from stream queue
    tableDiv = maker.addNode("td", tableRow, tableAverageAttr);
    maker.addText(tableDiv, averages[6], 1);

    // data rate popped from stream queue
    tableDiv = maker.addNode("td", tableRow, tableAverageAttr);
    maker.addText(tableDiv, averages[7], 1);

    // disk writer thread busy percentage
    tableDiv = maker.addNode("td", tableRow, tableAverageAttr);
    maker.addText(tableDiv, averages[8], 1);

    // number of events written to disk
    tableDiv = maker.addNode("td", tableRow, tableAverageAttr);
    maker.addText(tableDiv, averages[9], 1);

    // date rate written to disk
    tableDiv = maker.addNode("td", tableRow, tableAverageAttr);
    maker.addText(tableDiv, averages[10], 1);

    // number of dqm events in DQMEvent queue
    tableDiv = maker.addNode("td", tableRow, tableAverageAttr);
    maker.addText(tableDiv, averages[11], 1);

    // number of dqm events popped from DQMEvent queue
    tableDiv = maker.addNode("td", tableRow, tableAverageAttr);
    maker.addText(tableDiv, averages[12], 1);

    // data rate popped from DQMEvent queue
    tableDiv = maker.addNode("td", tableRow, tableAverageAttr);
    maker.addText(tableDiv, averages[13], 1);

    // DQMEvent processor thread busy percentage
    tableDiv = maker.addNode("td", tableRow, tableAverageAttr);
    maker.addText(tableDiv, averages[14], 1);
  }
}


void WebPageHelper::addOutputModuleTables(XHTMLMaker& maker,
                                          XHTMLMaker::Node *parent,
                           DataSenderMonitorCollection const& dsmc)
{
  DataSenderMonitorCollection::OutputModuleResultsList resultsList =
    dsmc.getTopLevelOutputModuleResults();

  addOutputModuleSummary(maker, parent, resultsList);
  addOutputModuleStatistics(maker, parent, resultsList);
}


void WebPageHelper::addOutputModuleStatistics(XHTMLMaker& maker,
                                              XHTMLMaker::Node *parent,
                                              long long uniqueRBID,
                               DataSenderMonitorCollection const& dsmc)
{
  DataSenderMonitorCollection::OutputModuleResultsList resultsList =
    dsmc.getOutputModuleResultsForRB(uniqueRBID);

  addOutputModuleStatistics(maker, parent, resultsList);
}


void WebPageHelper::addOutputModuleStatistics(XHTMLMaker& maker,
                                              XHTMLMaker::Node *parent,
    DataSenderMonitorCollection::OutputModuleResultsList const& resultsList)
{
  XHTMLMaker::AttrMap colspanAttr;
  colspanAttr[ "colspan" ] = "7";

  XHTMLMaker::AttrMap tableLabelAttr = _tableLabelAttr;
  tableLabelAttr[ "align" ] = "center";

  XHTMLMaker::Node* table = maker.addNode("table", parent, _tableAttr);

  XHTMLMaker::Node* tableRow = maker.addNode("tr", table, _rowAttr);
  XHTMLMaker::Node* tableDiv = maker.addNode("th", tableRow, colspanAttr);
  maker.addText(tableDiv, "Received Data Statistics (by output module)");

  // Header
  tableRow = maker.addNode("tr", table, _specialRowAttr);
  tableDiv = maker.addNode("th", tableRow);
  maker.addText(tableDiv, "Output Module");
  tableDiv = maker.addNode("th", tableRow, tableLabelAttr);
  maker.addText(tableDiv, "Events");
  tableDiv = maker.addNode("th", tableRow, tableLabelAttr);
  maker.addText(tableDiv, "Size (MB)");
  tableDiv = maker.addNode("th", tableRow, tableLabelAttr);
  maker.addText(tableDiv, "Size/Evt (KB)");
  tableDiv = maker.addNode("th", tableRow, tableLabelAttr);
  maker.addText(tableDiv, "RMS (KB)");
  tableDiv = maker.addNode("th", tableRow, tableLabelAttr);
  maker.addText(tableDiv, "Min (KB)");
  tableDiv = maker.addNode("th", tableRow, tableLabelAttr);
  maker.addText(tableDiv, "Max (KB)");

  if (resultsList.size() == 0)
  {
    XHTMLMaker::AttrMap messageAttr = colspanAttr;
    messageAttr[ "align" ] = "center";

    tableRow = maker.addNode("tr", table, _rowAttr);
    tableDiv = maker.addNode("td", tableRow, messageAttr);
    maker.addText(tableDiv, "No output modules are available yet.");
    return;
  }
  else
  {
    for (unsigned int idx = 0; idx < resultsList.size(); ++idx)
    {
      std::string outputModuleLabel = resultsList[idx]->name;

      tableRow = maker.addNode("tr", table, _rowAttr);
      tableDiv = maker.addNode("td", tableRow);
      maker.addText(tableDiv, outputModuleLabel);
      tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
      maker.addText(tableDiv, resultsList[idx]->eventStats.getSampleCount(), 0);
      tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
      maker.addText(tableDiv,
                    resultsList[idx]->eventStats.getValueSum()/(double)0x100000);
      tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
      maker.addText(tableDiv,
                    resultsList[idx]->eventStats.getValueAverage()/(double)0x400);
      tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
      maker.addText(tableDiv,
                    resultsList[idx]->eventStats.getValueRMS()/(double)0x400);
      tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
      maker.addText(tableDiv,
                    resultsList[idx]->eventStats.getValueMin()/(double)0x400);
      tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
      maker.addText(tableDiv,
                    resultsList[idx]->eventStats.getValueMax()/(double)0x400);
    }
  }
}


void WebPageHelper::addOutputModuleSummary(XHTMLMaker& maker,
                                           XHTMLMaker::Node *parent,
    DataSenderMonitorCollection::OutputModuleResultsList const& resultsList)
{
  XHTMLMaker::AttrMap colspanAttr;
  colspanAttr[ "colspan" ] = "3";

  XHTMLMaker::AttrMap tableLabelAttr = _tableLabelAttr;
  tableLabelAttr[ "align" ] = "center";

  XHTMLMaker::Node* table = maker.addNode("table", parent, _tableAttr);

  XHTMLMaker::Node* tableRow = maker.addNode("tr", table, _rowAttr);
  XHTMLMaker::Node* tableDiv = maker.addNode("th", tableRow, colspanAttr);
  maker.addText(tableDiv, "Output Module Summary");

  // Header
  tableRow = maker.addNode("tr", table, _specialRowAttr);
  tableDiv = maker.addNode("th", tableRow);
  maker.addText(tableDiv, "Name");
  tableDiv = maker.addNode("th", tableRow, tableLabelAttr);
  maker.addText(tableDiv, "ID");
  tableDiv = maker.addNode("th", tableRow, tableLabelAttr);
  maker.addText(tableDiv, "Header Size (bytes)");

  if (resultsList.size() == 0)
  {
    XHTMLMaker::AttrMap messageAttr = colspanAttr;
    messageAttr[ "align" ] = "center";

    tableRow = maker.addNode("tr", table, _rowAttr);
    tableDiv = maker.addNode("td", tableRow, messageAttr);
    maker.addText(tableDiv, "No output modules are available yet.");
    return;
  }
  else
  {
    for (unsigned int idx = 0; idx < resultsList.size(); ++idx)
    {
      tableRow = maker.addNode("tr", table, _rowAttr);
      tableDiv = maker.addNode("td", tableRow);
      maker.addText(tableDiv, resultsList[idx]->name);
      tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
      maker.addText(tableDiv, resultsList[idx]->id, 0);
      tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
      maker.addText(tableDiv, resultsList[idx]->initMsgSize, 0);
    }
  }
}


void WebPageHelper::addResourceBrokerList(XHTMLMaker& maker,
                                          XHTMLMaker::Node *parent,
                           DataSenderMonitorCollection const& dsmc)
{
  DataSenderMonitorCollection::ResourceBrokerResultsList rbResultsList =
    dsmc.getAllResourceBrokerResults();
  std::sort(rbResultsList.begin(), rbResultsList.end(), compareRBResultPtrValues);

  XHTMLMaker::AttrMap colspanAttr;
  colspanAttr[ "colspan" ] = "7";

  XHTMLMaker::AttrMap tableLabelAttr = _tableLabelAttr;
  tableLabelAttr[ "align" ] = "center";

  XHTMLMaker::Node* table = maker.addNode("table", parent, _tableAttr);

  XHTMLMaker::Node* tableRow = maker.addNode("tr", table, _rowAttr);
  XHTMLMaker::Node* tableDiv = maker.addNode("th", tableRow, colspanAttr);
  maker.addText(tableDiv, "Data Sender Overview");

  // Header
  tableRow = maker.addNode("tr", table, _specialRowAttr);
  tableDiv = maker.addNode("th", tableRow);
  maker.addText(tableDiv, "Resource Broker URL");
  tableDiv = maker.addNode("th", tableRow, tableLabelAttr);
  maker.addText(tableDiv, "RB TID");
  tableDiv = maker.addNode("th", tableRow, tableLabelAttr);
  maker.addText(tableDiv, "# of FUs");
  tableDiv = maker.addNode("th", tableRow, tableLabelAttr);
  maker.addText(tableDiv, "# of INIT messages");
  tableDiv = maker.addNode("th", tableRow, tableLabelAttr);
  maker.addText(tableDiv, "# of events");
  tableDiv = maker.addNode("th", tableRow, tableLabelAttr);
  maker.addText(tableDiv, "Recent event rate (Hz)");
  tableDiv = maker.addNode("th", tableRow, tableLabelAttr);
  maker.addText(tableDiv, "Last event number received");

  if (rbResultsList.size() == 0)
  {
    XHTMLMaker::AttrMap messageAttr = colspanAttr;
    messageAttr[ "align" ] = "center";

    tableRow = maker.addNode("tr", table, _rowAttr);
    tableDiv = maker.addNode("td", tableRow, messageAttr);
    maker.addText(tableDiv, "No data senders have registered yet.");
    return;
  }
  else
  {
    for (unsigned int idx = 0; idx < rbResultsList.size(); ++idx)
    {
      tableRow = maker.addNode("tr", table, _rowAttr);

      tableDiv = maker.addNode("td", tableRow);
      XHTMLMaker::AttrMap linkAttr;
      linkAttr[ "href" ] = baseURL() + "/rbsenderdetail?id=" +
        boost::lexical_cast<std::string>(rbResultsList[idx]->uniqueRBID);
      XHTMLMaker::Node* link = maker.addNode("a", tableDiv, linkAttr);
      maker.addText(link, rbResultsList[idx]->key.hltURL);

      tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
      maker.addText(tableDiv, rbResultsList[idx]->key.hltTid, 0);
      tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
      maker.addText(tableDiv, rbResultsList[idx]->filterUnitCount, 0);
      tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
      maker.addText(tableDiv, rbResultsList[idx]->initMsgCount, 0);
      tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
      maker.addText(tableDiv, rbResultsList[idx]->eventStats.getSampleCount(), 0);
      tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
      maker.addText(tableDiv, rbResultsList[idx]->eventStats.
                    getSampleRate(MonitoredQuantity::RECENT));
      tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
      maker.addText(tableDiv, rbResultsList[idx]->lastEventNumber, 0);
    }
  }
}


void WebPageHelper::addResourceBrokerDetails(XHTMLMaker& maker,
                                             XHTMLMaker::Node *parent,
                                             long long uniqueRBID,
                              DataSenderMonitorCollection const& dsmc)
{
  DataSenderMonitorCollection::RBResultPtr rbResultPtr =
    dsmc.getOneResourceBrokerResult(uniqueRBID);

  if (rbResultPtr.get() == 0)
  {
    maker.addText(parent, "The requested resource broker page is not currently available.");
    return;
  }

  int tmpDuration;
  std::string tmpText;

  XHTMLMaker::AttrMap colspanAttr;
  colspanAttr[ "colspan" ] = "2";

  XHTMLMaker::AttrMap tableAttr = _tableAttr;
  tableAttr[ "width" ] = "";

  XHTMLMaker::AttrMap tableLabelAttr = _tableLabelAttr;
  tableLabelAttr[ "align" ] = "center";

  XHTMLMaker::Node* table = maker.addNode("table", parent, tableAttr);

  XHTMLMaker::Node* tableRow = maker.addNode("tr", table, _rowAttr);
  XHTMLMaker::Node* tableDiv = maker.addNode("th", tableRow, colspanAttr);
  maker.addText(tableDiv, "Resource Broker Details");

  // Header
  tableRow = maker.addNode("tr", table, _specialRowAttr);
  tableDiv = maker.addNode("th", tableRow, tableLabelAttr);
  maker.addText(tableDiv, "Parameter");
  tableDiv = maker.addNode("th", tableRow, tableLabelAttr);
  maker.addText(tableDiv, "Value");

  tableRow = maker.addNode("tr", table, _rowAttr);
  tableDiv = maker.addNode("td", tableRow, _tableLabelAttr);
  maker.addText(tableDiv, "URL");
  tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
  XHTMLMaker::AttrMap linkAttr;
  linkAttr[ "href" ] = rbResultPtr->key.hltURL + "/urn:xdaq-application:lid=" +
    boost::lexical_cast<std::string>(rbResultPtr->key.hltLocalId);
  XHTMLMaker::Node* link = maker.addNode("a", tableDiv, linkAttr);
  maker.addText(link, rbResultPtr->key.hltURL);

  tableRow = maker.addNode("tr", table, _rowAttr);
  tableDiv = maker.addNode("td", tableRow, _tableLabelAttr);
  maker.addText(tableDiv, "Class Name");
  tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
  maker.addText(tableDiv, rbResultPtr->key.hltClassName);

  tableRow = maker.addNode("tr", table, _rowAttr);
  tableDiv = maker.addNode("td", tableRow, _tableLabelAttr);
  maker.addText(tableDiv, "Instance");
  tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
  maker.addText(tableDiv, rbResultPtr->key.hltInstance, 0);

  tableRow = maker.addNode("tr", table, _rowAttr);
  tableDiv = maker.addNode("td", tableRow, _tableLabelAttr);
  maker.addText(tableDiv, "Local ID");
  tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
  maker.addText(tableDiv, rbResultPtr->key.hltLocalId, 0);

  tableRow = maker.addNode("tr", table, _rowAttr);
  tableDiv = maker.addNode("td", tableRow, _tableLabelAttr);
  maker.addText(tableDiv, "Tid");
  tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
  maker.addText(tableDiv, rbResultPtr->key.hltTid, 0);

  tableRow = maker.addNode("tr", table, _rowAttr);
  tableDiv = maker.addNode("td", tableRow, _tableLabelAttr);
  maker.addText(tableDiv, "INIT Message Count");
  tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
  maker.addText(tableDiv, rbResultPtr->initMsgCount, 0);

  tableRow = maker.addNode("tr", table, _rowAttr);
  tableDiv = maker.addNode("td", tableRow, _tableLabelAttr);
  maker.addText(tableDiv, "Event Count");
  tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
  maker.addText(tableDiv, rbResultPtr->eventStats.getSampleCount(), 0);

  tableRow = maker.addNode("tr", table, _rowAttr);
  tableDiv = maker.addNode("td", tableRow, _tableLabelAttr);
  maker.addText(tableDiv, "Last Event Number Received");
  tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
  maker.addText(tableDiv, rbResultPtr->lastEventNumber, 0);

  tableRow = maker.addNode("tr", table, _rowAttr);
  tableDiv = maker.addNode("td", tableRow, _tableLabelAttr);
  maker.addText(tableDiv, "Last Run Number Received");
  tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
  maker.addText(tableDiv, rbResultPtr->lastRunNumber, 0);

  tableRow = maker.addNode("tr", table, _rowAttr);
  tableDiv = maker.addNode("td", tableRow, _tableLabelAttr);
  tmpDuration = static_cast<int>(rbResultPtr->eventStats.recentDuration);
  tmpText =  "Recent (" + boost::lexical_cast<std::string>(tmpDuration) +
    " sec) Event Rate (Hz)";
  maker.addText(tableDiv, tmpText);
  tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
  maker.addText(tableDiv, rbResultPtr->eventStats.recentSampleRate);

  tableRow = maker.addNode("tr", table, _rowAttr);
  tableDiv = maker.addNode("td", tableRow, _tableLabelAttr);
  tmpDuration = static_cast<int>(rbResultPtr->eventStats.fullDuration);
  tmpText =  "Full (" + boost::lexical_cast<std::string>(tmpDuration) +
    " sec) Event Rate (Hz)";
  maker.addText(tableDiv, tmpText);
  tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
  maker.addText(tableDiv, rbResultPtr->eventStats.fullSampleRate);
}


void WebPageHelper::addFilterUnitList(XHTMLMaker& maker,
                                      XHTMLMaker::Node *parent,
                                      long long uniqueRBID,
                                      DataSenderMonitorCollection const& dsmc)
{
  DataSenderMonitorCollection::FilterUnitResultsList fuResultsList =
    dsmc.getFilterUnitResultsForRB(uniqueRBID);

  XHTMLMaker::AttrMap colspanAttr;
  colspanAttr[ "colspan" ] = "7";

  XHTMLMaker::AttrMap tableLabelAttr = _tableLabelAttr;
  tableLabelAttr[ "align" ] = "center";

  XHTMLMaker::Node* table = maker.addNode("table", parent, _tableAttr);

  XHTMLMaker::Node* tableRow = maker.addNode("tr", table, _rowAttr);
  XHTMLMaker::Node* tableDiv = maker.addNode("th", tableRow, colspanAttr);
  maker.addText(tableDiv, "Filter Units");

  // Header
  tableRow = maker.addNode("tr", table, _specialRowAttr);
  tableDiv = maker.addNode("th", tableRow);
  maker.addText(tableDiv, "Process ID");
  tableDiv = maker.addNode("th", tableRow, tableLabelAttr);
  maker.addText(tableDiv, "GUID");
  tableDiv = maker.addNode("th", tableRow, tableLabelAttr);
  maker.addText(tableDiv, "# of INIT messages");
  tableDiv = maker.addNode("th", tableRow, tableLabelAttr);
  maker.addText(tableDiv, "# of events");
  tableDiv = maker.addNode("th", tableRow, tableLabelAttr);
  maker.addText(tableDiv, "Recent event rate (Hz)");
  tableDiv = maker.addNode("th", tableRow, tableLabelAttr);
  maker.addText(tableDiv, "Last event number received");
  tableDiv = maker.addNode("th", tableRow, tableLabelAttr);
  maker.addText(tableDiv, "Last run number received");

  if (fuResultsList.size() == 0)
  {
    XHTMLMaker::AttrMap messageAttr = colspanAttr;
    messageAttr[ "align" ] = "center";

    tableRow = maker.addNode("tr", table, _rowAttr);
    tableDiv = maker.addNode("td", tableRow, messageAttr);
    maker.addText(tableDiv, "No filter units have registered yet.");
    return;
  }
  else
  {
    for (unsigned int idx = 0; idx < fuResultsList.size(); ++idx)
    {
      tableRow = maker.addNode("tr", table, _rowAttr);

      tableDiv = maker.addNode("td", tableRow);
      maker.addText(tableDiv, fuResultsList[idx]->key.fuProcessId, 0);
      tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
      maker.addText(tableDiv, fuResultsList[idx]->key.fuGuid, 0);
      tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
      maker.addText(tableDiv, fuResultsList[idx]->initMsgCount, 0);
      tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
      maker.addText(tableDiv, fuResultsList[idx]->eventStats.getSampleCount(), 0);
      tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
      maker.addText(tableDiv, fuResultsList[idx]->eventStats.
                    getSampleRate(MonitoredQuantity::RECENT));
      tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
      maker.addText(tableDiv, fuResultsList[idx]->lastEventNumber, 0);
      tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
      maker.addText(tableDiv, fuResultsList[idx]->lastRunNumber, 0);
    }
  }
}


void WebPageHelper::addDQMEventStats
(
  XHTMLMaker& maker,
  XHTMLMaker::Node *table,
  DQMEventMonitorCollection::DQMEventStats const& stats,
  const MonitoredQuantity::DataSetType dataSet
)
{
  // Mean performance header
  XHTMLMaker::Node* tableRow = maker.addNode("tr", table, _rowAttr);
  XHTMLMaker::Node* tableDiv = maker.addNode("th", tableRow);
  if ( dataSet == MonitoredQuantity::FULL )
    maker.addText(tableDiv, "Mean performance for");
  else
    maker.addText(tableDiv, "Recent performance for last");

  addDurationToTableHead(maker, tableRow,
    stats.dqmEventSizeStats.getDuration(dataSet));
  addDurationToTableHead(maker, tableRow,
    stats.servedDQMEventSizeStats.getDuration(dataSet));
  addDurationToTableHead(maker, tableRow,
    stats.writtenDQMEventSizeStats.getDuration(dataSet));

  addRowForDQMEventsProcessed(maker, table, stats, dataSet);
  addRowForDQMEventBandwidth(maker, table, stats, dataSet);
  if ( dataSet == MonitoredQuantity::FULL )
  {
    addRowForTotalDQMEventVolume(maker, table, stats, dataSet);
  }
  else
  {
    addRowForMaxDQMEventBandwidth(maker, table, stats, dataSet);
    addRowForMinDQMEventBandwidth(maker, table, stats, dataSet);
  }
}


void WebPageHelper::addRowForDQMEventsProcessed
(
  XHTMLMaker& maker,
  XHTMLMaker::Node *table,
  DQMEventMonitorCollection::DQMEventStats const& stats,
  const MonitoredQuantity::DataSetType dataSet
)
{
  XHTMLMaker::Node* tableRow = maker.addNode("tr", table, _rowAttr);
  XHTMLMaker::Node* tableDiv = maker.addNode("td", tableRow);
  maker.addText(tableDiv, "DQM groups");
  tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
  maker.addText(tableDiv, stats.numberOfGroupsStats.getValueSum(dataSet), 0);
  tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
  maker.addText(tableDiv, stats.servedDQMEventSizeStats.getSampleCount(dataSet), 0);
  tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
  maker.addText(tableDiv, stats.numberOfWrittenGroupsStats.getValueSum(dataSet), 0);
}


void WebPageHelper::addRowForDQMEventBandwidth
(
  XHTMLMaker& maker,
  XHTMLMaker::Node *table,
  DQMEventMonitorCollection::DQMEventStats const& stats,
  const MonitoredQuantity::DataSetType dataSet
)
{
  XHTMLMaker::Node* tableRow = maker.addNode("tr", table, _rowAttr);
  XHTMLMaker::Node* tableDiv = maker.addNode("td", tableRow);
  maker.addText(tableDiv, "Bandwidth (MB/s)");
  tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
  maker.addText(tableDiv, stats.dqmEventSizeStats.getValueRate(dataSet));
  tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
  maker.addText(tableDiv, stats.servedDQMEventSizeStats.getValueRate(dataSet));
  tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
  maker.addText(tableDiv, stats.writtenDQMEventSizeStats.getValueRate(dataSet));
}


void WebPageHelper::addRowForTotalDQMEventVolume
(
  XHTMLMaker& maker,
  XHTMLMaker::Node *table,
  DQMEventMonitorCollection::DQMEventStats const& stats,
  const MonitoredQuantity::DataSetType dataSet
)
{
  XHTMLMaker::Node* tableRow = maker.addNode("tr", table, _rowAttr);
  XHTMLMaker::Node* tableDiv = maker.addNode("td", tableRow);
  maker.addText(tableDiv, "Total volume processed (MB)");
  tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
  maker.addText(tableDiv, stats.dqmEventSizeStats.getValueSum(dataSet), 3);
  tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
  maker.addText(tableDiv, stats.servedDQMEventSizeStats.getValueSum(dataSet), 3);
  tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
  maker.addText(tableDiv, stats.writtenDQMEventSizeStats.getValueSum(dataSet), 3);
}


void WebPageHelper::addRowForMaxDQMEventBandwidth
(
  XHTMLMaker& maker,
  XHTMLMaker::Node *table,
  DQMEventMonitorCollection::DQMEventStats const& stats,
  const MonitoredQuantity::DataSetType dataSet
)
{
  XHTMLMaker::Node* tableRow = maker.addNode("tr", table, _rowAttr);
  XHTMLMaker::Node* tableDiv = maker.addNode("td", tableRow);
  maker.addText(tableDiv, "Maximum Bandwidth (MB/s)");
  tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
  maker.addText(tableDiv, stats.dqmEventBandwidthStats.getValueMax(dataSet));
  tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
  maker.addText(tableDiv, stats.servedDQMEventBandwidthStats.getValueMax(dataSet));
  tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
  maker.addText(tableDiv, stats.writtenDQMEventBandwidthStats.getValueMax(dataSet));
}


void WebPageHelper::addRowForMinDQMEventBandwidth
(
  XHTMLMaker& maker,
  XHTMLMaker::Node *table,
  DQMEventMonitorCollection::DQMEventStats const& stats,
  const MonitoredQuantity::DataSetType dataSet
)
{
  XHTMLMaker::Node* tableRow = maker.addNode("tr", table, _rowAttr);
  XHTMLMaker::Node* tableDiv = maker.addNode("td", tableRow);
  maker.addText(tableDiv, "Minimum Bandwidth (MB/s)");
  tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
  maker.addText(tableDiv, stats.dqmEventBandwidthStats.getValueMin(dataSet));
  tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
  maker.addText(tableDiv, stats.servedDQMEventBandwidthStats.getValueMin(dataSet));
  tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
  maker.addText(tableDiv, stats.writtenDQMEventBandwidthStats.getValueMin(dataSet));
}


int WebPageHelper::smoothIdleTimes(std::vector<double>& idleTimes,
                                   std::vector<utils::duration_t>& durations,
                                   int firstIndex, int lastIndex)
{
  int workingSize = lastIndex - firstIndex + 1;
  double idleTimeSum = 0.0;
  double durationSum = 0.0;

  for (int idx = firstIndex; idx <= lastIndex; ++idx)
  {
    idleTimeSum += idleTimes[idx];
    durationSum += durations[idx];
  }

  if (idleTimeSum > durationSum && firstIndex > 0)
  {
    return smoothIdleTimes(idleTimes, durations, firstIndex-1, lastIndex);
  }
  else
  {
    if (lastIndex > firstIndex)
    {
      for (int idx = firstIndex; idx <= lastIndex; ++idx)
      {
        idleTimes[idx] = idleTimeSum / workingSize;
        durations[idx] = durationSum / workingSize;
      }
    }
    return (firstIndex - 1);
  }
}


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
