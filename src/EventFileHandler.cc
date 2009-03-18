// $Id: EventFileHandler.cc,v 1.1.2.2 2009/03/17 15:57:36 mommsen Exp $

#include <EventFilter/StorageManager/interface/EventFileHandler.h>
#include <IOPool/Streamer/interface/EventMessage.h>

#include <iostream>
 
using namespace stor;


EventFileHandler::EventFileHandler
(
  InitMsgView const& view,
  FilesMonitorCollection::FileRecord& fileRecord,
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


void EventFileHandler::writeHeader(InitMsgView const& view)
{
  _writer.doOutputHeader(view);
  _fileRecord.fileSize.addSample(view.size());
  // Fix me: the header increments the event count
}


void EventFileHandler::writeEvent(const I2OChain& chain)
{
  EventMsgView view( (void *) chain.getBufferData() );
  _writer.doOutputEvent(view);
  _fileRecord.fileSize.addSample(view.size());
  _lastEntry = utils::getCurrentTime();
}


void EventFileHandler::closeFile()
{
  _writer.stop();
  _fileRecord.fileSize.addSample(_writer.getStreamEOFSize());
  setadler(_writer.get_adler32_stream(), _writer.get_adler32_index());
  moveFileToClosed(true);
  writeToSummaryCatalog();
  updateDatabase();
}


//
// *** report status of FileHandler
//
// void EventFileHandler::report(ostream &os, int indentation) const
// {
//   string prefix(indentation, ' ');
//   os << prefix << "------------- EventFileHandler -------------\n";
//   _file -> report(os,indentation);
//   double time = (double) _file -> lastEntry() - (double) _file -> firstEntry();
//   double rate = (time>0) ? (double) _file -> events() / (double) time : 0.; 
//   double tput = (time>0) ? (double) _file -> fileSize() / ((double) time * 1048576.) : 0.; 
//   os << prefix << "rate                " << rate            << " evts/s\n";
//   os << prefix << "throughput          " << tput            << " MB/s\n";
//   os << prefix << "time                " << time            << " s\n";
//   os << prefix << "-----------------------------------------\n";  
// }


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
