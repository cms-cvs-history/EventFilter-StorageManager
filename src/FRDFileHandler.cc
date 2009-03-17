// $Id: FRDFileHandler.cc,v 1.1.2.1 2009/03/16 10:46:50 mommsen Exp $

#include <EventFilter/StorageManager/interface/FRDFileHandler.h>
#include <IOPool/Streamer/interface/FRDEventMessage.h>

#include <iostream>
 
using namespace stor;


FRDFileHandler::FRDFileHandler
(
  const uint32_t lumiSection,
  const std::string &file,
  DiskWritingParams dwParams
) :
FileHandler(lumiSection, file, dwParams),
_writer(completeFileName()+".dat")
{
  firstEntry(utils::getCurrentTime());
  insertFileInDatabase();
}


FRDFileHandler::~FRDFileHandler()
{
  closeFile();
}


void FRDFileHandler::writeEvent(const I2OChain& chain)
{
  FRDEventMsgView view( (void *) chain.getBufferData() );
  _writer.doOutputEvent(view);
  increaseFileSize(view.size());
  lastEntry(utils::getCurrentTime());
  increaseEventCount();
}


void FRDFileHandler::closeFile()
{
  _writer.stop();
  moveErrorFileToClosed();
  writeToSummaryCatalog();
  updateDatabase();
}


//
// *** report status of FileHandler
//
// void FRDFileHandler::report(ostream &os, int indentation) const
// {
//   string prefix(indentation, ' ');
//   os << prefix << "------------- FRDFileHandler -------------\n";
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
