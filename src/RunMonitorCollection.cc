// $Id: FragmentMonitorCollection.cc,v 1.1.2.10 2009/02/12 11:24:57 mommsen Exp $

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
  // Lock the infospace to assure that all items are consistent
  try
  {
    _infoSpace->lock();
    _runNumber = static_cast<uint32_t>(runNumbersSeen.getLastSampleValue());
    _infoSpace->unlock();
  }
  catch (...)
  {
    _infoSpace->unlock();
    
    std::ostringstream oss;
    oss << "Failed to update values of items in info space " << _infoSpace->name() 
      << " : unknown exception";
    
    XCEPT_RAISE(stor::exception::Monitoring, oss.str());
  }

  try
  {
    // The fireItemGroupChanged locks the infospace
    _infoSpace->fireItemGroupChanged(_infoSpaceItemNames, this);
  }
  catch (xdata::exception::Exception &e)
  {
    std::ostringstream oss;
    oss << "Failed to fire item group changed for info space " 
      << _infoSpace->name();
    
    XCEPT_RETHROW(stor::exception::Monitoring, oss.str(), e);
  }
}


void RunMonitorCollection::do_addDOMElement(xercesc::DOMElement *parent) const
{
  XHTMLMaker* maker = XHTMLMaker::instance();

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

  XHTMLMaker::Node* table = maker->addNode("table", parent, tableAttr);

  XHTMLMaker::Node* tableRow = maker->addNode("tr", table);
  XHTMLMaker::Node* tableDiv = maker->addNode("th", tableRow, colspanAttr);
  maker->addText(tableDiv, "Storage Manager Statistics");

  // Run number and lumi section
  tableRow = maker->addNode("tr", table);
  tableDiv = maker->addNode("td", tableRow, tableLabelAttr);
  maker->addText(tableDiv, "Run number");
  tableDiv = maker->addNode("td", tableRow, tableValueAttr);
  maker->addText(tableDiv, runNumbersSeen.getLastSampleValue(), 0);
  tableDiv = maker->addNode("td", tableRow, tableLabelAttr);
  maker->addText(tableDiv, "Lumi section");
  tableDiv = maker->addNode("td", tableRow, tableValueAttr);
  maker->addText(tableDiv, lumiSectionsSeen.getLastSampleValue(), 0);

  // Total events received
  tableRow = maker->addNode("tr", table, specialRowAttr);
  tableDiv = maker->addNode("td", tableRow, tableLabelAttr);
  maker->addText(tableDiv, "Events received (non-unique)");
  tableDiv = maker->addNode("td", tableRow, tableValueAttr);
  maker->addText(tableDiv, eventIDsReceived.getSampleCount(), 0);
  tableDiv = maker->addNode("td", tableRow, tableLabelAttr);
  maker->addText(tableDiv, "Error events received");
  tableDiv = maker->addNode("td", tableRow, tableValueAttr);
  maker->addText(tableDiv, errorEventIDsReceived.getSampleCount(), 0);

  // Last event IDs
  tableRow = maker->addNode("tr", table);
  tableDiv = maker->addNode("td", tableRow, tableLabelAttr);
  maker->addText(tableDiv, "Last event ID");
  tableDiv = maker->addNode("td", tableRow, tableValueAttr);
  maker->addText(tableDiv, eventIDsReceived.getLastSampleValue(), 0);
  tableDiv = maker->addNode("td", tableRow, tableLabelAttr);
  maker->addText(tableDiv, "Last error event ID");
  tableDiv = maker->addNode("td", tableRow, tableValueAttr);
  maker->addText(tableDiv, errorEventIDsReceived.getLastSampleValue(), 0);

}




/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
