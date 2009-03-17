// $Id: EventFileHandler.cc,v 1.1.2.1 2009/03/16 10:46:50 mommsen Exp $

#include <EventFilter/StorageManager/interface/EventFileHandler.h>
#include <IOPool/Streamer/interface/EventMessage.h>

#include <iostream>
 
using namespace stor;


EventFileHandler::EventFileHandler
(
  InitMsgView const& view,
  const uint32_t lumiSection,
  const std::string &file,
  DiskWritingParams dwParams
) :
FileHandler(lumiSection, file, dwParams),
_writer(completeFileName()+".dat", completeFileName()+".ind")
{
  writeHeader(view);

  firstEntry(utils::getCurrentTime());
  insertFileInDatabase();
}


EventFileHandler::~EventFileHandler()
{
  closeFile();
}


void EventFileHandler::writeHeader(InitMsgView const& view)
{
  _writer.doOutputHeader(view);
  increaseFileSize(view.size());
}


void EventFileHandler::writeEvent(const I2OChain& chain)
{
  EventMsgView view( (void *) chain.getBufferData() );
  _writer.doOutputEvent(view);
  increaseFileSize(view.size());
  lastEntry(utils::getCurrentTime());
  increaseEventCount();
}


void EventFileHandler::closeFile()
{
  _writer.stop();
  increaseFileSize(_writer.getStreamEOFSize());
  setadler(_writer.get_adler32_stream(), _writer.get_adler32_index());
  moveFileToClosed();
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
