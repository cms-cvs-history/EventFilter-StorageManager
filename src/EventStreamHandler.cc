// $Id: EventStreamHandler.cc,v 1.1.2.1 2009/03/20 10:34:36 mommsen Exp $

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
_initMsgView
(
  sharedResources->_initMsgCollection->getElementForOutputModule(streamConfig.outputModuleLabel())
)
{}


EventStreamHandler::FileHandlerPtr EventStreamHandler::newFileHandler(const I2OChain& event)
{
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
