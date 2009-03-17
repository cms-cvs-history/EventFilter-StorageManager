// $Id: EventStreamService.cc,v 1.9.10.7 2009/03/16 19:21:39 biery Exp $

#include <EventFilter/StorageManager/interface/EventStreamService.h>
#include "EventFilter/StorageManager/interface/EventOutputService.h"  

#include <iostream>
#include <iomanip>
#include <sys/stat.h>
#include <sys/statfs.h>

using namespace edm;
using namespace std;
using namespace stor;
using boost::shared_ptr;

//
// *** construct stream service from 
// *** parameter set and init message
//
EventStreamService::EventStreamService(ParameterSet const& pset,
                                       InitMsgView const& view,
                                       stor::DiskWritingParams dwParams)
{
  parameterSet_ = pset;
  runNumber_ = 0;
  lumiSection_ = 0;
  maxSize_ = 0;
  ntotal_ = 0;

  diskWritingParams_ = dwParams;

  saveInitMessage(view);
  initializeSelection(view);
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
bool EventStreamService::nextEvent(const uint8 * const bufPtr)
{
  EventMsgView view((void *) bufPtr);
  if (!acceptEvent(view))
    {
      return false;
    }
  runNumber_   = view.run();
  lumiSection_ = view.lumi();

  shared_ptr<OutputService> outputService = getOutputService(view);
  
  outputService->writeEvent(bufPtr);
  return true;
}


//
// *** close all files on stop signal
//
void EventStreamService::stop()
{
  for (OutputMapIterator it = outputMap_.begin(); it != outputMap_.end(); ) {
    boost::shared_ptr<FileRecord> fd(it->first);
    fd->setWhyClosed(FileRecord::stop);
    outputMap_.erase(it++);
    fillOutputSummaryClosed(fd);
  }
}

// 
// *** close all output service when lumiSectionTimeOut seconds have passed
// *** since the most recent event was added
// 
void EventStreamService::closeTimedOutFiles()
{
  double currentTime = getCurrentTime();
  for (OutputMapIterator it = outputMap_.begin(); it != outputMap_.end(); ) {
     if (currentTime - it->second->lastEntry() > diskWritingParams_._lumiSectionTimeOut) {
        boost::shared_ptr<FileRecord> fd(it->first);
        outputMap_.erase(it++);
        fillOutputSummaryClosed(fd);
     } else {
        ++it;
     }
  }
}


//
// *** find output service in map or return a new one
// *** rule: only one file for each lumi section is output map
//
boost::shared_ptr<OutputService> EventStreamService::getOutputService(EventMsgView const& view)
{
  for (OutputMapIterator it = outputMap_.begin(); it != outputMap_.end(); ++it) {
       if (it->first->lumiSection() == lumiSection_) {
	  if (checkEvent(it->first, view))
	    return it->second;
	  else { // close file since file size exceeded
            boost::shared_ptr<FileRecord> fd(it->first);
            fd->setWhyClosed(FileRecord::size);
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
boost::shared_ptr<OutputService> EventStreamService::newOutputService()
{
  boost::shared_ptr<FileRecord> file = generateFileRecord();
  InitMsgView view(&saved_initmsg_[0]);

  shared_ptr<OutputService> outputService(new EventOutputService(file, view));
  outputMap_[file] = outputService;

  return outputService;
}


//
// *** perform checks before writing the event
// *** so far ... check the event will fit into the file 
//
bool EventStreamService::checkEvent(shared_ptr<FileRecord> file, EventMsgView const& view) const
{
  if (file->fileSize() + static_cast<long long>(view.size()) > maxSize_ && file->events() > 0)
    return false;

  return true;
}


//
// *** initialize stream selection
// 
void EventStreamService::initializeSelection(InitMsgView const& initView)
{
  Strings triggerNameList;
  initView.hltTriggerNames(triggerNameList);
  eventSelector_.reset(new EventSelector(parameterSet_.getUntrackedParameter("SelectEvents", ParameterSet()),triggerNameList));
}


//
// *** accept event according to their high level trigger bits
//
bool EventStreamService::acceptEvent(EventMsgView const& view) 
{
  std::vector<unsigned char> hlt_out;
  hlt_out.resize(1 + (view.hltCount()-1)/4);
  view.hltTriggerBits(&hlt_out[0]);
  int num_paths = view.hltCount();
  bool rc = (eventSelector_->wantAll() || eventSelector_->acceptEvent(&hlt_out[0], num_paths));
  return rc;
}


//
// *** save init message need to open new output service
//
void EventStreamService::saveInitMessage(InitMsgView const& view)
{
  saved_initmsg_.resize(view.size() + 20);
  unsigned char* pos  = &saved_initmsg_[0];
  unsigned char* from = view.startAddress();
  unsigned int dsize  = view.size();
  copy(from,from+dsize,pos);
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
boost::shared_ptr<FileRecord> EventStreamService::generateFileRecord()
{
  std::ostringstream oss;   
  oss    << diskWritingParams_._setupLabel
	 << "." << setfill('0') << std::setw(8) << runNumber_ 
	 << "." << setfill('0') << std::setw(4) << lumiSection_
	 << "." << streamLabel_ 
	 << "." << diskWritingParams_._fileName
	 << "." << setfill('0') << std::setw(2) << diskWritingParams_._smInstanceString;
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

  if (diskWritingParams_._nLogicalDisk > 0)
    fd->setFileSystem((runNumber_ + atoi(diskWritingParams_._smInstanceString.c_str()) + ntotal_) % diskWritingParams_._nLogicalDisk); 
  
  fd->checkDirectories();
  fd->setRunNumber(runNumber_);
  fd->setStreamLabel(streamLabel_);

  // fd->report(cout, 12);
  return fd;
}


//
// *** report the status of stream service
//
void EventStreamService::report(ostream &os, int indentation) const
{
  string prefix(indentation, ' ');
  os << "\n";
  os << prefix << "------------- EventStreamService -------------\n";
  os << prefix << "fileName            " << diskWritingParams_._fileName << "\n";
  os << prefix << "filePath            " << diskWritingParams_._filePath << "\n";
  os << prefix << "sourceId            " << diskWritingParams_._smInstanceString << "\n";
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
