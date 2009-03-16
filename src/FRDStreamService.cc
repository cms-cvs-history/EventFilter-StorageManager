// $Id: FRDStreamService.cc,v 1.4.10.6 2009/03/16 19:05:35 biery Exp $

#include <EventFilter/StorageManager/interface/FRDStreamService.h>
#include "EventFilter/StorageManager/interface/FRDOutputService.h"  

#include <iostream>
#include <iomanip>
#include <sys/stat.h>
#include <sys/statfs.h>

using namespace edm;
using namespace stor;
using namespace std;
using boost::shared_ptr;

//
// *** construct stream service from parameter set
//
FRDStreamService::FRDStreamService(ParameterSet const& pset,
                                   stor::DiskWritingParams dwParams)
{
  parameterSet_ = pset;
  runNumber_ = 0;
  lumiSection_ = 0;
  numberOfFileSystems_ = 0;
  maxSize_ = 0;
  ntotal_ = 0;

  diskWritingParams_ = dwParams;

  setStreamParameter();

  // over-ride the stream-based max size, if needed
  // (this must happen after setStreamParameter()
  if (dwParams._maxFileSize > 0)
    {
      maxSize_ = 1048576 * ((long long) dwParams._maxFileSize);
    }
}


// 
// *** event loop for stream service
//
bool FRDStreamService::nextEvent(const uint8 * const bufPtr)
{
  FRDEventMsgView view((void *) bufPtr);

  // accept all Error events, so no call to any sort of acceptEvents() method...

  runNumber_   = view.run();
  lumiSection_ = 1;  // Error message doesn't yet have lumi section number
                     // *and* we want to keep all Error events in the same
                     // file, for now


  shared_ptr<OutputService> outputService = getOutputService(view);

  outputService->writeEvent(bufPtr);

  return true;
}


//
// *** close all files on stop signal
//
void FRDStreamService::stop()
{
  for (OutputMapIterator it = outputMap_.begin(); it != outputMap_.end(); ) {
    boost::shared_ptr<FileRecord> fd(it->first);
    outputMap_.erase(it++);
    fillOutputSummaryClosed(fd);
  }
}


// 
// *** close all output service when lumiSectionTimeOut seconds have passed
// *** since the most recent event was added
// 
void FRDStreamService::closeTimedOutFiles()
{
  // since we are currently storing all events in a single file,
  // we never close files at lumi section boundaries

  return;
}

//
// *** find output service in map or return a new one
// *** rule: only one file for each lumi section is output map
//
boost::shared_ptr<OutputService> FRDStreamService::getOutputService(FRDEventMsgView const& view)
{
  for (OutputMapIterator it = outputMap_.begin(); it != outputMap_.end(); ++it) {
       if (it->first->lumiSection() == lumiSection_) {
	  if (checkEvent(it->first, view))
	    return it->second;
	  else {
            boost::shared_ptr<FileRecord> fd(it->first);
            outputMap_.erase(it);
            fillOutputSummaryClosed(fd);
            break;
	  }
      }
  }
  return newOutputService();
}


// 
// *** generate file descriptor
// *** generate output service
// *** add ouput service to output map
// *** add ouput service to output summary
//
boost::shared_ptr<OutputService> FRDStreamService::newOutputService()
{
  boost::shared_ptr<FileRecord> file = generateFileRecord();

  shared_ptr<OutputService> outputService(new FRDOutputService(file));
  outputMap_[file] = outputService;

  return outputService;
}


//
// *** perform checks before writing the event
// *** so far ... check the event will fit into the file 
//
bool FRDStreamService::checkEvent(shared_ptr<FileRecord> file, FRDEventMsgView const& view) const
{
  if (file->fileSize() + static_cast<long long>(view.size()) > maxSize_ && file->events() > 0)
    return false;

  return true;
}


//
// *** generate a unique file descriptor
//
//     The run number, stream name and storage manager instance have 
//     to be part of the file name. I have added the lumi section, 
//     but in any case we have to make sure that file names are
//     unique. 
//
//     Keep a list of file names and check if file name 
//     was not already used in this run. 
//
boost::shared_ptr<FileRecord> FRDStreamService::generateFileRecord()
{
  std::ostringstream oss;   
  oss    << diskWritingParams_._setupLabel 
	 << "." << setfill('0') << std::setw(8) << runNumber_ 
	 << "." << setfill('0') << std::setw(4) << lumiSection_
	 << "." << streamLabel_ 
	 << "." << diskWritingParams_._fileName
	 << "." << setfill('0') << std::setw(2) << sourceId_;
  string fileName = oss.str();

  shared_ptr<FileRecord> fd = shared_ptr<FileRecord>(new FileRecord(lumiSection_, fileName, diskWritingParams_));    
  ++ntotal_;

  boost::mutex::scoped_lock sl(list_lock_);
  map<string, int>::iterator it = outputSummary_.find(fileName);
  if(it==outputSummary_.end()) {
     outputSummary_.insert(std::pair<string, int>(fileName,0));
  } else {
     ++it->second;
     fd->setFileCounter(it->second);
  }

  if (numberOfFileSystems_ > 0)
    fd->setFileSystem((runNumber_ + atoi(sourceId_.c_str()) + ntotal_) % numberOfFileSystems_); 
  
  fd->checkDirectories();
  fd->setRunNumber(runNumber_);
  fd->setStreamLabel(streamLabel_);

  // fd->report(cout, 12);
  return fd;
}

//
// *** report the status of stream service
//
void FRDStreamService::report(ostream &os, int indentation) const
{
  string prefix(indentation, ' ');
  os << "\n";
  os << prefix << "------------- FRDStreamService -------------\n";
  os << prefix << "fileName            " << diskWritingParams_._fileName << "\n";
  os << prefix << "filePath            " << diskWritingParams_._filePath << "\n";
  os << prefix << "sourceId            " << sourceId_              << "\n";
  os << prefix << "setupLabel          " << diskWritingParams_._setupLabel << "\n";
  os << prefix << "streamLabel         " << streamLabel_           << "\n";
  os << prefix << "maxSize             " << maxSize_               << "\n";
  os << prefix << "highWaterMark       " << diskWritingParams_._highWaterMark << "\n";
  os << prefix << "lumiSectionTimeOut  " << diskWritingParams_._lumiSectionTimeOut << "\n";
  os << prefix << "no. active files    " << outputMap_.size()      << "\n";
  os << prefix << "no. files           " << outputSummary_.size()  << "\n";
  os << prefix << "-----------------------------------------\n";
  os.flush();
}
