// $Id: EventFileHandler.cc,v 1.1.2.3 2009/03/18 18:35:41 mommsen Exp $

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
_writer(completeFileName()+".dat", completeFileName()+".ind")
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
  setadler(_writer.get_adler32_stream(), _writer.get_adler32_index());
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
