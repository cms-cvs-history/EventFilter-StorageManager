// $Id: EventFileHandler.cc,v 1.1.2.4 2009/03/20 10:34:36 mommsen Exp $

#include <EventFilter/StorageManager/interface/EventFileHandler.h>
#include <IOPool/Streamer/interface/EventMessage.h>

#include <iostream>
 
using namespace stor;


EventFileHandler::EventFileHandler
(
  InitMsgSharedPtr view,
  FilesMonitorCollection::FileRecordPtr fileRecord,
  const DiskWritingParams& dwParams
) :
FileHandler(fileRecord, dwParams),
_writer(
  fileRecord->completeFileName()+".dat",
  fileRecord->completeFileName()+".ind"
)
{
  writeHeader(view);
}


EventFileHandler::~EventFileHandler()
{
  closeFile();
}


void EventFileHandler::writeHeader(InitMsgSharedPtr view)
{
  // Fix me: use correct API:  _writer.doOutputHeader(view);
  //_fileRecord->fileSize.addSample(view.size());
  // Fix me: the header increments the event count
}


void EventFileHandler::writeEvent(const I2OChain& event)
{
  // Fix me: use correct API: _writer.doOutputEvent(view);
  _fileRecord->fileSize.addSample(static_cast<uint32_t>(event.totalDataSize()));
  _lastEntry = utils::getCurrentTime();
}


void EventFileHandler::closeFile()
{
  _writer.stop();
  _fileRecord->fileSize.addSample(_writer.getStreamEOFSize());
  setAdler(_writer.get_adler32_stream(), _writer.get_adler32_index());
  moveFileToClosed(true);
  writeToSummaryCatalog();
  updateDatabase();
}



/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
