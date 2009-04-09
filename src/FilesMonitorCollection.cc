// $Id: FilesMonitorCollection.cc,v 1.1.2.9 2009/04/09 12:25:26 mommsen Exp $

#include <string>
#include <sstream>
#include <iomanip>

#include "EventFilter/StorageManager/interface/Exception.h"
#include "EventFilter/StorageManager/interface/FilesMonitorCollection.h"

using namespace stor;

FilesMonitorCollection::FilesMonitorCollection(xdaq::Application *app) :
MonitorCollection(app),
_maxFileEntries(250),
_entryCounter(0)
{
  boost::mutex::scoped_lock sl(_fileRecordsMutex);
  _fileRecords.reserve(_maxFileEntries);

  // These infospace items were defined in the old SM
  // _infoSpaceItems.push_back(std::make_pair("closedFiles", &_closedFiles));
  // _infoSpaceItems.push_back(std::make_pair("openFiles", &_openFiles));
  // _infoSpaceItems.push_back(std::make_pair("fileList", &_fileList));
  // _infoSpaceItems.push_back(std::make_pair("eventsInFile", &_eventsInFile));
  // _infoSpaceItems.push_back(std::make_pair("fileSize", &_fileSize));

  putItemsIntoInfoSpace();
}


const FilesMonitorCollection::FileRecordPtr
FilesMonitorCollection::getNewFileRecord()
{
  boost::mutex::scoped_lock sl(_fileRecordsMutex);

  if (_fileRecords.size() >= _maxFileEntries)
  {
    _fileRecords.erase(_fileRecords.begin());
  }

  boost::shared_ptr<FileRecord> fileRecord(new FilesMonitorCollection::FileRecord());
  fileRecord->entryCounter = _entryCounter++;
  fileRecord->fileSize = 0;
  fileRecord->eventCount = 0;
  _fileRecords.push_back(fileRecord);
  return fileRecord;
}


void FilesMonitorCollection::do_calculateStatistics()
{
  // nothing to do
}


void FilesMonitorCollection::do_updateInfoSpace()
{
  // nothing to do
}


void FilesMonitorCollection::do_reset()
{
  boost::mutex::scoped_lock sl(_fileRecordsMutex);
  _fileRecords.clear();
  _entryCounter = 0;
}


std::string FilesMonitorCollection::FileRecord::closingReason()
{
  switch (whyClosed)
  {
    case notClosed:   return "open";
    case stop:        return "run stopped";
    case Nminus2lumi: return "LS changed";
    case timeout:     return "timeout";
    case size:        return "file size";
    default:          return "unknown";
  }
}


std::string FilesMonitorCollection::FileRecord::filePath()
{
  return ( baseFilePath + (whyClosed == notClosed ? "/open/" : "/closed/") );
}


std::string FilesMonitorCollection::FileRecord::fileName()
{
  std::ostringstream fileName;
  fileName << coreFileName 
    << "." << std::setfill('0') << std::setw(4) << fileCounter;
  return fileName.str();
}


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
