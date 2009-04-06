// $Id: EventStreamHandler.cc,v 1.1.2.3 2009/03/27 18:55:52 biery Exp $

#include "EventFilter/StorageManager/interface/EventFileHandler.h"
#include "EventFilter/StorageManager/interface/EventStreamHandler.h"


using namespace stor;


EventStreamHandler::EventStreamHandler
(
  const EventStreamConfigurationInfo& streamConfig,
  SharedResourcesPtr sharedResources
):
StreamHandler(sharedResources),
_streamConfig(streamConfig),
_initMsgCollection(sharedResources->_initMsgCollection)
{}


const EventStreamHandler::FileHandlerPtr
EventStreamHandler::newFileHandler(const I2OChain& event)
{
  // the INIT message is not available when the EventStreamHandler is
  // constructed, so we need to fetch it when we first need a new file
  // handler (when the first event is received, which is after the 
  // INIT messages have been received)
  if (_initMsgView.get() == 0)
    {
      _initMsgView = _initMsgCollection->getElementForOutputModule(_streamConfig.outputModuleLabel());
    }

  FilesMonitorCollection::FileRecordPtr fileRecord = getNewFileRecord(event);

  FileHandlerPtr newFileHandler(
    new EventFileHandler(_initMsgView, fileRecord, _diskWritingParams, getMaxFileSize())
  );
  _fileHandlers.push_back(newFileHandler);

  return newFileHandler;
}


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
