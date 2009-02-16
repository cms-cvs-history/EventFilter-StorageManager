// $Id: FragmentMonitorCollection.cc,v 1.1.2.12 2009/02/13 14:19:42 mommsen Exp $

#include <string>
#include <sstream>
#include <iomanip>

#include "EventFilter/StorageManager/interface/Exception.h"
#include "EventFilter/StorageManager/interface/FragmentMonitorCollection.h"
#include "EventFilter/StorageManager/interface/XHTMLMaker.h"

using namespace stor;

FragmentMonitorCollection::FragmentMonitorCollection(xdaq::Application *app) :
MonitorCollection(app, "Fragment")
{
  allFragmentSizes.setNewTimeWindowForRecentResults(5);
  allFragmentBandwidth.setNewTimeWindowForRecentResults(5);
  eventFragmentSizes.setNewTimeWindowForRecentResults(5);
  eventFragmentBandwidth.setNewTimeWindowForRecentResults(5);
  dqmEventFragmentSizes.setNewTimeWindowForRecentResults(300);
  dqmEventFragmentBandwidth.setNewTimeWindowForRecentResults(300);


  // There infospace items were defined in the old SM
  _infoSpaceItems.push_back(std::make_pair("duration", &_duration));
  _infoSpaceItems.push_back(std::make_pair("receivedFrames", &_receivedFrames));
  _infoSpaceItems.push_back(std::make_pair("totalSamples", &_totalSamples));
  _infoSpaceItems.push_back(std::make_pair("dqmRecords", &_dqmRecords));
  _infoSpaceItems.push_back(std::make_pair("meanBandwidth", &_meanBandwidth));
  _infoSpaceItems.push_back(std::make_pair("meanLatency", &_meanLatency));
  _infoSpaceItems.push_back(std::make_pair("meanRate", &_meanRate));
  _infoSpaceItems.push_back(std::make_pair("receivedVolume", &_receivedVolume));
  _infoSpaceItems.push_back(std::make_pair("receivedPeriod4Stats", &_receivedPeriod4Stats));
  _infoSpaceItems.push_back(std::make_pair("receivedSamples4Stats", &_receivedSamples4Stats));
  _infoSpaceItems.push_back(std::make_pair("instantBandwidth", &_instantBandwidth));
  _infoSpaceItems.push_back(std::make_pair("instantLatency", &_instantLatency));
  _infoSpaceItems.push_back(std::make_pair("instantRate", &_instantRate));
  _infoSpaceItems.push_back(std::make_pair("maxBandwidth", &_maxBandwidth));
  _infoSpaceItems.push_back(std::make_pair("minBandwidth", &_minBandwidth));
  _infoSpaceItems.push_back(std::make_pair("receivedDQMPeriod4Stats", &_receivedDQMPeriod4Stats));
  _infoSpaceItems.push_back(std::make_pair("receivedDQMSamples4Stats", &_receivedDQMSamples4Stats));

  putItemsIntoInfoSpace();
}

void FragmentMonitorCollection::addEventFragmentSample(const double bytecount) {
  double mbytes = bytecount / 0x100000;
  allFragmentSizes.addSample(mbytes);
  eventFragmentSizes.addSample(mbytes);
}


void FragmentMonitorCollection::addDQMEventFragmentSample(const double bytecount) {
  double mbytes = bytecount / 0x100000;
  allFragmentSizes.addSample(mbytes);
  dqmEventFragmentSizes.addSample(mbytes);
}


void FragmentMonitorCollection::do_calculateStatistics()
{
  allFragmentSizes.calculateStatistics();
  eventFragmentSizes.calculateStatistics();
  dqmEventFragmentSizes.calculateStatistics();

  allFragmentBandwidth.addSample(allFragmentSizes.getValueRate());
  allFragmentBandwidth.calculateStatistics();
  eventFragmentBandwidth.addSample(eventFragmentSizes.getValueRate());
  eventFragmentBandwidth.calculateStatistics();
  dqmEventFragmentBandwidth.addSample(dqmEventFragmentSizes.getValueRate());
  dqmEventFragmentBandwidth.calculateStatistics();
}


void FragmentMonitorCollection::do_updateInfoSpace()
{
  std::string errorMsg =
    "Failed to update values of items in info space " + _infoSpace->name();

  // Lock the infospace to assure that all items are consistent
  try
  {
    _infoSpace->lock();
    _duration       = static_cast<xdata::Double>(allFragmentSizes.getDuration());
    _receivedFrames = static_cast<xdata::UnsignedInteger32>(allFragmentSizes.getSampleCount());
    _totalSamples   = _receivedFrames;
    _dqmRecords     = static_cast<xdata::UnsignedInteger32>(dqmEventFragmentSizes.getSampleCount());
    
    _meanBandwidth  = static_cast<xdata::Double>(allFragmentBandwidth.getValueRate());
    _meanRate       = static_cast<xdata::Double>(allFragmentSizes.getSampleRate());
    _meanLatency    = static_cast<xdata::Double>(allFragmentSizes.getSampleLatency());
    _receivedVolume = static_cast<xdata::Double>(allFragmentSizes.getValueSum());

    _receivedPeriod4Stats  = static_cast<xdata::UnsignedInteger32>
      (allFragmentSizes.getDuration(MonitoredQuantity::RECENT));
    _receivedSamples4Stats = static_cast<xdata::UnsignedInteger32>
      (allFragmentSizes.getSampleCount(MonitoredQuantity::RECENT));

    _instantBandwidth      = static_cast<xdata::Double>
      (allFragmentBandwidth.getValueRate(MonitoredQuantity::RECENT));
    _instantRate           = static_cast<xdata::Double>
      (allFragmentSizes.getSampleRate(MonitoredQuantity::RECENT));
    _instantLatency        = static_cast<xdata::Double>
      (allFragmentSizes.getSampleLatency(MonitoredQuantity::RECENT));
    _maxBandwidth       = static_cast<xdata::Double>
      (allFragmentBandwidth.getValueMax(MonitoredQuantity::RECENT));
    _minBandwidth       = static_cast<xdata::Double>
      (allFragmentBandwidth.getValueMin(MonitoredQuantity::RECENT));

    _receivedDQMPeriod4Stats  = static_cast<xdata::UnsignedInteger32>
      (dqmEventFragmentSizes.getDuration(MonitoredQuantity::RECENT));
    _receivedDQMSamples4Stats = static_cast<xdata::UnsignedInteger32>
      (dqmEventFragmentSizes.getSampleCount(MonitoredQuantity::RECENT));

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
    XCEPT_RETHROW(stor::exception::Monitoring, errorMsg, e);
  }
}


void FragmentMonitorCollection::do_addDOMElement(xercesc::DOMElement *parent) const
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

  XHTMLMaker::AttrMap tableValueAttr;
  tableValueAttr[ "align" ] = "right";
  tableValueAttr[ "width" ] = "23%";

  XHTMLMaker::Node* table = maker->addNode("table", parent, tableAttr);

  // Received Data Statistics header
  XHTMLMaker::Node* tableRow = maker->addNode("tr", table);
  XHTMLMaker::Node* tableDiv = maker->addNode("th", tableRow, colspanAttr);
  maker->addText(tableDiv, "Received I2O Frames");

  // Parameter/Value header
  tableRow = maker->addNode("tr", table);
  tableDiv = maker->addNode("th", tableRow);
  maker->addText(tableDiv, "Parameter");
  tableDiv = maker->addNode("th", tableRow);
  maker->addText(tableDiv, "Total");
  tableDiv = maker->addNode("th", tableRow);
  maker->addText(tableDiv, "Events");
  tableDiv = maker->addNode("th", tableRow);
  maker->addText(tableDiv, "DQM histos");


  // Mean performance header
  tableRow = maker->addNode("tr", table);
  tableDiv = maker->addNode("th", tableRow);
  maker->addText(tableDiv, "Mean performance for");
  {
    tableDiv = maker->addNode("th", tableRow);
    std::ostringstream tmpString;
    tmpString << std::fixed << std::setprecision(1) <<
      allFragmentSizes.getDuration(MonitoredQuantity::FULL) << " s";
    maker->addText(tableDiv, tmpString.str());
  }
  {
    tableDiv = maker->addNode("th", tableRow);
    std::ostringstream tmpString;
    tmpString << std::fixed << std::setprecision(1) <<
      eventFragmentSizes.getDuration(MonitoredQuantity::FULL) << " s";
    maker->addText(tableDiv, tmpString.str());
  }
  {
    tableDiv = maker->addNode("th", tableRow);
    std::ostringstream tmpString;
    tmpString << std::fixed << std::setprecision(1) <<
      dqmEventFragmentSizes.getDuration(MonitoredQuantity::FULL) << " s";
    maker->addText(tableDiv, tmpString.str());
  }

  // Frames received entry
  tableRow = maker->addNode("tr", table);
  tableDiv = maker->addNode("td", tableRow);
  maker->addText(tableDiv, "Frames Received");
  tableDiv = maker->addNode("td", tableRow, tableValueAttr);
  maker->addText(tableDiv, allFragmentSizes.getSampleCount(MonitoredQuantity::FULL), 0);
  tableDiv = maker->addNode("td", tableRow, tableValueAttr);
  maker->addText(tableDiv, eventFragmentSizes.getSampleCount(MonitoredQuantity::FULL), 0);
  tableDiv = maker->addNode("td", tableRow, tableValueAttr);
  maker->addText(tableDiv, dqmEventFragmentSizes.getSampleCount(MonitoredQuantity::FULL), 0);

  // Bandwidth
  tableRow = maker->addNode("tr", table);
  tableDiv = maker->addNode("td", tableRow);
  maker->addText(tableDiv, "Bandwidth (MB/s)");
  tableDiv = maker->addNode("td", tableRow, tableValueAttr);
  maker->addText(tableDiv, allFragmentSizes.getValueRate(MonitoredQuantity::FULL));
  tableDiv = maker->addNode("td", tableRow, tableValueAttr);
  maker->addText(tableDiv, eventFragmentSizes.getValueRate(MonitoredQuantity::FULL));
  tableDiv = maker->addNode("td", tableRow, tableValueAttr);
  maker->addText(tableDiv, dqmEventFragmentSizes.getValueRate(MonitoredQuantity::FULL));

  // Rate
  tableRow = maker->addNode("tr", table);
  tableDiv = maker->addNode("td", tableRow);
  maker->addText(tableDiv, "Rate (frames/s)");
  tableDiv = maker->addNode("td", tableRow, tableValueAttr);
  maker->addText(tableDiv, allFragmentSizes.getSampleRate(MonitoredQuantity::FULL));
  tableDiv = maker->addNode("td", tableRow, tableValueAttr);
  maker->addText(tableDiv, eventFragmentSizes.getSampleRate(MonitoredQuantity::FULL));
  tableDiv = maker->addNode("td", tableRow, tableValueAttr);
  maker->addText(tableDiv, dqmEventFragmentSizes.getSampleRate(MonitoredQuantity::FULL));

  // Latency
  tableRow = maker->addNode("tr", table);
  tableDiv = maker->addNode("td", tableRow);
  maker->addText(tableDiv, "Latency (us/frame)");
  tableDiv = maker->addNode("td", tableRow, tableValueAttr);
  maker->addText(tableDiv, allFragmentSizes.getSampleLatency(MonitoredQuantity::FULL));
  tableDiv = maker->addNode("td", tableRow, tableValueAttr);
  maker->addText(tableDiv, eventFragmentSizes.getSampleLatency(MonitoredQuantity::FULL));
  tableDiv = maker->addNode("td", tableRow, tableValueAttr);
  maker->addText(tableDiv, dqmEventFragmentSizes.getSampleLatency(MonitoredQuantity::FULL));

  // Total volume received
  tableRow = maker->addNode("tr", table);
  tableDiv = maker->addNode("td", tableRow);
  maker->addText(tableDiv, "Total volume received (MB)");
  tableDiv = maker->addNode("td", tableRow, tableValueAttr);
  maker->addText(tableDiv, allFragmentSizes.getValueSum(MonitoredQuantity::FULL), 3);
  tableDiv = maker->addNode("td", tableRow, tableValueAttr);
  maker->addText(tableDiv, eventFragmentSizes.getValueSum(MonitoredQuantity::FULL), 3);
  tableDiv = maker->addNode("td", tableRow, tableValueAttr);
  maker->addText(tableDiv, dqmEventFragmentSizes.getValueSum(MonitoredQuantity::FULL), 3);


  // Recent statistics header
  tableRow = maker->addNode("tr", table);
  tableDiv = maker->addNode("th", tableRow);
  maker->addText(tableDiv, "Recent performance for last");
  {
    tableDiv = maker->addNode("th", tableRow);
    std::ostringstream tmpString;
    tmpString << std::fixed << std::setprecision(1) <<
      allFragmentSizes.getDuration(MonitoredQuantity::RECENT) << " s";
    maker->addText(tableDiv, tmpString.str());
  }
  {
    tableDiv = maker->addNode("th", tableRow);
    std::ostringstream tmpString;
    tmpString << std::fixed << std::setprecision(1) <<
      eventFragmentSizes.getDuration(MonitoredQuantity::RECENT) << " s";
    maker->addText(tableDiv, tmpString.str());
  }
  {
    tableDiv = maker->addNode("th", tableRow);
    std::ostringstream tmpString;
    tmpString << std::fixed << std::setprecision(1) <<
      dqmEventFragmentSizes.getDuration(MonitoredQuantity::RECENT) << " s";
    maker->addText(tableDiv, tmpString.str());
  }

  // Frames received entry
  tableRow = maker->addNode("tr", table);
  tableDiv = maker->addNode("td", tableRow);
  maker->addText(tableDiv, "Frames Received");
  tableDiv = maker->addNode("td", tableRow, tableValueAttr);
  maker->addText(tableDiv, allFragmentSizes.getSampleCount(MonitoredQuantity::RECENT), 0);
  tableDiv = maker->addNode("td", tableRow, tableValueAttr);
  maker->addText(tableDiv, eventFragmentSizes.getSampleCount(MonitoredQuantity::RECENT), 0);
  tableDiv = maker->addNode("td", tableRow, tableValueAttr);
  maker->addText(tableDiv, dqmEventFragmentSizes.getSampleCount(MonitoredQuantity::RECENT), 0);

  // Bandwidth
  tableRow = maker->addNode("tr", table);
  tableDiv = maker->addNode("td", tableRow);
  maker->addText(tableDiv, "Bandwidth (MB/s)");
  tableDiv = maker->addNode("td", tableRow, tableValueAttr);
  maker->addText(tableDiv, allFragmentSizes.getValueRate(MonitoredQuantity::RECENT));
  tableDiv = maker->addNode("td", tableRow, tableValueAttr);
  maker->addText(tableDiv, eventFragmentSizes.getValueRate(MonitoredQuantity::RECENT));
  tableDiv = maker->addNode("td", tableRow, tableValueAttr);
  maker->addText(tableDiv, dqmEventFragmentSizes.getValueRate(MonitoredQuantity::RECENT));

  // Rate
  tableRow = maker->addNode("tr", table);
  tableDiv = maker->addNode("td", tableRow);
  maker->addText(tableDiv, "Rate (frames/s)");
  tableDiv = maker->addNode("td", tableRow, tableValueAttr);
  maker->addText(tableDiv, allFragmentSizes.getSampleRate(MonitoredQuantity::RECENT));
  tableDiv = maker->addNode("td", tableRow, tableValueAttr);
  maker->addText(tableDiv, eventFragmentSizes.getSampleRate(MonitoredQuantity::RECENT));
  tableDiv = maker->addNode("td", tableRow, tableValueAttr);
  maker->addText(tableDiv, dqmEventFragmentSizes.getSampleRate(MonitoredQuantity::RECENT));

  // Latency
  tableRow = maker->addNode("tr", table);
  tableDiv = maker->addNode("td", tableRow);
  maker->addText(tableDiv, "Latency (us/frame)");
  tableDiv = maker->addNode("td", tableRow, tableValueAttr);
  maker->addText(tableDiv, allFragmentSizes.getSampleLatency(MonitoredQuantity::RECENT));
  tableDiv = maker->addNode("td", tableRow, tableValueAttr);
  maker->addText(tableDiv, eventFragmentSizes.getSampleLatency(MonitoredQuantity::RECENT));
  tableDiv = maker->addNode("td", tableRow, tableValueAttr);
  maker->addText(tableDiv, dqmEventFragmentSizes.getSampleLatency(MonitoredQuantity::RECENT));

  // Maximum Bandwidth
  tableRow = maker->addNode("tr", table);
  tableDiv = maker->addNode("td", tableRow);
  maker->addText(tableDiv, "Maximum Bandwidth (MB/s)");
  tableDiv = maker->addNode("td", tableRow, tableValueAttr);
  maker->addText(tableDiv, allFragmentBandwidth.getValueMax(MonitoredQuantity::RECENT));
  tableDiv = maker->addNode("td", tableRow, tableValueAttr);
  maker->addText(tableDiv, eventFragmentBandwidth.getValueMax(MonitoredQuantity::RECENT));
  tableDiv = maker->addNode("td", tableRow, tableValueAttr);
  maker->addText(tableDiv, dqmEventFragmentBandwidth.getValueMax(MonitoredQuantity::RECENT));

  // Minimum Bandwidth
  tableRow = maker->addNode("tr", table);
  tableDiv = maker->addNode("td", tableRow);
  maker->addText(tableDiv, "Minimum Bandwidth (MB/s)");
  tableDiv = maker->addNode("td", tableRow, tableValueAttr);
  maker->addText(tableDiv, allFragmentBandwidth.getValueMin(MonitoredQuantity::RECENT));
  tableDiv = maker->addNode("td", tableRow, tableValueAttr);
  maker->addText(tableDiv, eventFragmentBandwidth.getValueMin(MonitoredQuantity::RECENT));
  tableDiv = maker->addNode("td", tableRow, tableValueAttr);
  maker->addText(tableDiv, dqmEventFragmentBandwidth.getValueMin(MonitoredQuantity::RECENT));

}




/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
