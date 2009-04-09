// $Id: StreamsMonitorCollection.cc,v 1.1.2.3 2009/04/08 09:33:23 mommsen Exp $

#include <string>
#include <sstream>
#include <iomanip>

#include "EventFilter/StorageManager/interface/Exception.h"
#include "EventFilter/StorageManager/interface/StreamsMonitorCollection.h"

using namespace stor;


MonitoredQuantity StreamsMonitorCollection::_allStreamsFileCount;
MonitoredQuantity StreamsMonitorCollection::_allStreamsVolume;
MonitoredQuantity StreamsMonitorCollection::_allStreamsBandwidth;


StreamsMonitorCollection::StreamsMonitorCollection(xdaq::Application *app) :
MonitorCollection(app, "Streams"),
_timeWindowForRecentResults(300)
{
  _allStreamsFileCount.setNewTimeWindowForRecentResults(_timeWindowForRecentResults);
  _allStreamsVolume.setNewTimeWindowForRecentResults(_timeWindowForRecentResults);
  _allStreamsBandwidth.setNewTimeWindowForRecentResults(_timeWindowForRecentResults);

  // These infospace items were defined in the old SM
  _infoSpaceItems.push_back(std::make_pair("storedEvents",  &_storedEvents));
  _infoSpaceItems.push_back(std::make_pair("namesOfStream", &_namesOfStream));
  _infoSpaceItems.push_back(std::make_pair("storedEventsInStream", &_storedEventsInStream));

  putItemsIntoInfoSpace();
}


const StreamsMonitorCollection::StreamRecordPtr
StreamsMonitorCollection::getNewStreamRecord()
{
  boost::shared_ptr<StreamRecord> streamRecord(new StreamsMonitorCollection::StreamRecord());
  streamRecord->fileCount.setNewTimeWindowForRecentResults(_timeWindowForRecentResults);
  streamRecord->volume.setNewTimeWindowForRecentResults(_timeWindowForRecentResults);
  streamRecord->bandwidth.setNewTimeWindowForRecentResults(_timeWindowForRecentResults);
  _streamRecords.push_back(streamRecord);
  return streamRecord;
}


void StreamsMonitorCollection::do_calculateStatistics()
{
  MonitoredQuantity::Stats stats;

  for (
    StreamRecordList::const_iterator 
      it = _streamRecords.begin(), itEnd = _streamRecords.end();
    it != itEnd;
    ++it
  ) 
  {
    (*it)->fileCount.calculateStatistics();
    (*it)->volume.calculateStatistics();
    (*it)->volume.getStats(stats);
    (*it)->bandwidth.addSample(stats.getValueRate());
    (*it)->bandwidth.calculateStatistics();
  }

  _allStreamsFileCount.calculateStatistics();
  _allStreamsVolume.calculateStatistics();
  _allStreamsVolume.getStats(stats);
  _allStreamsBandwidth.addSample(stats.getValueRate());
  _allStreamsBandwidth.calculateStatistics();
}


void StreamsMonitorCollection::do_updateInfoSpace()
{
  std::string errorMsg =
    "Failed to update values of items in info space " + _infoSpace->name();

  // Lock the infospace to assure that all items are consistent
  try
  {
    _infoSpace->lock();

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


void StreamsMonitorCollection::do_reset()
{
  _streamRecords.clear();

  _allStreamsFileCount.reset();
  _allStreamsVolume.reset();
  _allStreamsBandwidth.reset();
}


void StreamsMonitorCollection::StreamRecord::incrementFileCount()
{
  fileCount.addSample(1);
  _allStreamsFileCount.addSample(1);
}


void StreamsMonitorCollection::StreamRecord::addSizeInBytes(double size)
{
  size = size / (1024 * 1024);
  volume.addSample(size);
  _allStreamsVolume.addSample(size);
}



/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
