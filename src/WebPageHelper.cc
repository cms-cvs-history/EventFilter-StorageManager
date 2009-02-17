// $Id: WebPageHelper.cc,v 1.1.2.4 2009/02/16 16:13:04 mommsen Exp $

#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <sys/statfs.h>

#include "EventFilter/StorageManager/interface/FragmentMonitorCollection.h"
#include "EventFilter/StorageManager/interface/RunMonitorCollection.h"
#include "EventFilter/StorageManager/interface/WebPageHelper.h"

using namespace stor;


WebPageHelper::WebPageHelper(xdaq::ApplicationDescriptor *appDescriptor) :
_appDescriptor(appDescriptor)
{}


void WebPageHelper::defaultWebPage
(
  xgi::Output *out, 
  const std::string stateName,
  const boost::shared_ptr<StatisticsReporter>& statReporter,
  toolbox::mem::Pool *pool,
  const int nLogicalDisk,
  const std::string filePath
)
{
  XHTMLMaker* maker = XHTMLMaker::instance();
  
  // Create the body with the standard header
  XHTMLMaker::Node* body = createWebPageBody(stateName);

  //TODO: Failed printout

  // Run and event summary
  statReporter->getRunMonitorCollection().addDOMElement(body);
  
  // Resource usage
  addDOMforResourceUsage(body, pool, nLogicalDisk, filePath);
  
  // Add the received data statistics table
  statReporter->getFragmentMonitorCollection().addDOMElement(body);

  addDOMforSMLinks(body);
  
  // Dump the webpage to the output stream
  maker->out(*out);

}


XHTMLMaker::Node* WebPageHelper::createWebPageBody(const std::string stateName)
{
  XHTMLMaker* maker = XHTMLMaker::instance();
  
  std::ostringstream title;
  title << _appDescriptor->getClassName()
    << " instance " << _appDescriptor->getInstance();
  XHTMLMaker::Node* body = maker->start(title.str());
  
  std::ostringstream stylesheetLink;
  stylesheetLink << "/" << _appDescriptor->getURN()
    << "/styles.css";
  XHTMLMaker::AttrMap stylesheetAttr;
  stylesheetAttr[ "rel" ] = "stylesheet";
  stylesheetAttr[ "type" ] = "text/css";
  stylesheetAttr[ "href" ] = stylesheetLink.str();
  maker->addNode("link", maker->getHead(), stylesheetAttr);
  
  XHTMLMaker::AttrMap tableAttr;
  tableAttr[ "border" ] = "0";
  tableAttr[ "cellspacing" ] = "7";
  tableAttr[ "width" ] = "100%";
  XHTMLMaker::Node* table = maker->addNode("table", body, tableAttr);
  
  XHTMLMaker::Node* tableRow = maker->addNode("tr", table);
  
  XHTMLMaker::AttrMap tableDivAttr;
  tableDivAttr[ "align" ] = "left";
  tableDivAttr[ "width" ] = "64";
  XHTMLMaker::Node* tableDiv = maker->addNode("td", tableRow, tableDivAttr);

  XHTMLMaker::AttrMap smLinkAttr;
  smLinkAttr[ "href" ] = _appDescriptor->getContextDescriptor()->getURL()
    + "/" + _appDescriptor->getURN();
  XHTMLMaker::Node* smLink = maker->addNode("a", tableDiv, smLinkAttr);
  
  XHTMLMaker::AttrMap smImgAttr;
  smImgAttr[ "align" ] = "middle";
  smImgAttr[ "src" ] = "/evf/images/smicon.jpg"; // $XDAQ_DOCUMENT_ROOT is prepended to this path
  smImgAttr[ "alt" ] = "main";
  smImgAttr[ "width" ] = "64";
  smImgAttr[ "height" ] = "64";
  smImgAttr[ "border" ] = "0";
  maker->addNode("img", smLink, smImgAttr);
  
  tableDivAttr[ "width" ] = "40%";
  tableDiv = maker->addNode("td", tableRow, tableDivAttr);
  XHTMLMaker::Node* header = maker->addNode("h3", tableDiv);
  maker->addText(header, title.str());
  
  tableDivAttr[ "width" ] = "30%";
  tableDiv = maker->addNode("td", tableRow, tableDivAttr);
  header = maker->addNode("h3", tableDiv);
  maker->addText(header, stateName);
  
  tableDivAttr[ "align" ] = "right";
  tableDivAttr[ "width" ] = "64";
  tableDiv = maker->addNode("td", tableRow, tableDivAttr);
  
  XHTMLMaker::AttrMap xdaqLinkAttr;
  xdaqLinkAttr[ "href" ] = "/urn:xdaq-application:lid=3";
  XHTMLMaker::Node* xdaqLink = maker->addNode("a", tableDiv, xdaqLinkAttr);
  
  XHTMLMaker::AttrMap xdaqImgAttr;
  xdaqImgAttr[ "align" ] = "middle";
  xdaqImgAttr[ "src" ] = "/hyperdaq/images/HyperDAQ.jpg"; // $XDAQ_DOCUMENT_ROOT is prepended to this path
  xdaqImgAttr[ "alt" ] = "HyperDAQ";
  xdaqImgAttr[ "width" ] = "64";
  xdaqImgAttr[ "height" ] = "64";
  xdaqImgAttr[ "border" ] = "0";
  maker->addNode("img", xdaqLink, xdaqImgAttr);

  maker->addNode("hr", body);
  
  return body;
}


void WebPageHelper::addDOMforSMLinks(xercesc::DOMElement *parent)
{
  XHTMLMaker* maker = XHTMLMaker::instance();

  std::string url = _appDescriptor->getContextDescriptor()->getURL()
    + "/" + _appDescriptor->getURN();

  XHTMLMaker::AttrMap linkAttr;
  XHTMLMaker::Node *link;

  maker->addNode("hr", parent);

  linkAttr[ "href" ] = url + "/storedData";
  link = maker->addNode("a", parent, linkAttr);
  maker->addText(link, "Stored data web page (remainder of old default web page)");

  maker->addNode("hr", parent);

  linkAttr[ "href" ] = url + "/rbsenderlist";
  link = maker->addNode("a", parent, linkAttr);
  maker->addText(link, "RB Sender list web page");

  maker->addNode("hr", parent);

  linkAttr[ "href" ] = url + "/streameroutput";
  link = maker->addNode("a", parent, linkAttr);
  maker->addText(link, "Streamer Output Status web page");

  maker->addNode("hr", parent);

  linkAttr[ "href" ] = url + "/EventServerStats?update=off";
  link = maker->addNode("a", parent, linkAttr);
  maker->addText(link, "Event Server Statistics");

  maker->addNode("hr", parent);

}


void WebPageHelper::addDOMforResourceUsage
(
  xercesc::DOMElement *parent,
  toolbox::mem::Pool *pool,
  const int nLogicalDisk,
  const std::string filePath
)
{
  XHTMLMaker* maker = XHTMLMaker::instance();
  
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
  
  XHTMLMaker::Node* outerTable = maker->addNode("table", parent, tableAttr);
  XHTMLMaker::Node* outerTableRow = maker->addNode("tr", outerTable);
  XHTMLMaker::Node* outerTableDiv = maker->addNode("th", outerTableRow, colspanAttr);
  maker->addText(outerTableDiv, "Resource Usage");
  
  outerTableRow = maker->addNode("tr", outerTable);
  outerTableDiv = maker->addNode("td", outerTableRow, halfWidthAttr);
  XHTMLMaker::Node* table = maker->addNode("table", outerTableDiv, innerTableAttr);
  
  // Memory pool usage
  XHTMLMaker::Node* tableRow = maker->addNode("tr", table);
  XHTMLMaker::Node* tableDiv;
  if (pool)
  {
    tableDiv = maker->addNode("td", tableRow, tableLabelAttr);
    maker->addText(tableDiv, "Memory pool used (bytes)");
    tableDiv = maker->addNode("td", tableRow, tableValueAttr);
    maker->addText(tableDiv, pool->getMemoryUsage().getUsed(), 0);
  }
  else
  {
    tableDiv = maker->addNode("td", tableRow, colspanAttr);
    maker->addText(tableDiv, "Memory pool pointer not yet available");
  }
  
  
  // Disk usage
  unsigned int nD = nLogicalDisk ? nLogicalDisk : 1;
  for(unsigned int i=0;i<nD;++i) {
    std::string path(filePath);
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
    
    tableRow = maker->addNode("tr", table);
    tableDiv = maker->addNode("td", tableRow, tableLabelAttr);
    {
      std::ostringstream tmpString;
      tmpString << "Disk " << i << " usage";
      maker->addText(tableDiv, tmpString.str());
    }
    if(used>89)
      tableDiv = maker->addNode("td", tableRow, warningAttr);
    else
      tableDiv = maker->addNode("td", tableRow, tableValueAttr);
    {
      std::ostringstream tmpString;
      tmpString << used << "% (" << btotal-bfree << " of " << btotal << " GB)";
      maker->addText(tableDiv, tmpString.str());
    }
  }
  
  outerTableDiv = maker->addNode("td", outerTableRow, halfWidthAttr);
  table = maker->addNode("table", outerTableDiv, innerTableAttr);
  
  // # copy worker
  tableRow = maker->addNode("tr", table);
  tableDiv = maker->addNode("td", tableRow, tableLabelAttr);
  maker->addText(tableDiv, "# CopyWorker");
  tableDiv = maker->addNode("td", tableRow, tableValueAttr);
  maker->addText(tableDiv, getProcessCount("CopyWorker.pl"), 0);
  
  // # inject worker
  tableRow = maker->addNode("tr", table);
  tableDiv = maker->addNode("td", tableRow, tableLabelAttr);
  maker->addText(tableDiv, "# InjectWorker");
  tableDiv = maker->addNode("td", tableRow, tableValueAttr);
  maker->addText(tableDiv, getProcessCount("InjectWorker.pl"), 0);
  
}


int WebPageHelper::getProcessCount(std::string processName) const
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
