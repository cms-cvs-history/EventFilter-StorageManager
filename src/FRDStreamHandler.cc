// $Id: FRDStreamHandler.cc,v 1.1.2.4 2009/03/01 20:36:29 biery Exp $

#include "EventFilter/StorageManager/interface/FRDFileHandler.h"
#include "EventFilter/StorageManager/interface/FRDStreamHandler.h"


using namespace stor;


FRDStreamHandler::FRDStreamHandler
(
  const ErrorStreamConfigurationInfo& streamConfig,
  SharedResourcesPtr sharedResources
):
StreamHandler(sharedResources),
_streamConfig(streamConfig)
{}


FRDStreamHandler::FileHandlerPtr FRDStreamHandler::newFileHandler(const I2OChain& event)
{
  FilesMonitorCollection::FileRecordPtr fileRecord = getNewFileRecord(event);

  FileHandlerPtr newFileHandler(
    new FRDFileHandler(fileRecord, _diskWritingParams)
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
