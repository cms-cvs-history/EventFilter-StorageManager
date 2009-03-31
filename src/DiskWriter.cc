// $Id: DiskWriter.cc,v 1.1.2.7 2009/03/27 01:55:53 biery Exp $

#include "toolbox/task/WorkLoopFactory.h"
#include "xcept/tools.h"

#include "EventFilter/StorageManager/interface/DiskWriter.h"
#include "EventFilter/StorageManager/interface/Exception.h"
#include "EventFilter/StorageManager/interface/FRDStreamHandler.h"
#include "EventFilter/StorageManager/interface/EventStreamHandler.h"

using namespace stor;

DiskWriter::DiskWriter(xdaq::Application *app, SharedResourcesPtr sr) :
_app(app),
_sharedResources(sr),
_timeout(1),
_actionIsActive(true)
{}


DiskWriter::~DiskWriter()
{
  // Stop the activity
  _actionIsActive = false;

  // Cancel the workloop (will wait until the action has finished)
  _writingWL->cancel();
}


void DiskWriter::startWorkLoop()
{
  try
  {
    std::string identifier = utils::getIdentifier(_app->getApplicationDescriptor());
    
    _writingWL = toolbox::task::getWorkLoopFactory()->
      getWorkLoop( identifier + "DiskWriter",
        "waiting" );
    
    if ( ! _writingWL->isActive() )
    {
      toolbox::task::ActionSignature* processAction = 
        toolbox::task::bind(this, &DiskWriter::writeAction, 
          identifier + "WriteNextEvent");
      _writingWL->submit(processAction);
      
      _writingWL->activate();
    }
  }
  catch (xcept::Exception& e)
  {
    std::string msg = "Failed to start workloop 'DiskWriter' with 'writeNextEvent'.";
    XCEPT_RETHROW(stor::exception::DiskWriting, msg, e);
  }
}


bool DiskWriter::writeAction(toolbox::task::WorkLoop*)
{
  std::string errorMsg = "Failed to write an event";
  
  try
  {
    writeNextEvent();
  }
  catch(xcept::Exception &e)
  {
    LOG4CPLUS_ERROR(_app->getApplicationLogger(),
      errorMsg << xcept::stdformat_exception_history(e));

    XCEPT_DECLARE_NESTED(stor::exception::DiskWriting,
      sentinelException, errorMsg, e);
    _app->notifyQualified("error", sentinelException);
    // How to go to failed state?
  }
  catch(std::exception &e)
  {
    errorMsg += ": ";
    errorMsg += e.what();

    LOG4CPLUS_ERROR(_app->getApplicationLogger(),
      errorMsg);
    
    XCEPT_DECLARE(stor::exception::DiskWriting,
      sentinelException, errorMsg);
    _app->notifyQualified("error", sentinelException);
  }
  catch(...)
  {
    errorMsg += ": Unknown exception";

    LOG4CPLUS_ERROR(_app->getApplicationLogger(),
      errorMsg);
    
    XCEPT_DECLARE(stor::exception::DiskWriting,
      sentinelException, errorMsg);
    _app->notifyQualified("error", sentinelException);
  }

  return _actionIsActive;
}


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
  boost::mutex::scoped_lock sl(_streamConfigMutex);

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


void DiskWriter::configureEventStreams(EvtStrConfigList& cfgList)
{
  boost::mutex::scoped_lock sl(_streamConfigMutex);

  for (
    EvtStrConfigList::iterator it = cfgList.begin(),
      itEnd = cfgList.end();
    it != itEnd;
    ++it
  ) 
  {
    makeEventStream(*it);
  }
}


void DiskWriter::configureErrorStreams(ErrStrConfigList& cfgList)
{
  boost::mutex::scoped_lock sl(_streamConfigMutex);

  for (
    ErrStrConfigList::iterator it = cfgList.begin(),
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
  boost::mutex::scoped_lock sl(_streamConfigMutex);

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
