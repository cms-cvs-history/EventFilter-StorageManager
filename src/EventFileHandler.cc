// $Id: EventFileHandler.cc,v 1.1.2.7 2009/03/27 01:30:09 biery Exp $

#include <EventFilter/StorageManager/interface/EventFileHandler.h>
#include <IOPool/Streamer/interface/EventMessage.h>

#include <iostream>
 
using namespace stor;


EventFileHandler::EventFileHandler
(
  InitMsgSharedPtr view,
  FilesMonitorCollection::FileRecordPtr fileRecord,
  const DiskWritingParams& dwParams,
  const long long& maxFileSize
) :
FileHandler(fileRecord, dwParams, maxFileSize),
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
#if 0
  edm::StreamerFileWriterEventParams evtParams;

  event.hltTriggerBits(evtParams.hltBits);
  evtParams.headerPtr = (char*) event.headerLocation();
  evtParams.headerSize = event.headerSize();

  unsigned int fragCount = event.fragmentCount();
  evtParams.fragmentCount = fragCount;

  for (unsigned int idx = 0; idx < fragCount; ++idx)
    {
      evtParams.fragmentIndex = idx;
      evtParams.dataPtr = (char*) event.dataLocation(idx);
      evtParams.dataSize = event.dataSize(idx);

      _writer.doOutputEventFragment(evtParams);
    }

  _fileRecord->fileSize.addSample(static_cast<uint32_t>(event.totalDataSize()));
  _lastEntry = utils::getCurrentTime();
#endif
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
