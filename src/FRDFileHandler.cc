// $Id: FRDFileHandler.cc,v 1.1.2.6 2009/03/20 17:54:30 mommsen Exp $

#include <EventFilter/StorageManager/interface/FRDFileHandler.h>
#include <IOPool/Streamer/interface/FRDEventMessage.h>

#include <iostream>
 
using namespace stor;


FRDFileHandler::FRDFileHandler
(
  FilesMonitorCollection::FileRecordPtr fileRecord,
  const DiskWritingParams& dwParams,
  const long long& maxFileSize
) :
FileHandler(fileRecord, dwParams, maxFileSize),
_writer(fileRecord->completeFileName()+".dat")
{}


FRDFileHandler::~FRDFileHandler()
{
  closeFile();
}


void FRDFileHandler::writeEvent(const I2OChain& chain)
{
  FRDEventMsgView view( (void *) chain.getBufferData() );
  _writer.doOutputEvent(view);
  _fileRecord->fileSize.addSample(view.size());
  _fileRecord->eventCount.addSample(1);
  _lastEntry = utils::getCurrentTime();
}


void FRDFileHandler::closeFile()
{
  _writer.stop();
  moveFileToClosed(false);
  writeToSummaryCatalog();
  updateDatabase();
}



/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
