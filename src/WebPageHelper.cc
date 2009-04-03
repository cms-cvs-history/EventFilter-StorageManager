// $Id: WebPageHelper.cc,v 1.1.2.8 2009/03/02 18:08:22 biery Exp $

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


void WebPageHelper::defaultWebPage
(
  xgi::Output *out, 
  const SharedResourcesPtr sharedResources,
  xdaq::ApplicationDescriptor* appDesc,
  toolbox::mem::Pool *pool
)
{
  boost::mutex::scoped_lock lock(_xhtmlMakerMutex);
  XHTMLMonitor theMonitor;
  XHTMLMaker maker;
  
  StatisticsReporterPtr statReporter = sharedResources->_statisticsReporter;

  // Create the body with the standard header
  XHTMLMaker::Node* body = createWebPageBody(maker,
    statReporter->externallyVisibleState(), appDesc);

  //TODO: Failed printout

  // Run and event summary
  addDOMforRunMonitor(maker, body, statReporter->getRunMonitorCollection());
  
  // Resource usage
  addDOMforResourceUsage(maker, body, pool, 
    sharedResources->_configuration->getDiskWritingParams());
  
  // Add the received data statistics table
  addDOMforFragmentMonitor(maker, body,
                           statReporter->getFragmentMonitorCollection());

  addDOMforSMLinks(maker, body, appDesc);
  
  // Dump the webpage to the output stream
  maker.out(*out);
}


void WebPageHelper::filesWebPage
(
  xgi::Output *out,
  const StatisticsReporterPtr statReporter,
  xdaq::ApplicationDescriptor* appDesc
)
{
  boost::mutex::scoped_lock lock(_xhtmlMakerMutex);
  XHTMLMonitor theMonitor;
  XHTMLMaker maker;
  
  // Create the body with the standard header
  XHTMLMaker::Node* body = 
    createWebPageBody(maker, statReporter->externallyVisibleState(), appDesc);


  addDOMforSMLinks(maker, body, appDesc);
  
   // Dump the webpage to the output stream
  maker.out(*out);
}


XHTMLMaker::Node* WebPageHelper::createWebPageBody
(
  XHTMLMaker& maker,
  const std::string stateName,
  xdaq::ApplicationDescriptor* appDescriptor
)
{
  std::ostringstream title;
  title << appDescriptor->getClassName()
    << " instance " << appDescriptor->getInstance();
  XHTMLMaker::Node* body = maker.start(title.str());
  
  std::ostringstream stylesheetLink;
  stylesheetLink << "/" << appDescriptor->getURN()
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
  smLinkAttr[ "href" ] = appDescriptor->getContextDescriptor()->getURL()
    + "/" + appDescriptor->getURN();
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
  XHTMLMaker::Node *parent,
  xdaq::ApplicationDescriptor* appDescriptor
)
{
  std::string url = appDescriptor->getContextDescriptor()->getURL()
    + "/" + appDescriptor->getURN();

  XHTMLMaker::AttrMap linkAttr;
  XHTMLMaker::Node *link;

  maker.addNode("hr", parent);

  linkAttr[ "href" ] = url + "/storedData";
  link = maker.addNode("a", parent, linkAttr);
  maker.addText(link, "Stored data web page (remainder of old default web page)");

  maker.addNode("hr", parent);

  linkAttr[ "href" ] = url + "/rbsenderlist";
  link = maker.addNode("a", parent, linkAttr);
  maker.addText(link, "RB Sender list web page");

  maker.addNode("hr", parent);

  linkAttr[ "href" ] = url + "/streameroutput";
  link = maker.addNode("a", parent, linkAttr);
  maker.addText(link, "Streamer Output Status web page");

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
  XHTMLMaker::AttrMap tableAttr;
  tableAttr[ "frame" ] = "void";
  tableAttr[ "rules" ] = "group";
  tableAttr[ "class" ] = "states";
  tableAttr[ "cellspacing" ] = "0";
  tableAttr[ "cellpadding" ] = "0";
  tableAttr[ "width" ] = "100%";
  
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
  
  XHTMLMaker::Node* outerTable = maker.addNode("table", parent, tableAttr);
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
  MonitoredQuantity const& allFragmentSizes =
    fmc.getAllFragmentSizeMQ();
  MonitoredQuantity const& eventFragmentSizes =
    fmc.getEventFragmentSizeMQ();
  MonitoredQuantity const& dqmEventFragmentSizes =
    fmc.getDQMEventFragmentSizeMQ();

  MonitoredQuantity const& allFragmentBandwidth =
    fmc.getAllFragmentBandwidthMQ();
  MonitoredQuantity const& eventFragmentBandwidth =
    fmc.getEventFragmentBandwidthMQ();
  MonitoredQuantity const& dqmEventFragmentBandwidth =
    fmc.getDQMEventFragmentBandwidthMQ();

  XHTMLMaker::AttrMap tableAttr;
  tableAttr[ "frame" ] = "void";
  tableAttr[ "rules" ] = "group";
  tableAttr[ "class" ] = "states";
  tableAttr[ "cellpadding" ] = "2";
  tableAttr[ "width" ] = "100%";

  XHTMLMaker::AttrMap colspanAttr;
  colspanAttr[ "colspan" ] = "4";

  XHTMLMaker::AttrMap tableValueAttr;
  tableValueAttr[ "align" ] = "right";
  tableValueAttr[ "width" ] = "23%";

  XHTMLMaker::Node* table = maker.addNode("table", parent, tableAttr);

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
    tmpString << std::fixed << std::setprecision(1) <<
      allFragmentSizes.getDuration(MonitoredQuantity::FULL) << " s";
    maker.addText(tableDiv, tmpString.str());
  }
  {
    tableDiv = maker.addNode("th", tableRow);
    std::ostringstream tmpString;
    tmpString << std::fixed << std::setprecision(1) <<
      eventFragmentSizes.getDuration(MonitoredQuantity::FULL) << " s";
    maker.addText(tableDiv, tmpString.str());
  }
  {
    tableDiv = maker.addNode("th", tableRow);
    std::ostringstream tmpString;
    tmpString << std::fixed << std::setprecision(1) <<
      dqmEventFragmentSizes.getDuration(MonitoredQuantity::FULL) << " s";
    maker.addText(tableDiv, tmpString.str());
  }

  // Frames received entry
  tableRow = maker.addNode("tr", table);
  tableDiv = maker.addNode("td", tableRow);
  maker.addText(tableDiv, "Frames Received");
  tableDiv = maker.addNode("td", tableRow, tableValueAttr);
  maker.addText(tableDiv, allFragmentSizes.getSampleCount(MonitoredQuantity::FULL), 0);
  tableDiv = maker.addNode("td", tableRow, tableValueAttr);
  maker.addText(tableDiv, eventFragmentSizes.getSampleCount(MonitoredQuantity::FULL), 0);
  tableDiv = maker.addNode("td", tableRow, tableValueAttr);
  maker.addText(tableDiv, dqmEventFragmentSizes.getSampleCount(MonitoredQuantity::FULL), 0);

  // Bandwidth
  tableRow = maker.addNode("tr", table);
  tableDiv = maker.addNode("td", tableRow);
  maker.addText(tableDiv, "Bandwidth (MB/s)");
  tableDiv = maker.addNode("td", tableRow, tableValueAttr);
  maker.addText(tableDiv, allFragmentSizes.getValueRate(MonitoredQuantity::FULL));
  tableDiv = maker.addNode("td", tableRow, tableValueAttr);
  maker.addText(tableDiv, eventFragmentSizes.getValueRate(MonitoredQuantity::FULL));
  tableDiv = maker.addNode("td", tableRow, tableValueAttr);
  maker.addText(tableDiv, dqmEventFragmentSizes.getValueRate(MonitoredQuantity::FULL));

  // Rate
  tableRow = maker.addNode("tr", table);
  tableDiv = maker.addNode("td", tableRow);
  maker.addText(tableDiv, "Rate (frames/s)");
  tableDiv = maker.addNode("td", tableRow, tableValueAttr);
  maker.addText(tableDiv, allFragmentSizes.getSampleRate(MonitoredQuantity::FULL));
  tableDiv = maker.addNode("td", tableRow, tableValueAttr);
  maker.addText(tableDiv, eventFragmentSizes.getSampleRate(MonitoredQuantity::FULL));
  tableDiv = maker.addNode("td", tableRow, tableValueAttr);
  maker.addText(tableDiv, dqmEventFragmentSizes.getSampleRate(MonitoredQuantity::FULL));

  // Latency
  tableRow = maker.addNode("tr", table);
  tableDiv = maker.addNode("td", tableRow);
  maker.addText(tableDiv, "Latency (us/frame)");
  tableDiv = maker.addNode("td", tableRow, tableValueAttr);
  maker.addText(tableDiv, allFragmentSizes.getSampleLatency(MonitoredQuantity::FULL));
  tableDiv = maker.addNode("td", tableRow, tableValueAttr);
  maker.addText(tableDiv, eventFragmentSizes.getSampleLatency(MonitoredQuantity::FULL));
  tableDiv = maker.addNode("td", tableRow, tableValueAttr);
  maker.addText(tableDiv, dqmEventFragmentSizes.getSampleLatency(MonitoredQuantity::FULL));

  // Total volume received
  tableRow = maker.addNode("tr", table);
  tableDiv = maker.addNode("td", tableRow);
  maker.addText(tableDiv, "Total volume received (MB)");
  tableDiv = maker.addNode("td", tableRow, tableValueAttr);
  maker.addText(tableDiv, allFragmentSizes.getValueSum(MonitoredQuantity::FULL), 3);
  tableDiv = maker.addNode("td", tableRow, tableValueAttr);
  maker.addText(tableDiv, eventFragmentSizes.getValueSum(MonitoredQuantity::FULL), 3);
  tableDiv = maker.addNode("td", tableRow, tableValueAttr);
  maker.addText(tableDiv, dqmEventFragmentSizes.getValueSum(MonitoredQuantity::FULL), 3);


  // Recent statistics header
  tableRow = maker.addNode("tr", table);
  tableDiv = maker.addNode("th", tableRow);
  maker.addText(tableDiv, "Recent performance for last");
  {
    tableDiv = maker.addNode("th", tableRow);
    std::ostringstream tmpString;
    tmpString << std::fixed << std::setprecision(1) <<
      allFragmentSizes.getDuration(MonitoredQuantity::RECENT) << " s";
    maker.addText(tableDiv, tmpString.str());
  }
  {
    tableDiv = maker.addNode("th", tableRow);
    std::ostringstream tmpString;
    tmpString << std::fixed << std::setprecision(1) <<
      eventFragmentSizes.getDuration(MonitoredQuantity::RECENT) << " s";
    maker.addText(tableDiv, tmpString.str());
  }
  {
    tableDiv = maker.addNode("th", tableRow);
    std::ostringstream tmpString;
    tmpString << std::fixed << std::setprecision(1) <<
      dqmEventFragmentSizes.getDuration(MonitoredQuantity::RECENT) << " s";
    maker.addText(tableDiv, tmpString.str());
  }

  // Frames received entry
  tableRow = maker.addNode("tr", table);
  tableDiv = maker.addNode("td", tableRow);
  maker.addText(tableDiv, "Frames Received");
  tableDiv = maker.addNode("td", tableRow, tableValueAttr);
  maker.addText(tableDiv, allFragmentSizes.getSampleCount(MonitoredQuantity::RECENT), 0);
  tableDiv = maker.addNode("td", tableRow, tableValueAttr);
  maker.addText(tableDiv, eventFragmentSizes.getSampleCount(MonitoredQuantity::RECENT), 0);
  tableDiv = maker.addNode("td", tableRow, tableValueAttr);
  maker.addText(tableDiv, dqmEventFragmentSizes.getSampleCount(MonitoredQuantity::RECENT), 0);

  // Bandwidth
  tableRow = maker.addNode("tr", table);
  tableDiv = maker.addNode("td", tableRow);
  maker.addText(tableDiv, "Bandwidth (MB/s)");
  tableDiv = maker.addNode("td", tableRow, tableValueAttr);
  maker.addText(tableDiv, allFragmentSizes.getValueRate(MonitoredQuantity::RECENT));
  tableDiv = maker.addNode("td", tableRow, tableValueAttr);
  maker.addText(tableDiv, eventFragmentSizes.getValueRate(MonitoredQuantity::RECENT));
  tableDiv = maker.addNode("td", tableRow, tableValueAttr);
  maker.addText(tableDiv, dqmEventFragmentSizes.getValueRate(MonitoredQuantity::RECENT));

  // Rate
  tableRow = maker.addNode("tr", table);
  tableDiv = maker.addNode("td", tableRow);
  maker.addText(tableDiv, "Rate (frames/s)");
  tableDiv = maker.addNode("td", tableRow, tableValueAttr);
  maker.addText(tableDiv, allFragmentSizes.getSampleRate(MonitoredQuantity::RECENT));
  tableDiv = maker.addNode("td", tableRow, tableValueAttr);
  maker.addText(tableDiv, eventFragmentSizes.getSampleRate(MonitoredQuantity::RECENT));
  tableDiv = maker.addNode("td", tableRow, tableValueAttr);
  maker.addText(tableDiv, dqmEventFragmentSizes.getSampleRate(MonitoredQuantity::RECENT));

  // Latency
  tableRow = maker.addNode("tr", table);
  tableDiv = maker.addNode("td", tableRow);
  maker.addText(tableDiv, "Latency (us/frame)");
  tableDiv = maker.addNode("td", tableRow, tableValueAttr);
  maker.addText(tableDiv, allFragmentSizes.getSampleLatency(MonitoredQuantity::RECENT));
  tableDiv = maker.addNode("td", tableRow, tableValueAttr);
  maker.addText(tableDiv, eventFragmentSizes.getSampleLatency(MonitoredQuantity::RECENT));
  tableDiv = maker.addNode("td", tableRow, tableValueAttr);
  maker.addText(tableDiv, dqmEventFragmentSizes.getSampleLatency(MonitoredQuantity::RECENT));

  // Maximum Bandwidth
  tableRow = maker.addNode("tr", table);
  tableDiv = maker.addNode("td", tableRow);
  maker.addText(tableDiv, "Maximum Bandwidth (MB/s)");
  tableDiv = maker.addNode("td", tableRow, tableValueAttr);
  maker.addText(tableDiv, allFragmentBandwidth.getValueMax(MonitoredQuantity::RECENT));
  tableDiv = maker.addNode("td", tableRow, tableValueAttr);
  maker.addText(tableDiv, eventFragmentBandwidth.getValueMax(MonitoredQuantity::RECENT));
  tableDiv = maker.addNode("td", tableRow, tableValueAttr);
  maker.addText(tableDiv, dqmEventFragmentBandwidth.getValueMax(MonitoredQuantity::RECENT));

  // Minimum Bandwidth
  tableRow = maker.addNode("tr", table);
  tableDiv = maker.addNode("td", tableRow);
  maker.addText(tableDiv, "Minimum Bandwidth (MB/s)");
  tableDiv = maker.addNode("td", tableRow, tableValueAttr);
  maker.addText(tableDiv, allFragmentBandwidth.getValueMin(MonitoredQuantity::RECENT));
  tableDiv = maker.addNode("td", tableRow, tableValueAttr);
  maker.addText(tableDiv, eventFragmentBandwidth.getValueMin(MonitoredQuantity::RECENT));
  tableDiv = maker.addNode("td", tableRow, tableValueAttr);
  maker.addText(tableDiv, dqmEventFragmentBandwidth.getValueMin(MonitoredQuantity::RECENT));

}


void WebPageHelper::addDOMforRunMonitor(XHTMLMaker& maker,
                                        XHTMLMaker::Node *parent,
                                        RunMonitorCollection const& rmc)
{
  MonitoredQuantity const& eventIDsReceived =
    rmc.getEventIDsReceivedMQ();
  MonitoredQuantity const& errorEventIDsReceived =
    rmc.getErrorEventIDsReceivedMQ();
  MonitoredQuantity const& runNumbersSeen =
      rmc.getRunNumbersSeenMQ();
  MonitoredQuantity const& lumiSectionsSeen =
      rmc.getLumiSectionsSeenMQ();

  XHTMLMaker::AttrMap tableAttr;
  tableAttr[ "frame" ] = "void";
  tableAttr[ "rules" ] = "group";
  tableAttr[ "class" ] = "states";
  tableAttr[ "cellpadding" ] = "2";
  tableAttr[ "width" ] = "100%";

  XHTMLMaker::AttrMap colspanAttr;
  colspanAttr[ "colspan" ] = "4";
  
  XHTMLMaker::AttrMap specialRowAttr;
  specialRowAttr[ "class" ] = "special";

  XHTMLMaker::AttrMap tableLabelAttr;
  tableLabelAttr[ "align" ] = "left";
  tableLabelAttr[ "width" ] = "27%";

  XHTMLMaker::AttrMap tableValueAttr;
  tableValueAttr[ "align" ] = "right";
  tableValueAttr[ "width" ] = "23%";

  XHTMLMaker::Node* table = maker.addNode("table", parent, tableAttr);

  XHTMLMaker::Node* tableRow = maker.addNode("tr", table);
  XHTMLMaker::Node* tableDiv = maker.addNode("th", tableRow, colspanAttr);
  maker.addText(tableDiv, "Storage Manager Statistics");

  // Run number and lumi section
  tableRow = maker.addNode("tr", table);
  tableDiv = maker.addNode("td", tableRow, tableLabelAttr);
  maker.addText(tableDiv, "Run number");
  tableDiv = maker.addNode("td", tableRow, tableValueAttr);
  maker.addText(tableDiv, runNumbersSeen.getLastSampleValue(), 0);
  tableDiv = maker.addNode("td", tableRow, tableLabelAttr);
  maker.addText(tableDiv, "Lumi section");
  tableDiv = maker.addNode("td", tableRow, tableValueAttr);
  maker.addText(tableDiv, lumiSectionsSeen.getLastSampleValue(), 0);

  // Total events received
  tableRow = maker.addNode("tr", table, specialRowAttr);
  tableDiv = maker.addNode("td", tableRow, tableLabelAttr);
  maker.addText(tableDiv, "Events received (non-unique)");
  tableDiv = maker.addNode("td", tableRow, tableValueAttr);
  maker.addText(tableDiv, eventIDsReceived.getSampleCount(), 0);
  tableDiv = maker.addNode("td", tableRow, tableLabelAttr);
  maker.addText(tableDiv, "Error events received");
  tableDiv = maker.addNode("td", tableRow, tableValueAttr);
  maker.addText(tableDiv, errorEventIDsReceived.getSampleCount(), 0);

  // Last event IDs
  tableRow = maker.addNode("tr", table);
  tableDiv = maker.addNode("td", tableRow, tableLabelAttr);
  maker.addText(tableDiv, "Last event ID");
  tableDiv = maker.addNode("td", tableRow, tableValueAttr);
  maker.addText(tableDiv, eventIDsReceived.getLastSampleValue(), 0);
  tableDiv = maker.addNode("td", tableRow, tableLabelAttr);
  maker.addText(tableDiv, "Last error event ID");
  tableDiv = maker.addNode("td", tableRow, tableValueAttr);
  maker.addText(tableDiv, errorEventIDsReceived.getLastSampleValue(), 0);

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
