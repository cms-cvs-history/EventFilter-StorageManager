// $Id: WebPageHelper.cc,v 1.1.2.15 2009/04/06 18:31:12 mommsen Exp $

#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <sys/statfs.h>

#include "EventFilter/StorageManager/interface/MonitoredQuantity.h"
#include "EventFilter/StorageManager/interface/WebPageHelper.h"
#include "EventFilter/StorageManager/interface/XHTMLMonitor.h"

using namespace stor;


boost::mutex WebPageHelper::_xhtmlMakerMutex;

WebPageHelper::WebPageHelper(xdaq::ApplicationDescriptor* appDesc) :
_appDescriptor(appDesc)
{
  _tableAttr[ "frame" ] = "void";
  _tableAttr[ "rules" ] = "group";
  _tableAttr[ "class" ] = "states";
  _tableAttr[ "cellspacing" ] = "0";
  _tableAttr[ "cellpadding" ] = "2";
  _tableAttr[ "width" ] = "100%";
  
  _specialRowAttr[ "class" ] = "special";

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
  const StatisticsReporterPtr statReporter
)
{
  boost::mutex::scoped_lock lock(_xhtmlMakerMutex);
  XHTMLMonitor theMonitor;
  XHTMLMaker maker;
  
  // Create the body with the standard header
  XHTMLMaker::Node* body = 
    createWebPageBody(maker, statReporter->externallyVisibleState());

  addDOMforStoredData(maker, body, statReporter->getStreamsMonitorCollection());  

  addDOMforSMLinks(maker, body);
  
   // Dump the webpage to the output stream
  maker.out(*out);
}


void WebPageHelper::filesWebPage
(
  xgi::Output *out,
  const StatisticsReporterPtr statReporter
)
{
  boost::mutex::scoped_lock lock(_xhtmlMakerMutex);
  XHTMLMonitor theMonitor;
  XHTMLMaker maker;
  
  // Create the body with the standard header
  XHTMLMaker::Node* body = 
    createWebPageBody(maker, statReporter->externallyVisibleState());

  addDOMforFiles(maker, body, statReporter->getFilesMonitorCollection());  

  addDOMforSMLinks(maker, body);
  
   // Dump the webpage to the output stream
  maker.out(*out);
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
  
  tableDivAttr[ "width" ] = "40%";
  tableDiv = maker.addNode("td", tableRow, tableDivAttr);
  XHTMLMaker::Node* header = maker.addNode("h3", tableDiv);
  maker.addText(header, title.str());
  
  tableDivAttr[ "width" ] = "30%";
  tableDiv = maker.addNode("td", tableRow, tableDivAttr);
  header = maker.addNode("h3", tableDiv);
  maker.addText(header, stateName);
  
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
  maker.addText(link, "Stored data web page (remainder of old default web page)");

  linkAttr[ "href" ] = url + "/newStoredData";
  link = maker.addNode("a", parent, linkAttr);
  maker.addText(link, "New Stored data web page");

  maker.addNode("hr", parent);

  linkAttr[ "href" ] = url + "/rbsenderlist";
  link = maker.addNode("a", parent, linkAttr);
  maker.addText(link, "RB Sender list web page");

  maker.addNode("hr", parent);

  linkAttr[ "href" ] = url + "/streameroutput";
  link = maker.addNode("a", parent, linkAttr);
  maker.addText(link, "Old Streamer Output Status web page");

  linkAttr[ "href" ] = url + "/fileStatistics";
  link = maker.addNode("a", parent, linkAttr);
  maker.addText(link, "File Statistics web page");

  maker.addNode("hr", parent);

  linkAttr[ "href" ] = url + "/EventServerStats?update=off";
  link = maker.addNode("a", parent, linkAttr);
  maker.addText(link, "Event Server Statistics");

  maker.addNode("hr", parent);

}


void WebPageHelper::addDOMforResourceUsage
(
  XHTMLMaker& maker,
  XHTMLMaker::Node *parent,
  toolbox::mem::Pool *pool,
  const DiskWritingParams dwParams
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

  XHTMLMaker::AttrMap tableLabelAttr;
  tableLabelAttr[ "align" ] = "left";
  tableLabelAttr[ "width" ] = "45%";
  
  XHTMLMaker::AttrMap tableValueAttr;
  tableValueAttr[ "align" ] = "right";
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


void WebPageHelper::
addDOMforFragmentMonitor(XHTMLMaker& maker,
                         XHTMLMaker::Node *parent,
                         FragmentMonitorCollection const& fmc)
{
  MonitoredQuantity::Stats allFragmentSizeStats;
  fmc.getAllFragmentSizeMQ().getStats(allFragmentSizeStats);
  MonitoredQuantity::Stats eventFragmentSizeStats;
  fmc.getEventFragmentSizeMQ().getStats(eventFragmentSizeStats);
  MonitoredQuantity::Stats dqmEventFragmentSizeStats;
  fmc.getDQMEventFragmentSizeMQ().getStats(dqmEventFragmentSizeStats);

  MonitoredQuantity::Stats allFragmentBandwidthStats;
  fmc.getAllFragmentBandwidthMQ().getStats(allFragmentBandwidthStats);
  MonitoredQuantity::Stats eventFragmentBandwidthStats;
  fmc.getEventFragmentBandwidthMQ().getStats(eventFragmentBandwidthStats);
  MonitoredQuantity::Stats dqmEventFragmentBandwidthStats;
  fmc.getDQMEventFragmentBandwidthMQ().getStats(dqmEventFragmentBandwidthStats);

  XHTMLMaker::AttrMap colspanAttr;
  colspanAttr[ "colspan" ] = "4";

  XHTMLMaker::AttrMap tableValueAttr;
  tableValueAttr[ "align" ] = "right";
  tableValueAttr[ "width" ] = "23%";

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


  // Mean performance header
  tableRow = maker.addNode("tr", table);
  tableDiv = maker.addNode("th", tableRow);
  maker.addText(tableDiv, "Mean performance for");
  {
    tableDiv = maker.addNode("th", tableRow);
    std::ostringstream tmpString;
    tmpString << std::fixed << std::setprecision(0) <<
      allFragmentSizeStats.getDuration(MonitoredQuantity::FULL) << " s";
    maker.addText(tableDiv, tmpString.str());
  }
  {
    tableDiv = maker.addNode("th", tableRow);
    std::ostringstream tmpString;
    tmpString << std::fixed << std::setprecision(0) <<
      eventFragmentSizeStats.getDuration(MonitoredQuantity::FULL) << " s";
    maker.addText(tableDiv, tmpString.str());
  }
  {
    tableDiv = maker.addNode("th", tableRow);
    std::ostringstream tmpString;
    tmpString << std::fixed << std::setprecision(0) <<
      dqmEventFragmentSizeStats.getDuration(MonitoredQuantity::FULL) << " s";
    maker.addText(tableDiv, tmpString.str());
  }

  // Frames received entry
  tableRow = maker.addNode("tr", table);
  tableDiv = maker.addNode("td", tableRow);
  maker.addText(tableDiv, "Frames Received");
  tableDiv = maker.addNode("td", tableRow, tableValueAttr);
  maker.addText(tableDiv, allFragmentSizeStats.getSampleCount(MonitoredQuantity::FULL), 0);
  tableDiv = maker.addNode("td", tableRow, tableValueAttr);
  maker.addText(tableDiv, eventFragmentSizeStats.getSampleCount(MonitoredQuantity::FULL), 0);
  tableDiv = maker.addNode("td", tableRow, tableValueAttr);
  maker.addText(tableDiv, dqmEventFragmentSizeStats.getSampleCount(MonitoredQuantity::FULL), 0);

  // Bandwidth
  tableRow = maker.addNode("tr", table);
  tableDiv = maker.addNode("td", tableRow);
  maker.addText(tableDiv, "Bandwidth (MB/s)");
  tableDiv = maker.addNode("td", tableRow, tableValueAttr);
  maker.addText(tableDiv, allFragmentSizeStats.getValueRate(MonitoredQuantity::FULL));
  tableDiv = maker.addNode("td", tableRow, tableValueAttr);
  maker.addText(tableDiv, eventFragmentSizeStats.getValueRate(MonitoredQuantity::FULL));
  tableDiv = maker.addNode("td", tableRow, tableValueAttr);
  maker.addText(tableDiv, dqmEventFragmentSizeStats.getValueRate(MonitoredQuantity::FULL));

  // Rate
  tableRow = maker.addNode("tr", table);
  tableDiv = maker.addNode("td", tableRow);
  maker.addText(tableDiv, "Rate (frames/s)");
  tableDiv = maker.addNode("td", tableRow, tableValueAttr);
  maker.addText(tableDiv, allFragmentSizeStats.getSampleRate(MonitoredQuantity::FULL));
  tableDiv = maker.addNode("td", tableRow, tableValueAttr);
  maker.addText(tableDiv, eventFragmentSizeStats.getSampleRate(MonitoredQuantity::FULL));
  tableDiv = maker.addNode("td", tableRow, tableValueAttr);
  maker.addText(tableDiv, dqmEventFragmentSizeStats.getSampleRate(MonitoredQuantity::FULL));

  // Latency
  tableRow = maker.addNode("tr", table);
  tableDiv = maker.addNode("td", tableRow);
  maker.addText(tableDiv, "Latency (us/frame)");
  tableDiv = maker.addNode("td", tableRow, tableValueAttr);
  maker.addText(tableDiv, allFragmentSizeStats.getSampleLatency(MonitoredQuantity::FULL));
  tableDiv = maker.addNode("td", tableRow, tableValueAttr);
  maker.addText(tableDiv, eventFragmentSizeStats.getSampleLatency(MonitoredQuantity::FULL));
  tableDiv = maker.addNode("td", tableRow, tableValueAttr);
  maker.addText(tableDiv, dqmEventFragmentSizeStats.getSampleLatency(MonitoredQuantity::FULL));

  // Total volume received
  tableRow = maker.addNode("tr", table);
  tableDiv = maker.addNode("td", tableRow);
  maker.addText(tableDiv, "Total volume received (MB)");
  tableDiv = maker.addNode("td", tableRow, tableValueAttr);
  maker.addText(tableDiv, allFragmentSizeStats.getValueSum(MonitoredQuantity::FULL), 3);
  tableDiv = maker.addNode("td", tableRow, tableValueAttr);
  maker.addText(tableDiv, eventFragmentSizeStats.getValueSum(MonitoredQuantity::FULL), 3);
  tableDiv = maker.addNode("td", tableRow, tableValueAttr);
  maker.addText(tableDiv, dqmEventFragmentSizeStats.getValueSum(MonitoredQuantity::FULL), 3);


  // Recent statistics header
  tableRow = maker.addNode("tr", table);
  tableDiv = maker.addNode("th", tableRow);
  maker.addText(tableDiv, "Recent performance for last");
  {
    tableDiv = maker.addNode("th", tableRow);
    std::ostringstream tmpString;
    tmpString << std::fixed << std::setprecision(0) <<
      allFragmentSizeStats.getDuration(MonitoredQuantity::RECENT) << " s";
    maker.addText(tableDiv, tmpString.str());
  }
  {
    tableDiv = maker.addNode("th", tableRow);
    std::ostringstream tmpString;
    tmpString << std::fixed << std::setprecision(0) <<
      eventFragmentSizeStats.getDuration(MonitoredQuantity::RECENT) << " s";
    maker.addText(tableDiv, tmpString.str());
  }
  {
    tableDiv = maker.addNode("th", tableRow);
    std::ostringstream tmpString;
    tmpString << std::fixed << std::setprecision(0) <<
      dqmEventFragmentSizeStats.getDuration(MonitoredQuantity::RECENT) << " s";
    maker.addText(tableDiv, tmpString.str());
  }

  // Frames received entry
  tableRow = maker.addNode("tr", table);
  tableDiv = maker.addNode("td", tableRow);
  maker.addText(tableDiv, "Frames Received");
  tableDiv = maker.addNode("td", tableRow, tableValueAttr);
  maker.addText(tableDiv, allFragmentSizeStats.getSampleCount(MonitoredQuantity::RECENT), 0);
  tableDiv = maker.addNode("td", tableRow, tableValueAttr);
  maker.addText(tableDiv, eventFragmentSizeStats.getSampleCount(MonitoredQuantity::RECENT), 0);
  tableDiv = maker.addNode("td", tableRow, tableValueAttr);
  maker.addText(tableDiv, dqmEventFragmentSizeStats.getSampleCount(MonitoredQuantity::RECENT), 0);

  // Bandwidth
  tableRow = maker.addNode("tr", table);
  tableDiv = maker.addNode("td", tableRow);
  maker.addText(tableDiv, "Bandwidth (MB/s)");
  tableDiv = maker.addNode("td", tableRow, tableValueAttr);
  maker.addText(tableDiv, allFragmentSizeStats.getValueRate(MonitoredQuantity::RECENT));
  tableDiv = maker.addNode("td", tableRow, tableValueAttr);
  maker.addText(tableDiv, eventFragmentSizeStats.getValueRate(MonitoredQuantity::RECENT));
  tableDiv = maker.addNode("td", tableRow, tableValueAttr);
  maker.addText(tableDiv, dqmEventFragmentSizeStats.getValueRate(MonitoredQuantity::RECENT));

  // Rate
  tableRow = maker.addNode("tr", table);
  tableDiv = maker.addNode("td", tableRow);
  maker.addText(tableDiv, "Rate (frames/s)");
  tableDiv = maker.addNode("td", tableRow, tableValueAttr);
  maker.addText(tableDiv, allFragmentSizeStats.getSampleRate(MonitoredQuantity::RECENT));
  tableDiv = maker.addNode("td", tableRow, tableValueAttr);
  maker.addText(tableDiv, eventFragmentSizeStats.getSampleRate(MonitoredQuantity::RECENT));
  tableDiv = maker.addNode("td", tableRow, tableValueAttr);
  maker.addText(tableDiv, dqmEventFragmentSizeStats.getSampleRate(MonitoredQuantity::RECENT));

  // Latency
  tableRow = maker.addNode("tr", table);
  tableDiv = maker.addNode("td", tableRow);
  maker.addText(tableDiv, "Latency (us/frame)");
  tableDiv = maker.addNode("td", tableRow, tableValueAttr);
  maker.addText(tableDiv, allFragmentSizeStats.getSampleLatency(MonitoredQuantity::RECENT));
  tableDiv = maker.addNode("td", tableRow, tableValueAttr);
  maker.addText(tableDiv, eventFragmentSizeStats.getSampleLatency(MonitoredQuantity::RECENT));
  tableDiv = maker.addNode("td", tableRow, tableValueAttr);
  maker.addText(tableDiv, dqmEventFragmentSizeStats.getSampleLatency(MonitoredQuantity::RECENT));

  // Maximum Bandwidth
  tableRow = maker.addNode("tr", table);
  tableDiv = maker.addNode("td", tableRow);
  maker.addText(tableDiv, "Maximum Bandwidth (MB/s)");
  tableDiv = maker.addNode("td", tableRow, tableValueAttr);
  maker.addText(tableDiv, allFragmentBandwidthStats.getValueMax(MonitoredQuantity::RECENT));
  tableDiv = maker.addNode("td", tableRow, tableValueAttr);
  maker.addText(tableDiv, eventFragmentBandwidthStats.getValueMax(MonitoredQuantity::RECENT));
  tableDiv = maker.addNode("td", tableRow, tableValueAttr);
  maker.addText(tableDiv, dqmEventFragmentBandwidthStats.getValueMax(MonitoredQuantity::RECENT));

  // Minimum Bandwidth
  tableRow = maker.addNode("tr", table);
  tableDiv = maker.addNode("td", tableRow);
  maker.addText(tableDiv, "Minimum Bandwidth (MB/s)");
  tableDiv = maker.addNode("td", tableRow, tableValueAttr);
  maker.addText(tableDiv, allFragmentBandwidthStats.getValueMin(MonitoredQuantity::RECENT));
  tableDiv = maker.addNode("td", tableRow, tableValueAttr);
  maker.addText(tableDiv, eventFragmentBandwidthStats.getValueMin(MonitoredQuantity::RECENT));
  tableDiv = maker.addNode("td", tableRow, tableValueAttr);
  maker.addText(tableDiv, dqmEventFragmentBandwidthStats.getValueMin(MonitoredQuantity::RECENT));

}


void WebPageHelper::addDOMforRunMonitor(XHTMLMaker& maker,
                                        XHTMLMaker::Node *parent,
                                        RunMonitorCollection const& rmc)
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
  
  XHTMLMaker::AttrMap tableLabelAttr;
  tableLabelAttr[ "align" ] = "left";
  tableLabelAttr[ "width" ] = "27%";

  XHTMLMaker::AttrMap tableValueAttr;
  tableValueAttr[ "align" ] = "right";
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


void WebPageHelper::addDOMforStoredData(XHTMLMaker& maker,
                                        XHTMLMaker::Node *parent,
                                        StreamsMonitorCollection const& smc)
{
  MonitoredQuantity::Stats allStreamsVolumeStats;
  smc.getAllStreamsVolumeMQ().getStats(allStreamsVolumeStats);

  XHTMLMaker::AttrMap rowspanAttr;
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
  tableDiv = maker.addNode("th", tableRow);
  maker.addText(tableDiv, "average");
  tableDiv = maker.addNode("th", tableRow);
  maker.addText(tableDiv, "min");
  tableDiv = maker.addNode("th", tableRow);
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

void WebPageHelper::listStreamRecordsStats
(
  XHTMLMaker& maker,
  XHTMLMaker::Node *table,
  StreamsMonitorCollection const& smc,
  MonitoredQuantity::DataSetType dataSet
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

  XHTMLMaker::AttrMap tableValueAttr;
  tableValueAttr[ "align" ] = "right";
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
    tableDiv = maker.addNode("td", tableRow, tableValueAttr);
    maker.addText(tableDiv, streamFileCountStats.getSampleCount(dataSet), 0);
    tableDiv = maker.addNode("td", tableRow, tableValueAttr);
    maker.addText(tableDiv, streamVolumeStats.getSampleCount(dataSet), 0);
    tableDiv = maker.addNode("td", tableRow, tableValueAttr);
    maker.addText(tableDiv, streamVolumeStats.getSampleRate(dataSet), 1);
    tableDiv = maker.addNode("td", tableRow, tableValueAttr);
    maker.addText(tableDiv, streamVolumeStats.getValueSum(dataSet), 1);
    tableDiv = maker.addNode("td", tableRow, tableValueAttr);
    maker.addText(tableDiv, streamBandwidthStats.getValueRate(dataSet));
    tableDiv = maker.addNode("td", tableRow, tableValueAttr);
    maker.addText(tableDiv, streamBandwidthStats.getValueMin(dataSet));
    tableDiv = maker.addNode("td", tableRow, tableValueAttr);
    maker.addText(tableDiv, streamBandwidthStats.getValueMax(dataSet));
  }
  
  tableRow = maker.addNode("tr", table, _specialRowAttr);
  tableDiv = maker.addNode("td", tableRow);
  maker.addText(tableDiv, "Total");
  tableDiv = maker.addNode("td", tableRow, tableValueAttr);
  maker.addText(tableDiv, allStreamsFileCountStats.getSampleCount(dataSet), 0);
  tableDiv = maker.addNode("td", tableRow, tableValueAttr);
  maker.addText(tableDiv, allStreamsVolumeStats.getSampleCount(dataSet), 0);
  tableDiv = maker.addNode("td", tableRow, tableValueAttr);
  maker.addText(tableDiv, allStreamsVolumeStats.getSampleRate(dataSet), 1);
  tableDiv = maker.addNode("td", tableRow, tableValueAttr);
  maker.addText(tableDiv, allStreamsVolumeStats.getValueSum(dataSet), 1);
  tableDiv = maker.addNode("td", tableRow, tableValueAttr);
  maker.addText(tableDiv, allStreamsBandwidthStats.getValueRate(dataSet));
  tableDiv = maker.addNode("td", tableRow, tableValueAttr);
  maker.addText(tableDiv, allStreamsBandwidthStats.getValueMin(dataSet));
  tableDiv = maker.addNode("td", tableRow, tableValueAttr);
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
  
  XHTMLMaker::AttrMap tableValueAttr;
  tableValueAttr[ "align" ] = "right";

  XHTMLMaker::AttrMap tableLabelAttr;
  tableLabelAttr[ "align" ] = "center";

  XHTMLMaker::Node* table = maker.addNode("table", parent, _tableAttr);

  XHTMLMaker::Node* tableRow = maker.addNode("tr", table);
  XHTMLMaker::Node* tableDiv = maker.addNode("th", tableRow, colspanAttr);
  maker.addText(tableDiv, "File Statistics (most recent first)");

  // Header
  tableRow = maker.addNode("tr", table, _specialRowAttr);
  tableDiv = maker.addNode("th", tableRow);
  maker.addText(tableDiv, "#");
  tableDiv = maker.addNode("th", tableRow);
  maker.addText(tableDiv, "Pathname");
  tableDiv = maker.addNode("th", tableRow);
  maker.addText(tableDiv, "Events");
  tableDiv = maker.addNode("th", tableRow);
  maker.addText(tableDiv, "Size (Bytes)");
  tableDiv = maker.addNode("th", tableRow);
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
    tableDiv = maker.addNode("td", tableRow, tableValueAttr);
    maker.addText(tableDiv, (*it)->entryCounter, 0);
    tableDiv = maker.addNode("td", tableRow);
    maker.addText(tableDiv, (*it)->completeFileName());
    tableDiv = maker.addNode("td", tableRow, tableValueAttr);
    maker.addText(tableDiv, (*it)->eventCount, 0);
    tableDiv = maker.addNode("td", tableRow, tableValueAttr);
    maker.addText(tableDiv, (*it)->fileSize, 0);
    tableDiv = maker.addNode("td", tableRow, tableLabelAttr);
    maker.addText(tableDiv, (*it)->closingReason());
  }
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
