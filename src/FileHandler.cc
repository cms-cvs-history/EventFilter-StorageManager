// $Id: FileHandler.cc,v 1.18 2010/03/31 12:33:53 mommsen Exp $
/// @file: FileHandler.cc

#include <EventFilter/StorageManager/interface/Exception.h>
#include <EventFilter/StorageManager/interface/FileHandler.h>
#include <EventFilter/StorageManager/interface/I2OChain.h>

#include <FWCore/MessageLogger/interface/MessageLogger.h>
#include <FWCore/Version/interface/GetReleaseVersion.h>

#include <errno.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <cstdio>
#include <sys/stat.h>
#include <string.h>

using namespace stor;
using namespace std;


FileHandler::FileHandler
(
  FilesMonitorCollection::FileRecordPtr fileRecord,
  const DbFileHandlerPtr dbFileHandler,
  const DiskWritingParams& dwParams,
  const unsigned long long& maxFileSize
):
_fileRecord(fileRecord),
_dbFileHandler(dbFileHandler),
_firstEntry(utils::getCurrentTime()),
_lastEntry(0),
_diskWritingParams(dwParams),
_maxFileSize(maxFileSize),
_cmsver(edm::getReleaseVersion()),
_adlerstream(0),
_adlerindex(0)
{
  // stripp quotes if present
  if(_cmsver[0]=='"') _cmsver=_cmsver.substr(1,_cmsver.size()-2);

  checkDirectories();

  insertFileInDatabase();
}

void FileHandler::writeEvent(const I2OChain& event)
{
  if ( ! _fileRecord->isOpen )
  {
    std::ostringstream msg;
    msg << "Tried to write an event to "
      << _fileRecord->completeFileName()
      << "which has already been closed.";
    XCEPT_RAISE(stor::exception::DiskWriting, msg.str());
  }

  do_writeEvent(event);

  _fileRecord->fileSize += event.totalDataSize();
  ++_fileRecord->eventCount;
  _lastEntry = utils::getCurrentTime();
}


//////////////////////
// File bookkeeping //
//////////////////////

void FileHandler::writeToSummaryCatalog() const
{
  ostringstream currentStat;
  string ind(":");
  currentStat << _fileRecord->filePath()               << ind
              << _fileRecord->fileName()               << ind
              << fileSize()                            << ind 
              << events()                              << ind
              << utils::timeStamp(_lastEntry)          << ind
              << (int) (_lastEntry - _firstEntry)      << ind
              << _fileRecord->whyClosed << endl;
  string currentStatString (currentStat.str());
  ofstream of(_diskWritingParams._fileCatalog.c_str(), ios_base::ate | ios_base::out | ios_base::app );
  of << currentStatString;
  of.close();
}


void FileHandler::updateDatabase() const
{
  std::ostringstream oss;
  oss << "./closeFile.pl "
      << " --FILENAME "     << _fileRecord->fileName() <<  ".dat"
      << " --FILECOUNTER "  << _fileRecord->fileCounter
      << " --NEVENTS "      << events()
      << " --FILESIZE "     << fileSize()                          
      << " --STARTTIME "    << static_cast<int>(_firstEntry)
      << " --STOPTIME "     << static_cast<int>(_lastEntry)
      << " --STATUS "       << "closed"
      << " --RUNNUMBER "    << _fileRecord->runNumber
      << " --LUMISECTION "  << _fileRecord->superLumiSection
      << " --PATHNAME "     << _fileRecord->filePath()
      << " --HOSTNAME "     << _diskWritingParams._hostName
      << " --SETUPLABEL "   << _diskWritingParams._setupLabel
      << " --STREAM "       << _fileRecord->streamLabel                      
      << " --INSTANCE "     << _diskWritingParams._smInstanceString
      << " --SAFETY "       << _diskWritingParams._initialSafetyLevel
      << " --APPVERSION "   << _cmsver
      << " --APPNAME CMSSW"
      << " --TYPE streamer"               
      << " --DEBUGCLOSE "   << _fileRecord->whyClosed
      << " --CHECKSUM "     << hex << _adlerstream
      << " --CHECKSUMIND "  << hex << _adlerindex
      << "\n";

  _dbFileHandler->writeOld( _lastEntry, oss.str() );
}


void FileHandler::insertFileInDatabase() const
{
  std::ostringstream oss;
  oss << "./insertFile.pl "
      << " --FILENAME "     << _fileRecord->fileName() <<  ".dat"
      << " --FILECOUNTER "  << _fileRecord->fileCounter
      << " --NEVENTS "      << events()
      << " --FILESIZE "     << fileSize()
      << " --STARTTIME "    << static_cast<int>(_firstEntry)
      << " --STOPTIME 0"
      << " --STATUS open"
      << " --RUNNUMBER "    << _fileRecord->runNumber
      << " --LUMISECTION "  << _fileRecord->superLumiSection
      << " --PATHNAME "     << _fileRecord->filePath()
      << " --HOSTNAME "     << _diskWritingParams._hostName
      << " --SETUPLABEL "   << _diskWritingParams._setupLabel
      << " --STREAM "       << _fileRecord->streamLabel
      << " --INSTANCE "     << _diskWritingParams._smInstanceString
      << " --SAFETY "       << _diskWritingParams._initialSafetyLevel
      << " --APPVERSION "   << _cmsver
      << " --APPNAME CMSSW"
      << " --TYPE streamer"               
      << " --CHECKSUM 0"
      << " --CHECKSUMIND 0"
      << "\n";

  _dbFileHandler->writeOld( _firstEntry, oss.str() );
}


bool FileHandler::tooLarge(const unsigned long& dataSize)
{
  if ( ((fileSize() + dataSize) > _maxFileSize) && (events() > 0) )
  {
    closeFile(FilesMonitorCollection::FileRecord::size);
    return true;
  }
  else
  {
    return false;
  }
}


int FileHandler::events() const
{
  return _fileRecord->eventCount;
}


const unsigned long long FileHandler::fileSize() const
{
  return _fileRecord->fileSize;
}


/////////////////////////////
// File system interaction //
/////////////////////////////


void FileHandler::moveFileToClosed
(
  const bool& useIndexFile,
  const FilesMonitorCollection::FileRecord::ClosingReason& reason
)
{
  const string openFileName(_fileRecord->completeFileName(FilesMonitorCollection::FileRecord::open));
  const string openIndexFileName(openFileName + ".ind");
  const string openStreamerFileName(openFileName + ".dat");

  const string closedFileName(_fileRecord->completeFileName(FilesMonitorCollection::FileRecord::closed));
  const string closedIndexFileName(closedFileName + ".ind");
  const string closedStreamerFileName(closedFileName + ".dat");

  size_t openStreamerFileSize = checkFileSizeMatch(openStreamerFileName, fileSize());

  makeFileReadOnly(openStreamerFileName);
  if (useIndexFile) makeFileReadOnly(openIndexFileName);

  if (useIndexFile) renameFile(openIndexFileName, closedIndexFileName);
  try
  {
    renameFile(openStreamerFileName, closedStreamerFileName);
  }
  catch (stor::exception::DiskWriting& e)
  {
    // Rename failed. Move index file back to open location
    if (useIndexFile) renameFile(closedIndexFileName, openIndexFileName);
    XCEPT_RETHROW(stor::exception::DiskWriting, 
      "Could not move streamer file to closed area.", e);
  }
  _fileRecord->isOpen = false;
  _fileRecord->whyClosed = reason;
  checkFileSizeMatch(closedStreamerFileName, openStreamerFileSize);
}


size_t FileHandler::checkFileSizeMatch(const string& fileName, const size_t& size) const
{
  struct stat64 statBuff;
  int statStatus = stat64(fileName.c_str(), &statBuff);
  if ( statStatus != 0 )
  {
    _fileRecord->whyClosed = FilesMonitorCollection::FileRecord::inaccessible;
    std::ostringstream msg;
    msg << "Error checking the status of file "
      << fileName
      << ": " << strerror(errno);
    XCEPT_RAISE(stor::exception::DiskWriting, msg.str());
  }
  
  if ( sizeMismatch(size, statBuff.st_size) )
  {
    _fileRecord->whyClosed = FilesMonitorCollection::FileRecord::truncated;
    std::ostringstream msg;
    msg << "Found an unexpected file size when trying to move"
      << " the file to the closed state. File " << fileName
      << " has an actual size of " << statBuff.st_size
      << " (" << statBuff.st_blocks << " blocks)"
      << " instead of the expected size of " << size
      << " (" << (size/512)+1 << " blocks).";
    XCEPT_RAISE(stor::exception::FileTruncation, msg.str());
  }

  return statBuff.st_size;
}


bool FileHandler::sizeMismatch(const double& initialSize, const double& finalSize) const
{
  double pctDiff = calcPctDiff(initialSize, finalSize);
  return (pctDiff > _diskWritingParams._fileSizeTolerance);
}


void FileHandler::makeFileReadOnly(const string& fileName) const
{
  int ronly  = chmod(fileName.c_str(), S_IREAD|S_IRGRP|S_IROTH);
  if (ronly != 0) {
    std::ostringstream msg;
    msg << "Unable to change permissions of " << fileName
      << " to read only: " << strerror(errno);
    XCEPT_RAISE(stor::exception::DiskWriting, msg.str());
  }
}


void FileHandler::renameFile(const string& openFileName, const string& closedFileName) const
{
  int result = rename( openFileName.c_str(), closedFileName.c_str() );
  if (result != 0) {
    _fileRecord->whyClosed = FilesMonitorCollection::FileRecord::notClosed;
    std::ostringstream msg;
    msg << "Unable to move " << openFileName << " to "
      << closedFileName << ": " << strerror(errno);
    XCEPT_RAISE(stor::exception::DiskWriting, msg.str());
  }
}


void FileHandler::checkDirectories() const
{
  utils::checkDirectory(_diskWritingParams._filePath);
  utils::checkDirectory(_fileRecord->baseFilePath);
  utils::checkDirectory(_fileRecord->baseFilePath + "/open");
  utils::checkDirectory(_fileRecord->baseFilePath + "/closed");
}


double FileHandler::calcPctDiff(const double& value1, const double& value2) const
{
  if (value1 == value2) return 0;
  double largerValue = std::max(value1,value2);
  double smallerValue = std::min(value1,value2);
  return ( largerValue > 0 ? (largerValue - smallerValue) / largerValue : 0 );
}


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
