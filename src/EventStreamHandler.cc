// $Id: EventStreamHandler.cc,v 1.1.2.4 2009/03/01 20:36:29 biery Exp $

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
    new EventFileHandler(_initMsgView, fileRecord, _diskWritingParams)
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
