// $Id: FragmentMonitorCollection.cc,v 1.1.2.3 2009/02/04 21:51:56 biery Exp $

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
  // Lock the infospace to assure that all items are consistent
  _infoSpace->lock();
  _receivedFrames = static_cast<uint32_t>(allFragmentSizes.getSampleRate());
  _receivedFramesSize = static_cast<uint32_t>(allFragmentSizes.getValueSum());
  _receivedFramesBandwidth = allFragmentSizes.getValueRate(MonitoredQuantity::RECENT);
  _infoSpace->unlock();
  // Can these updates throw?
  // If so, we'll need to catch it and release the lock on the infospace.

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
