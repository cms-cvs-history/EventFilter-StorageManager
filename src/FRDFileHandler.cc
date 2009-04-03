// $Id: FRDFileHandler.cc,v 1.1.2.8 2009/04/03 13:36:37 mommsen Exp $

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
  unsigned int fragCount = chain.fragmentCount();
  for (unsigned int idx = 0; idx < fragCount; ++idx)
    {
#ifdef IOP_STR_V3_TAG
      _writer.doOutputEventFragment(chain.dataLocation(idx),
                                    chain.dataSize(idx));
#endif
    }

  _fileRecord->fileSize += chain.totalDataSize();
  ++_fileRecord->eventCount;
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
