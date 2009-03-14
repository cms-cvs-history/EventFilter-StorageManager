// $Id: StreamService.cc,v 1.15.10.3 2009/03/13 21:23:53 biery Exp $

#include <EventFilter/StorageManager/interface/StreamService.h>
#include <EventFilter/StorageManager/interface/Parameter.h>
#include "EventFilter/StorageManager/interface/Configurator.h"

#include <iostream>
#include <iomanip>
#include <sys/time.h> 
#include <sys/stat.h>
#include <sys/statfs.h>

using namespace edm;
using namespace stor;
using namespace std;
using boost::shared_ptr;

//
// *** destructor
//
StreamService::~StreamService()
{
}


//
// *** file private data member from parameter set
//
void StreamService::setStreamParameter()
{
  // some parameters common to streams are given in the XML file
  // these are defaults, actually set at configure time

  // 02-Sep-2008, KAB:  NOTE that most, if not all, of these parameters are
  // overwritten with either defaults from stor::Parameter or values set 
  // in the SM configuration (confdb/online or xml/offline).
  // The overwrite from the Parameter class happens in ServiceManager::manageInitMsg.

  streamLabel_        = parameterSet_.getParameter<string> ("streamLabel");
  maxSize_ = 1048576 * (long long) parameterSet_.getParameter<int> ("maxSize");
  highWaterMark_      = 0.9;// set by setHighWaterMark
  lumiSectionTimeOut_ = 45; // set by setLumiSectionTimeOut
  sourceId_           = ""; // set by setSourceId
  // report(cout, 4);
}


//
// *** get all files in this run (including open files)
// return for each the count, filename, number of events, file size separated by a space
//
std::list<std::string> StreamService::getFileList()
{
  boost::mutex::scoped_lock sl(list_lock_);

  std::list<std::string> files_=outputSummaryClosed_;
  for (OutputMapIterator it = outputMap_.begin() ; it != outputMap_.end(); ++it) {
    std::ostringstream entry;
    it->first->info(entry);
    files_.push_back(entry.str());
  }

  return files_;
}

//
// *** Copy file string from OutputMap into OutputMapClosed
//
void StreamService::fillOutputSummaryClosed(const boost::shared_ptr<FileRecord> &file)
{
  boost::mutex::scoped_lock sl(list_lock_);

  std::ostringstream entry;
  file->info(entry);
  outputSummaryClosed_.push_back(entry.str());
}


//
// *** get all open (current) files
//
std::list<std::string> StreamService::getCurrentFileList()
{
  std::list<std::string> files_;
  for (OutputMapIterator it = outputMap_.begin(), itEnd = outputMap_.end(); it != itEnd; ++it) {
    files_.push_back(it->first->completeFileName());
  }
  return files_;
}

//
// *** get the current time
//
double StreamService::getCurrentTime() const
{
  struct timeval now;
  struct timezone dummyTZ;
  gettimeofday(&now, &dummyTZ);
  return ((double) now.tv_sec + ((double) now.tv_usec / 1000000.0));
}
