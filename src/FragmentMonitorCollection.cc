// $Id: FragmentMonitorCollection.cc,v 1.1.2.2 2009/02/04 17:52:34 mommsen Exp $

#include "EventFilter/StorageManager/interface/FragmentMonitorCollection.h"

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
  // No need to lock the infospace, as we are notifying the 
  // infospace for changes (TBC)
  _receivedFrames = static_cast<uint32_t>(allFragmentSizes.getSampleRate());
  _receivedFramesSize = static_cast<uint32_t>(allFragmentSizes.getValueSum());
  _receivedFramesBandwidth = allFragmentSizes.getValueRate(MonitoredQuantity::RECENT);

  // The fireItemGroupChanged locks the infospace
  _infoSpace->fireItemGroupChanged(_infoSpaceItemNames, this);
}


xercesc::DOMElement* FragmentMonitorCollection::do_addDOMElement(xercesc::DOMElement *parent)
{
  //Create the HTML snipped from ownded MonitoredQuantities
  return parent;
}



/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
