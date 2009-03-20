// $Id: FRDFileHandler.cc,v 1.1.2.3 2009/03/18 18:35:41 mommsen Exp $

#include <EventFilter/StorageManager/interface/FRDFileHandler.h>
#include <IOPool/Streamer/interface/FRDEventMessage.h>

#include <iostream>
 
using namespace stor;


FRDFileHandler::FRDFileHandler
(
  FilesMonitorCollection::FileRecordPtr fileRecord,
  const DiskWritingParams& dwParams
) :
FileHandler(fileRecord, dwParams),
_writer(completeFileName()+".dat")
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
  _lastEntry = utils::getCurrentTime();
}


void FRDFileHandler::closeFile()
{
  _writer.stop();
  moveFileToClosed(false);
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
