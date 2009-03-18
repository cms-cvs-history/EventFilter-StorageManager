// $Id: FilesMonitorCollection.cc,v 1.1.2.7 2009/03/18 09:13:05 mommsen Exp $

#include <string>
#include <sstream>
#include <iomanip>

#include "EventFilter/StorageManager/interface/Exception.h"
#include "EventFilter/StorageManager/interface/FilesMonitorCollection.h"

using namespace stor;

FilesMonitorCollection::FilesMonitorCollection(xdaq::Application *app) :
MonitorCollection(app, "Files")
{

  putItemsIntoInfoSpace();
}


FilesMonitorCollection::FileRecordPtr
FilesMonitorCollection::getNewFileRecord(double timeWindowForRecentResults)
{
  boost::shared_ptr<FileRecord> fileRecord;
  _fileRecords.push_back(fileRecord);
  return fileRecord;
}


void FilesMonitorCollection::do_calculateStatistics()
{
  for (
    FileRecordList::iterator it = _fileRecords.begin(), itEnd = _fileRecords.end();
    it != itEnd;
    ++it
  ) 
  {
    if ( (*it)->whyClosed == FileRecord::notClosed )
      (*it)->fileSize.calculateStatistics();
  }
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




/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
