// $Id: EventFileHandler.cc,v 1.1.2.9 2009/03/27 18:56:33 biery Exp $

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

#ifdef NEW_DW_TEST
  InitMsgView initView(&(*view)[0]);
  _writer.doOutputHeader(initView);
  _fileRecord->fileSize.addSample(view->size());
  _lastEntry = utils::getCurrentTime();
#endif
}


void EventFileHandler::writeEvent(const I2OChain& event)
{
#ifdef NEW_DW_TEST
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


const int EventFileHandler::events() const
{
  int eventCount = _fileRecord->fileSize.getSampleCount();
  return (eventCount > 0 ? eventCount - 1 : 0);
}



/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
