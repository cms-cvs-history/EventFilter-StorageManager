// $Id: RunMonitorCollection.cc,v 1.1.2.3 2009/02/16 13:40:12 mommsen Exp $

#include <string>
#include <sstream>
#include <iomanip>

#include "EventFilter/StorageManager/interface/Exception.h"
#include "EventFilter/StorageManager/interface/RunMonitorCollection.h"
#include "EventFilter/StorageManager/interface/XHTMLMaker.h"

using namespace stor;

RunMonitorCollection::RunMonitorCollection(xdaq::Application *app) :
MonitorCollection(app, "Run")
{
  _infoSpaceItems.push_back(std::make_pair("runNumber", &_runNumber));
  _infoSpaceItems.push_back(std::make_pair("receivedEvents", &_receivedEvents));
  _infoSpaceItems.push_back(std::make_pair("receivedErrorEvents", &_receivedErrorEvents));

  putItemsIntoInfoSpace();
}


void RunMonitorCollection::do_calculateStatistics()
{
  eventIDsReceived.calculateStatistics();
  errorEventIDsReceived.calculateStatistics();
  runNumbersSeen.calculateStatistics();
  lumiSectionsSeen.calculateStatistics();
}


void RunMonitorCollection::do_updateInfoSpace()
{
  std::string errorMsg =
    "Failed to update values of items in info space " + _infoSpace->name();

  // Lock the infospace to assure that all items are consistent
  try
  {
    _infoSpace->lock();
    _runNumber = static_cast<xdata::UnsignedInteger32>(runNumbersSeen.getLastSampleValue());
    _receivedEvents = static_cast<xdata::UnsignedInteger32>(eventIDsReceived.getSampleCount());
    _receivedErrorEvents = static_cast<xdata::UnsignedInteger32>(errorEventIDsReceived.getSampleCount());
    _infoSpace->unlock();
  }
  catch(std::exception &e)
  {
    _infoSpace->unlock();
 
    errorMsg += ": ";
    errorMsg += e.what();
    XCEPT_RAISE(stor::exception::Monitoring, errorMsg);
  }
  catch (...)
  {
    _infoSpace->unlock();
 
    errorMsg += " : unknown exception";
    XCEPT_RAISE(stor::exception::Monitoring, errorMsg);
  }

  try
  {
    // The fireItemGroupChanged locks the infospace
    _infoSpace->fireItemGroupChanged(_infoSpaceItemNames, this);
  }
  catch (xdata::exception::Exception &e)
  {
    XCEPT_RETHROW(stor::exception::Infospace, errorMsg, e);
  }
}


void RunMonitorCollection::do_addDOMElement(XHTMLMaker& maker, XHTMLMaker::Node *parent) const
{
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




/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
