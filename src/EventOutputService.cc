// $Id: EventOutputService.cc,v 1.3 2008/10/24 17:41:55 loizides Exp $

#include <EventFilter/StorageManager/interface/EventOutputService.h>
#include <IOPool/Streamer/interface/EventMessage.h>

#include <iostream>
 
using namespace edm;
using namespace stor;
using namespace std;
using boost::shared_ptr;


//
// *** EventOutputService
//
EventOutputService::EventOutputService(boost::shared_ptr<FileRecord> file, 
                                       InitMsgView const& view)
{
  file_ = file;

  string streamerFileName = file_->completeFileName() + ".dat";
  string indexFileName    = file_->completeFileName() + ".ind";

  writer_ = shared_ptr<StreamerFileWriter> (new StreamerFileWriter(streamerFileName, indexFileName));
  writeHeader(view);

  file_ -> firstEntry(getTimeStamp());
  file_ -> insertFileInDatabase();
}


//
// *** call close file
//
EventOutputService::~EventOutputService()
{
  //std::cout << "EventOutputService Destructor called." << std::endl;
  closeFile();
}


// 
// *** write file header
// *** write increase file size by file header size
//
void EventOutputService::writeHeader(InitMsgView const& view)
{
  writer_ -> doOutputHeader(view);
  file_   -> increaseFileSize(view.size());
}


//
// *** write event to file
// *** increase the file size
// *** update time of last entry 
// *** increase event count
//
void EventOutputService::writeEvent(const uint8 * const bufPtr)
{
  EventMsgView view((void *) bufPtr);
  writer_ -> doOutputEvent(view);
  file_   -> increaseFileSize(view.size());
  file_   -> lastEntry(getTimeStamp());
  file_   -> increaseEventCount();
}


// 
// *** stop file write
// *** add end of file record size to file size
// *** move file to "closed" directory
// *** write to summary catalog
// *** write to mail box
//
void EventOutputService::closeFile()
{
  writer_ -> stop();
  file_   -> increaseFileSize(writer_->getStreamEOFSize());
  file_   -> setadler(writer_->get_adler32_stream(),writer_->get_adler32_index());
  file_   -> moveFileToClosed();
  file_   -> writeToSummaryCatalog();
  file_   -> updateDatabase();
}


//
// *** report status of OutputService
//
void EventOutputService::report(ostream &os, int indentation) const
{
  string prefix(indentation, ' ');
  os << prefix << "------------- EventOutputService -------------\n";
  file_ -> report(os,indentation);
  double time = (double) file_ -> lastEntry() - (double) file_ -> firstEntry();
  double rate = (time>0) ? (double) file_ -> events() / (double) time : 0.; 
  double tput = (time>0) ? (double) file_ -> fileSize() / ((double) time * 1048576.) : 0.; 
  os << prefix << "rate                " << rate            << " evts/s\n";
  os << prefix << "throughput          " << tput            << " MB/s\n";
  os << prefix << "time                " << time            << " s\n";
  os << prefix << "-----------------------------------------\n";  
}
