// $Id: FilesMonitorCollection.cc,v 1.1.2.4 2009/04/03 13:36:37 mommsen Exp $

#include <string>
#include <sstream>
#include <iomanip>

#include "EventFilter/StorageManager/interface/Exception.h"
#include "EventFilter/StorageManager/interface/FilesMonitorCollection.h"

using namespace stor;

FilesMonitorCollection::FilesMonitorCollection(xdaq::Application *app) :
MonitorCollection(app, "Files"),
_maxFileEntries(250),
_entryCounter(0)
{
  _fileRecords.reserve(_maxFileEntries);

  putItemsIntoInfoSpace();
}


FilesMonitorCollection::FileRecordPtr
FilesMonitorCollection::getNewFileRecord(double timeWindowForRecentResults)
{
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
  std::string errorMsg =
    "Failed to update values of items in info space " + _infoSpace->name();

  // Lock the infospace to assure that all items are consistent
  try
  {
    _infoSpace->lock();

    _infoSpace->unlock();
  }
  catch(std::exception &e)
  {
    _infoSpace->unlock();
 
    errorMsg += ": ";
    errorMsg += e.what();
    XCEPT_RAISE(stor::exception::Monitoring, errorMsg);
  }
  catch (...)
  {
    _infoSpace->unlock();
 
    errorMsg += " : unknown exception";
    XCEPT_RAISE(stor::exception::Monitoring, errorMsg);
  }

  try
  {
    // The fireItemGroupChanged locks the infospace
    _infoSpace->fireItemGroupChanged(_infoSpaceItemNames, this);
  }
  catch (xdata::exception::Exception &e)
  {
    XCEPT_RETHROW(stor::exception::Infospace, errorMsg, e);
  }
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
