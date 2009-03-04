// $Id: FragmentMonitorCollection.cc,v 1.1.2.16 2009/03/02 18:51:41 biery Exp $

#include <string>
#include <sstream>
#include <iomanip>

#include "EventFilter/StorageManager/interface/Exception.h"
#include "EventFilter/StorageManager/interface/FragmentMonitorCollection.h"

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
      (static_cast<unsigned int>
       (allFragmentSizes.getDuration(MonitoredQuantity::RECENT)));
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
      (static_cast<unsigned int>
       (dqmEventFragmentSizes.getDuration(MonitoredQuantity::RECENT)));
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




/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
