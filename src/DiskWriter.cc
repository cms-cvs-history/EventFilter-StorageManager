// $Id: DiskWriter.cc,v 1.1.2.4 2009/03/01 20:36:29 biery Exp $

#include "EventFilter/StorageManager/interface/DiskWriter.h"
#include "EventFilter/StorageManager/interface/Exception.h"
#include "EventFilter/StorageManager/interface/FRDStreamHandler.h"
#include "EventFilter/StorageManager/interface/EventStreamHandler.h"

using namespace stor;

DiskWriter::DiskWriter(SharedResourcesPtr sr) :
_sharedResources(sr),
_timeout(1000)
{}


void DiskWriter::writeNextEvent()
{
  I2OChain event;
  boost::shared_ptr<StreamQueue> sq = _sharedResources->_streamQueue;
  if (sq->deq_timed_wait(event, _timeout))
  {
    writeEventToStreams(event);
  }
}


void DiskWriter::writeEventToStreams(const I2OChain& event)
{
  std::vector<StreamID> streams = event.getStreamTags();
  for (
    std::vector<StreamID>::iterator it = streams.begin(), itEnd = streams.end();
    it != itEnd;
    ++it
  )
  {
    try
    {
      _streamHandlers.at(*it)->writeEvent(event);
    }
    catch (std::out_of_range& e)
    {
      std::ostringstream msg;
      msg << "Unable to retrieve stream handler for " << (*it) << " : ";
      msg << e.what();
      XCEPT_RAISE(exception::UnknownStreamId, msg.str());
    }
  }
}


void DiskWriter::configureEventStreams(EventStreamConfigurationInfoList& cfgList)
{
  for (
    EventStreamConfigurationInfoList::iterator it = cfgList.begin(),
      itEnd = cfgList.end();
    it != itEnd;
    ++it
  ) 
  {
    makeEventStream(*it);
  }
}


void DiskWriter::configureErrorStreams(ErrorStreamConfigurationInfoList& cfgList)
{
  for (
    ErrorStreamConfigurationInfoList::iterator it = cfgList.begin(),
      itEnd = cfgList.end();
    it != itEnd;
    ++it
  ) 
  {
    makeErrorStream(*it);
  }
}


void DiskWriter::makeEventStream(EventStreamConfigurationInfo& streamCfg)
{
  boost::shared_ptr<EventStreamHandler> newHandler(
    new EventStreamHandler(streamCfg, _sharedResources)
  );
  _streamHandlers.push_back(boost::dynamic_pointer_cast<StreamHandler>(newHandler));
  streamCfg.setStreamId(_streamHandlers.size() - 1);
}


void DiskWriter::makeErrorStream(ErrorStreamConfigurationInfo& streamCfg)
{
  boost::shared_ptr<FRDStreamHandler> newHandler(
    new FRDStreamHandler(streamCfg, _sharedResources)
  );
  _streamHandlers.push_back(boost::dynamic_pointer_cast<StreamHandler>(newHandler));
  streamCfg.setStreamId(_streamHandlers.size() - 1);
}


void DiskWriter::destroyStreams()
{
  _streamHandlers.clear();
}


const bool DiskWriter::empty() const
{
  // TODO: actual implementation of logic: all events are written
  return true;
}




/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
