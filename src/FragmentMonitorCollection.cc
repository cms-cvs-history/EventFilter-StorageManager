// $Id: FragmentMonitorCollection.cc,v 1.1.2.6 2009/02/05 14:51:46 mommsen Exp $

#include <sstream>
#include <iomanip>

#include "EventFilter/StorageManager/interface/Exception.h"
#include "EventFilter/StorageManager/interface/FragmentMonitorCollection.h"
#include "EventFilter/StorageManager/interface/XHTMLMaker.h"

using namespace stor;

FragmentMonitorCollection::FragmentMonitorCollection(xdaq::Application *app) :
MonitorCollection(app, "Fragment")
{
  _infoSpaceItems.push_back(std::make_pair("receivedFrames", &_receivedFrames));
  _infoSpaceItems.push_back(std::make_pair("receivedFramesSize", &_receivedFramesSize));
  _infoSpaceItems.push_back(std::make_pair("receivedFramesBandwidth", &_receivedFramesBandwidth));

  putItemsIntoInfoSpace();
}


void FragmentMonitorCollection::do_calculateStatistics()
{
  allFragmentSizes.calculateStatistics();
  eventFragmentSizes.calculateStatistics();
  dqmEventFragmentSizes.calculateStatistics();

  eventFragmentBandwidth.addSample(eventFragmentSizes.getValueRate());
  eventFragmentBandwidth.calculateStatistics();
}


void FragmentMonitorCollection::do_updateInfoSpace()
{
  // Lock the infospace to assure that all items are consistent
  _infoSpace->lock();
  _receivedFrames = static_cast<uint32_t>(allFragmentSizes.getSampleRate());
  _receivedFramesSize = static_cast<uint32_t>(allFragmentSizes.getValueSum());
  _receivedFramesBandwidth = allFragmentSizes.getValueRate(MonitoredQuantity::RECENT);
  _infoSpace->unlock();
  // Can these updates throw?
  // If so, we'll need to catch it and release the lock on the infospace.

  try
  {
    // The fireItemGroupChanged locks the infospace
    _infoSpace->fireItemGroupChanged(_infoSpaceItemNames, this);
  }
  catch (xdata::exception::Exception &e)
  {
      std::stringstream oss;
      
      oss << "Failed to fire item group changed for info space " 
        << _infoSpace->name();
      
      XCEPT_RETHROW(stor::exception::Monitoring, oss.str(), e);
  }
}


void FragmentMonitorCollection::do_addDOMElement(xercesc::DOMElement *parent)
{
  XHTMLMaker* maker = XHTMLMaker::instance();

  XHTMLMaker::AttrMap tableAttr;
  tableAttr[ "frame" ] = "void";
  tableAttr[ "rules" ] = "group";
  tableAttr[ "class" ] = "states";

  XHTMLMaker::AttrMap tableHeaderAttr;
  tableHeaderAttr[ "colspan" ] = "2";

  XHTMLMaker::AttrMap tableValueAttr;
  tableValueAttr[ "align" ] = "right";

  XHTMLMaker::Node* table = maker->addNode("table", parent, tableAttr);

  // Received Data Statistics header
  XHTMLMaker::Node* tableRow = maker->addNode("tr", table);
  XHTMLMaker::Node* tableHeader = maker->addNode("th", tableRow, tableHeaderAttr);
  maker->addText(tableHeader, "Received Data Statistics");

  // Parameter/Value header
  tableRow = maker->addNode("tr", table);
  tableHeader = maker->addNode("th", tableRow);
  maker->addText(tableHeader, "Parameter");
  tableHeader = maker->addNode("th", tableRow);
  maker->addText(tableHeader, "Value");

  // Frames received entry
  tableRow = maker->addNode("tr", table);
  XHTMLMaker::Node* tableDiv = maker->addNode("td", tableRow);
  maker->addText(tableDiv, "Frames Received");
  tableDiv = maker->addNode("td", tableRow, tableValueAttr);
  maker->addText(tableDiv, eventFragmentSizes.getSampleCount(), 0);

  // DQM records received entry
  tableRow = maker->addNode("tr", table);
  tableDiv = maker->addNode("td", tableRow);
  maker->addText(tableDiv, "DQM Records Received");
  tableDiv = maker->addNode("td", tableRow, tableValueAttr);
  maker->addText(tableDiv, "n/a");

  // Memory usage does not belong here

  // Recent statistics header
  {
    tableRow = maker->addNode("tr", table);
    tableHeader = maker->addNode("th", tableRow, tableHeaderAttr);
    std::ostringstream tmpString;
    tmpString << "Statistics for last " <<
      std::fixed << std::setprecision(1) <<
      eventFragmentSizes.getDuration(MonitoredQuantity::RECENT) << " sec (and last " <<
      eventFragmentSizes.getSampleCount(MonitoredQuantity::RECENT) << " frames)";
    maker->addText(tableHeader, tmpString.str());
  }

  // Bandwidth
  tableRow = maker->addNode("tr", table);
  tableDiv = maker->addNode("td", tableRow);
  maker->addText(tableDiv, "Bandwidth (MB/s)");
  tableDiv = maker->addNode("td", tableRow, tableValueAttr);
  maker->addText(tableDiv, eventFragmentSizes.getValueRate(MonitoredQuantity::RECENT));

  // Rate
  tableRow = maker->addNode("tr", table);
  tableDiv = maker->addNode("td", tableRow);
  maker->addText(tableDiv, "Rate (Frames/s)");
  tableDiv = maker->addNode("td", tableRow, tableValueAttr);
  maker->addText(tableDiv, eventFragmentSizes.getSampleRate(MonitoredQuantity::RECENT));

  // Latency
  tableRow = maker->addNode("tr", table);
  tableDiv = maker->addNode("td", tableRow);
  maker->addText(tableDiv, "Latency (us/Frame)");
  tableDiv = maker->addNode("td", tableRow, tableValueAttr);
  maker->addText(tableDiv, eventFragmentSizes.getSampleLatency(MonitoredQuantity::RECENT));

  // Maximum Bandwidth
  tableRow = maker->addNode("tr", table);
  tableDiv = maker->addNode("td", tableRow);
  maker->addText(tableDiv, "Maximum Bandwidth (MB/s)");
  tableDiv = maker->addNode("td", tableRow, tableValueAttr);
  maker->addText(tableDiv, eventFragmentSizes.getValueMax(MonitoredQuantity::RECENT));

  // Minimum Bandwidth
  tableRow = maker->addNode("tr", table);
  tableDiv = maker->addNode("td", tableRow);
  maker->addText(tableDiv, "Minimum Bandwidth (MB/s)");
  tableDiv = maker->addNode("td", tableRow, tableValueAttr);
  maker->addText(tableDiv, eventFragmentSizes.getValueMin(MonitoredQuantity::RECENT));



  // Mean performance header
  {
    tableRow = maker->addNode("tr", table);
    tableHeader = maker->addNode("th", tableRow, tableHeaderAttr);
    std::ostringstream tmpString;
    tmpString << "Mean performance for " <<
      std::fixed << std::setprecision(1) <<
      eventFragmentSizes.getDuration(MonitoredQuantity::FULL) << " sec (and " <<
      eventFragmentSizes.getSampleCount(MonitoredQuantity::FULL) << " frames)";
    maker->addText(tableHeader, tmpString.str());
  }

  // Bandwidth
  tableRow = maker->addNode("tr", table);
  tableDiv = maker->addNode("td", tableRow);
  maker->addText(tableDiv, "Bandwidth (MB/s)");
  tableDiv = maker->addNode("td", tableRow, tableValueAttr);
  maker->addText(tableDiv, eventFragmentSizes.getValueRate(MonitoredQuantity::FULL));

  // Rate
  tableRow = maker->addNode("tr", table);
  tableDiv = maker->addNode("td", tableRow);
  maker->addText(tableDiv, "Rate (Frames/s)");
  tableDiv = maker->addNode("td", tableRow, tableValueAttr);
  maker->addText(tableDiv, eventFragmentSizes.getSampleRate(MonitoredQuantity::FULL));

  // Latency
  tableRow = maker->addNode("tr", table);
  tableDiv = maker->addNode("td", tableRow);
  maker->addText(tableDiv, "Latency (us/Frame)");
  tableDiv = maker->addNode("td", tableRow, tableValueAttr);
  maker->addText(tableDiv, eventFragmentSizes.getSampleLatency(MonitoredQuantity::FULL));

  // Total volume received
  tableRow = maker->addNode("tr", table);
  tableDiv = maker->addNode("td", tableRow);
  maker->addText(tableDiv, "Total volume received (MB)");
  tableDiv = maker->addNode("td", tableRow, tableValueAttr);
  maker->addText(tableDiv, eventFragmentSizes.getValueSum(MonitoredQuantity::FULL), 3);

}




/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
