// $Id: WebPageHelper.cc,v 1.1.2.27 2009/04/28 18:30:28 biery Exp $

#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <sys/statfs.h>

#include "boost/lexical_cast.hpp"

#include "EventFilter/StorageManager/interface/MonitoredQuantity.h"
#include "EventFilter/StorageManager/interface/WebPageHelper.h"
#include "EventFilter/StorageManager/interface/XHTMLMonitor.h"
#include "EventFilter/StorageManager/interface/RegistrationCollection.h"

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
  _tableAttr[ "frame" ] = "void";
  _tableAttr[ "rules" ] = "group";
  _tableAttr[ "class" ] = "states";
  _tableAttr[ "cellspacing" ] = "0";
  _tableAttr[ "cellpadding" ] = "2";
  _tableAttr[ "width" ] = "100%";
  
  _specialRowAttr[ "class" ] = "special";

  _tableLabelAttr[ "align" ] = "left";

  _tableValueAttr[ "align" ] = "right";
}


void WebPageHelper::defaultWebPage
(
  xgi::Output *out, 
  const SharedResourcesPtr sharedResources,
  toolbox::mem::Pool *pool
)
{
  boost::mutex::scoped_lock lock(_xhtmlMakerMutex);
  XHTMLMonitor theMonitor;
  XHTMLMaker maker;
  
  StatisticsReporterPtr statReporter = sharedResources->_statisticsReporter;

  // Create the body with the standard header
  XHTMLMaker::Node* body = createWebPageBody(maker,
    statReporter->externallyVisibleState());

  //TODO: Failed printout

  // Run and event summary
  addDOMforRunMonitor(maker, body, statReporter->getRunMonitorCollection());
  
  // Resource usage
  addDOMforResourceUsage(maker, body, pool, 
    sharedResources->_configuration->getDiskWritingParams());
  
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
  XHTMLMaker::Node* body = 
    createWebPageBody(maker, statReporter->externallyVisibleState());

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
  XHTMLMaker::Node* body = 
    createWebPageBody(maker, statReporter->externallyVisibleState());

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
  XHTMLMaker::Node* body =
    createWebPageBody( maker, resPtr->_statisticsReporter->externallyVisibleState() );

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
  XHTMLMaker::Node* cs_top_row = maker.addNode( "tr", cs_tbody );

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

  boost::shared_ptr<RegistrationCollection> rc = resPtr->_registrationCollection;
  RegistrationCollection::ConsumerRegistrations regs;
  rc->getEventConsumers( regs );

  // Loop over consumers:
  for( RegistrationCollection::ConsumerRegistrations::const_iterator it = regs.begin();
       it != regs.end(); ++it )
    {

      // Row:
      XHTMLMaker::Node* cs_tr = maker.addNode( "tr", cs_tbody );

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

      // Status. TODO...
      XHTMLMaker::AttrMap status_attr;
      status_attr[ "style" ] = "color:green";
      XHTMLMaker::Node* cs_td_status = maker.addNode( "td", cs_tr, status_attr );
      maker.addText( cs_td_status, "Active" );

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

    }

  // Link to the old EventServer page:
  maker.addNode( "hr", body );
  XHTMLMaker::AttrMap old_page_attr;
  old_page_attr[ "href" ] = baseURL() + "/EventServerStats?update=off";
  XHTMLMaker::Node* old_page = maker.addNode( "a", body, old_page_attr );
  maker.addText( old_page, "Old Event Server Page" );

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
  XHTMLMaker::Node* body = 
    createWebPageBody(maker, statReporter->externallyVisibleState());

  addOutputModuleStatistics(maker, body,
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
  long long localRBID
)
{
  boost::mutex::scoped_lock lock(_xhtmlMakerMutex);
  XHTMLMonitor theMonitor;
  XHTMLMaker maker;
  
  StatisticsReporterPtr statReporter = sharedResources->_statisticsReporter;
  
  // Create the body with the standard header
  XHTMLMaker::Node* body = 
    createWebPageBody(maker, statReporter->externallyVisibleState());

  addResourceBrokerDetails(maker, body, localRBID,
                           statReporter->getDataSenderMonitorCollection());  

  addOutputModuleStatistics(maker, body, localRBID,
                            statReporter->getDataSenderMonitorCollection());  

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
  const std::string stateName
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
  
  XHTMLMaker::Node* tableRow = maker.addNode("tr", table);
  
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
  XHTMLMaker::Node* instanceTableRow = maker.addNode("tr", instanceTable);
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
  maker.addText(header, stateName);

  instanceTableRow = maker.addNode("tr", instanceTable);
  instanceTableDiv = maker.addNode("td", instanceTableRow);
  fontAttr[ "size" ] = "-3";
  XHTMLMaker::Node* version = maker.addNode("font", instanceTableDiv, fontAttr);
  maker.addText(version, _smVersion);

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

  linkAttr[ "href" ] = url + "/oldDefault";
  link = maker.addNode("a", parent, linkAttr);
  maker.addText(link, "Remainder of old default web page");

  maker.addNode("hr", parent);

  linkAttr[ "href" ] = url + "/storedData";
  link = maker.addNode("a", parent, linkAttr);
  maker.addText(link, "New Stored data web page");

  maker.addNode("hr", parent);

  linkAttr[ "href" ] = url + "/rbsenderlist";
  link = maker.addNode("a", parent, linkAttr);
  maker.addText(link, "RB Sender list web page");

  maker.addNode("hr", parent);

  linkAttr[ "href" ] = url + "/oldrbsenderlist";
  link = maker.addNode("a", parent, linkAttr);
  maker.addText(link, "Old RB Sender list web page");

  maker.addNode("hr", parent);

  linkAttr[ "href" ] = url + "/fileStatistics";
  link = maker.addNode("a", parent, linkAttr);
  maker.addText(link, "File Statistics web page");

  maker.addNode("hr", parent);

  linkAttr[ "href" ] = url + "/consumerStatistics";
  link = maker.addNode("a", parent, linkAttr);
  maker.addText(link, "Consumer Statistics");

  maker.addNode("hr", parent);

}


void WebPageHelper::addDOMforResourceUsage
(
  XHTMLMaker& maker,
  XHTMLMaker::Node *parent,
  toolbox::mem::Pool *pool,
  DiskWritingParams const& dwParams
)
{
  XHTMLMaker::AttrMap colspanAttr;
  colspanAttr[ "colspan" ] = "2";

  XHTMLMaker::AttrMap halfWidthAttr;
  halfWidthAttr[ "width" ] = "50%";
  
  XHTMLMaker::AttrMap innerTableAttr;
  innerTableAttr[ "width" ] = "100%";
  innerTableAttr[ "valign" ] = "top";
  innerTableAttr[ "cellpadding" ] = "2";

  XHTMLMaker::AttrMap tableLabelAttr = _tableLabelAttr;
  tableLabelAttr[ "width" ] = "45%";
  
  XHTMLMaker::AttrMap tableValueAttr = _tableValueAttr;
  tableValueAttr[ "width" ] = "55%";
  
  XHTMLMaker::AttrMap warningAttr = tableValueAttr;
  warningAttr[ "bgcolor" ] = "#EF5A10";
  
  XHTMLMaker::Node* outerTable = maker.addNode("table", parent, _tableAttr);
  XHTMLMaker::Node* outerTableRow = maker.addNode("tr", outerTable);
  XHTMLMaker::Node* outerTableDiv = maker.addNode("th", outerTableRow, colspanAttr);
  maker.addText(outerTableDiv, "Resource Usage");
  
  outerTableRow = maker.addNode("tr", outerTable);
  outerTableDiv = maker.addNode("td", outerTableRow, halfWidthAttr);
  XHTMLMaker::Node* table = maker.addNode("table", outerTableDiv, innerTableAttr);
  
  // Memory pool usage
  XHTMLMaker::Node* tableRow = maker.addNode("tr", table);
  XHTMLMaker::Node* tableDiv;
  if (pool)
  {
    tableDiv = maker.addNode("td", tableRow, tableLabelAttr);
    maker.addText(tableDiv, "Memory pool used (bytes)");
    tableDiv = maker.addNode("td", tableRow, tableValueAttr);
    maker.addText(tableDiv, pool->getMemoryUsage().getUsed(), 0);
  }
  else
  {
    tableDiv = maker.addNode("td", tableRow, colspanAttr);
    maker.addText(tableDiv, "Memory pool pointer not yet available");
  }
  
  
  // Disk usage
  int nLogicalDisk = dwParams._nLogicalDisk;
  unsigned int nD = nLogicalDisk ? nLogicalDisk : 1;
  for(unsigned int i=0;i<nD;++i) {
    std::string path(dwParams._filePath);
    if(nLogicalDisk>0) {
      std::ostringstream oss;
      oss << "/" << std::setfill('0') << std::setw(2) << i; 
      path += oss.str();
    }
    struct statfs64 buf;
    int retVal = statfs64(path.c_str(), &buf);
    double btotal = 0;
    double bfree = 0;
    unsigned int used = 0;
    if(retVal==0) {
      unsigned int blksize = buf.f_bsize;
      btotal = buf.f_blocks * blksize / 1024 / 1024 /1024;
      bfree  = buf.f_bavail  * blksize / 1024 / 1024 /1024;
      used   = (int)(100 * (1. - bfree / btotal)); 
    }
    
    tableRow = maker.addNode("tr", table);
    tableDiv = maker.addNode("td", tableRow, tableLabelAttr);
    {
      std::ostringstream tmpString;
      tmpString << "Disk " << i << " usage";
      maker.addText(tableDiv, tmpString.str());
    }
    if(used>89)
      tableDiv = maker.addNode("td", tableRow, warningAttr);
    else
      tableDiv = maker.addNode("td", tableRow, tableValueAttr);
    {
      std::ostringstream tmpString;
      tmpString << used << "% (" << btotal-bfree << " of " << btotal << " GB)";
      maker.addText(tableDiv, tmpString.str());
    }
  }
  
  outerTableDiv = maker.addNode("td", outerTableRow, halfWidthAttr);
  table = maker.addNode("table", outerTableDiv, innerTableAttr);
  
  // # copy worker
  tableRow = maker.addNode("tr", table);
  tableDiv = maker.addNode("td", tableRow, tableLabelAttr);
  maker.addText(tableDiv, "# CopyWorker");
  tableDiv = maker.addNode("td", tableRow, tableValueAttr);
  maker.addText(tableDiv, getProcessCount("CopyWorker.pl"), 0);
  
  // # inject worker
  tableRow = maker.addNode("tr", table);
  tableDiv = maker.addNode("td", tableRow, tableLabelAttr);
  maker.addText(tableDiv, "# InjectWorker");
  tableDiv = maker.addNode("td", tableRow, tableValueAttr);
  maker.addText(tableDiv, getProcessCount("InjectWorker.pl"), 0);
  
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
  XHTMLMaker::Node* tableRow = maker.addNode("tr", table);
  XHTMLMaker::Node* tableDiv = maker.addNode("th", tableRow, colspanAttr);
  maker.addText(tableDiv, "Received I2O Frames");

  // Parameter/Value header
  tableRow = maker.addNode("tr", table);
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
  XHTMLMaker::Node* tableRow = maker.addNode("tr", table);
  XHTMLMaker::Node* tableDiv = maker.addNode("th", tableRow);
  if ( dataSet == MonitoredQuantity::FULL )
    maker.addText(tableDiv, "Mean performance for");
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
  XHTMLMaker::Node* tableRow = maker.addNode("tr", table);
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
  XHTMLMaker::Node* tableRow = maker.addNode("tr", table);
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
  XHTMLMaker::Node* tableRow = maker.addNode("tr", table);
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
  XHTMLMaker::Node* tableRow = maker.addNode("tr", table);
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
  XHTMLMaker::Node* tableRow = maker.addNode("tr", table);
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
  XHTMLMaker::Node* tableRow = maker.addNode("tr", table);
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
  XHTMLMaker::Node* tableRow = maker.addNode("tr", table);
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

  XHTMLMaker::Node* tableRow = maker.addNode("tr", table);
  XHTMLMaker::Node* tableDiv = maker.addNode("th", tableRow, colspanAttr);
  maker.addText(tableDiv, "Storage Manager Statistics");

  // Run number and lumi section
  tableRow = maker.addNode("tr", table);
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
  tableRow = maker.addNode("tr", table);
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

  XHTMLMaker::Node* tableRow = maker.addNode("tr", table);
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
    tableRow = maker.addNode("tr", table);
    tableDiv = maker.addNode("td", tableRow, colspanAttr);
    maker.addText(tableDiv, "no streams available yet");
    return;
  }
  // Mean performance
  tableRow = maker.addNode("tr", table);
  tableDiv = maker.addNode("th", tableRow, colspanAttr);
  {
    std::ostringstream tmpString;
    tmpString << "Mean performance for " << std::fixed << std::setprecision(0) <<
      allStreamsVolumeStats.getDuration() << " s";
    maker.addText(tableDiv, tmpString.str());
  }
  listStreamRecordsStats(maker, table, smc, MonitoredQuantity::FULL);
  
  
  // Recent performance
  tableRow = maker.addNode("tr", table);
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
  tableRow = maker.addNode("tr", table);
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
    
    
    tableRow = maker.addNode("tr", table);
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

  XHTMLMaker::Node* tableRow = maker.addNode("tr", table);
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
    tableRow = maker.addNode("tr", table);
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
    tableRow = maker.addNode("tr", table);
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


void WebPageHelper::addOutputModuleStatistics(XHTMLMaker& maker,
                                              XHTMLMaker::Node *parent,
                               DataSenderMonitorCollection const& dsmc)
{
  DataSenderMonitorCollection::OutputModuleResultsList resultsList =
    dsmc.getTopLevelOutputModuleResults();

  addOutputModuleStatistics(maker, parent, resultsList);
}


void WebPageHelper::addOutputModuleStatistics(XHTMLMaker& maker,
                                              XHTMLMaker::Node *parent,
                                              long long localRBID,
                               DataSenderMonitorCollection const& dsmc)
{
  DataSenderMonitorCollection::OutputModuleResultsList resultsList =
    dsmc.getOutputModuleResultsForRB(localRBID);

  addOutputModuleStatistics(maker, parent, resultsList);
}


void WebPageHelper::addOutputModuleStatistics(XHTMLMaker& maker,
                                              XHTMLMaker::Node *parent,
    DataSenderMonitorCollection::OutputModuleResultsList const& resultsList)
{
  XHTMLMaker::AttrMap colspanAttr;
  colspanAttr[ "colspan" ] = "8";

  XHTMLMaker::AttrMap tableLabelAttr = _tableLabelAttr;
  tableLabelAttr[ "align" ] = "center";

  XHTMLMaker::Node* table = maker.addNode("table", parent, _tableAttr);

  XHTMLMaker::Node* tableRow = maker.addNode("tr", table);
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
  tableDiv = maker.addNode("th", tableRow, tableLabelAttr);
  maker.addText(tableDiv, "Header Size (bytes)");

  if (resultsList.size() == 0)
  {
    XHTMLMaker::AttrMap messageAttr = colspanAttr;
    messageAttr[ "align" ] = "center";

    tableRow = maker.addNode("tr", table);
    tableDiv = maker.addNode("td", tableRow, messageAttr);
    maker.addText(tableDiv, "No output modules are available yet,");
    return;
  }
  else
  {
    for (unsigned int idx = 0; idx < resultsList.size(); ++idx)
    {
      std::string outputModuleLabel = resultsList[idx]->name;

      tableRow = maker.addNode("tr", table);
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

  XHTMLMaker::AttrMap colspanAttr;
  colspanAttr[ "colspan" ] = "7";

  XHTMLMaker::AttrMap tableLabelAttr = _tableLabelAttr;
  tableLabelAttr[ "align" ] = "center";

  XHTMLMaker::Node* table = maker.addNode("table", parent, _tableAttr);

  XHTMLMaker::Node* tableRow = maker.addNode("tr", table);
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

    tableRow = maker.addNode("tr", table);
    tableDiv = maker.addNode("td", tableRow, messageAttr);
    maker.addText(tableDiv, "No data senders have registered yet.");
    return;
  }
  else
  {
    for (unsigned int idx = 0; idx < rbResultsList.size(); ++idx)
    {
      tableRow = maker.addNode("tr", table);

      tableDiv = maker.addNode("td", tableRow);
      XHTMLMaker::AttrMap linkAttr;
      linkAttr[ "href" ] = baseURL() + "/rbsenderdetail?id=" +
        boost::lexical_cast<std::string>(rbResultsList[idx]->localRBID);
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
                                             long long localRBID,
                              DataSenderMonitorCollection const& dsmc)
{
  DataSenderMonitorCollection::RBResultPtr rbResultPtr =
    dsmc.getOneResourceBrokerResult(localRBID);

  if (rbResultPtr.get() == 0)
  {
    maker.addText(parent, "The requested resource broker page is not longer available.");
    maker.addText(parent, "Please reload the main resource broker list and re-select the resource broker of interest.");
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

  XHTMLMaker::Node* tableRow = maker.addNode("tr", table);
  XHTMLMaker::Node* tableDiv = maker.addNode("th", tableRow, colspanAttr);
  maker.addText(tableDiv, "Resource Broker Details");

  // Header
  tableRow = maker.addNode("tr", table, _specialRowAttr);
  tableDiv = maker.addNode("th", tableRow, tableLabelAttr);
  maker.addText(tableDiv, "Parameter");
  tableDiv = maker.addNode("th", tableRow, tableLabelAttr);
  maker.addText(tableDiv, "Value");

  tableRow = maker.addNode("tr", table);
  tableDiv = maker.addNode("td", tableRow, _tableLabelAttr);
  maker.addText(tableDiv, "URL");
  tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
  maker.addText(tableDiv, rbResultPtr->key.hltURL);

  tableRow = maker.addNode("tr", table);
  tableDiv = maker.addNode("td", tableRow, _tableLabelAttr);
  maker.addText(tableDiv, "Class Name");
  tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
  maker.addText(tableDiv, rbResultPtr->key.hltClassName);

  tableRow = maker.addNode("tr", table);
  tableDiv = maker.addNode("td", tableRow, _tableLabelAttr);
  maker.addText(tableDiv, "Instance");
  tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
  maker.addText(tableDiv, rbResultPtr->key.hltInstance, 0);

  tableRow = maker.addNode("tr", table);
  tableDiv = maker.addNode("td", tableRow, _tableLabelAttr);
  maker.addText(tableDiv, "Local ID");
  tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
  maker.addText(tableDiv, rbResultPtr->key.hltLocalId, 0);

  tableRow = maker.addNode("tr", table);
  tableDiv = maker.addNode("td", tableRow, _tableLabelAttr);
  maker.addText(tableDiv, "Tid");
  tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
  maker.addText(tableDiv, rbResultPtr->key.hltTid, 0);

  tableRow = maker.addNode("tr", table);
  tableDiv = maker.addNode("td", tableRow, _tableLabelAttr);
  maker.addText(tableDiv, "INIT Message Count");
  tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
  maker.addText(tableDiv, rbResultPtr->initMsgCount, 0);

  tableRow = maker.addNode("tr", table);
  tableDiv = maker.addNode("td", tableRow, _tableLabelAttr);
  maker.addText(tableDiv, "Event Count");
  tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
  maker.addText(tableDiv, rbResultPtr->eventStats.getSampleCount(), 0);

  tableRow = maker.addNode("tr", table);
  tableDiv = maker.addNode("td", tableRow, _tableLabelAttr);
  maker.addText(tableDiv, "Last Event Number Received");
  tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
  maker.addText(tableDiv, rbResultPtr->lastEventNumber, 0);

  tableRow = maker.addNode("tr", table);
  tableDiv = maker.addNode("td", tableRow, _tableLabelAttr);
  tmpDuration = static_cast<int>(rbResultPtr->eventStats.recentDuration);
  tmpText =  "Recent (" + boost::lexical_cast<std::string>(tmpDuration) +
    " sec) Event Rate (Hz)";
  maker.addText(tableDiv, tmpText);
  tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
  maker.addText(tableDiv, rbResultPtr->eventStats.recentSampleRate);

  tableRow = maker.addNode("tr", table);
  tableDiv = maker.addNode("td", tableRow, _tableLabelAttr);
  tmpDuration = static_cast<int>(rbResultPtr->eventStats.fullDuration);
  tmpText =  "Full (" + boost::lexical_cast<std::string>(tmpDuration) +
    " sec) Event Rate (Hz)";
  maker.addText(tableDiv, tmpText);
  tableDiv = maker.addNode("td", tableRow, _tableValueAttr);
  maker.addText(tableDiv, rbResultPtr->eventStats.fullSampleRate);
}


int WebPageHelper::getProcessCount(std::string processName)
{

  int count = -1;
  char buf[128];
  std::string command = "ps -C " + processName + " --no-header | wc -l";

  FILE *fp = popen(command.c_str(), "r");
  if ( fp )
  {
    if ( fgets(buf, sizeof(buf), fp) )
    {
      count = strtol(buf, '\0', 10);
    }
    pclose( fp );
  }
  return count;
}


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
