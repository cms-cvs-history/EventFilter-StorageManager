// $Id: StreamHandler.cc,v 1.17 2010/03/19 17:33:54 mommsen Exp $
/// @file: StreamHandler.cc

#include <sstream>
#include <iomanip>

#include "EventFilter/StorageManager/interface/FileHandler.h"
#include "EventFilter/StorageManager/interface/I2OChain.h"
#include "EventFilter/StorageManager/interface/StreamHandler.h"

#include "boost/bind.hpp"


using namespace stor;


StreamHandler::StreamHandler(
  const SharedResourcesPtr sharedResources,
  const DbFileHandlerPtr dbFileHandler
) :
_statReporter(sharedResources->_statisticsReporter),
_streamRecord(_statReporter->getStreamsMonitorCollection().getNewStreamRecord()),
_diskWritingParams(sharedResources->_configuration->getDiskWritingParams()),
_dbFileHandler(dbFileHandler)
{}


void StreamHandler::closeAllFiles()
{
  std::string errorMsg = "Failed to close all files for stream " + streamLabel() + ": ";
  
  for (FileHandlers::const_iterator it = _fileHandlers.begin(),
         itEnd = _fileHandlers.end(); it != itEnd; ++it)
  {
    try
    {
      (*it)->closeFile(FilesMonitorCollection::FileRecord::runEnded);
    }
    catch(xcept::Exception& e)
    {
      XCEPT_DECLARE_NESTED( stor::exception::DiskWriting,
        sentinelException, errorMsg, e );
      _statReporter->alarmHandler()->
        notifySentinel(AlarmHandler::ERROR, sentinelException);
    }
    catch(std::exception &e)
    {
      errorMsg += e.what();
      XCEPT_DECLARE( stor::exception::DiskWriting,
        sentinelException, errorMsg );
      _statReporter->alarmHandler()->
        notifySentinel(AlarmHandler::ERROR, sentinelException);
    }
    catch(...)
    {
      errorMsg += "Unknown exception";
      XCEPT_DECLARE( stor::exception::DiskWriting,
        sentinelException, errorMsg );
      _statReporter->alarmHandler()->
        notifySentinel(AlarmHandler::ERROR, sentinelException);
    }
  }
  _fileHandlers.clear();
}


void StreamHandler::closeTimedOutFiles(utils::time_point_t currentTime)
{
  _fileHandlers.erase(std::remove_if(_fileHandlers.begin(),
                                     _fileHandlers.end(),
                                     boost::bind(&FileHandler::tooOld,
                                                 _1, currentTime)),
                      _fileHandlers.end());
}

void StreamHandler::closeFilesForLumiSection
(
  const uint32_t& lumiSection,
  std::string& str
)
{
  if ( ! superLumiSectionComplete(lumiSection) ) return;

  const uint32_t superLS = getSuperLumiSection(lumiSection);

  _streamRecord->reportSuperLumiSectionInfo(superLS, str);

  _fileHandlers.erase(std::remove_if(_fileHandlers.begin(),
                                     _fileHandlers.end(),
                                     boost::bind(&FileHandler::isFromSuperLumiSection,
                                                 _1, superLS)),
                      _fileHandlers.end());
}


bool StreamHandler::superLumiSectionComplete(const uint32_t& lumiSection)
{
  const uint32_t superLumiSectionLength = _diskWritingParams._superLumiSectionLength;
  if ( superLumiSectionLength == 1 ) return true;

  const uint32_t superLS = getSuperLumiSection(lumiSection);

  IncompleteSuperLSMap::iterator pos = _incompleteSuperLS.lower_bound(superLS);

  if(pos != _incompleteSuperLS.end() && !(_incompleteSuperLS.key_comp()(superLS, pos->first)))
  {
    // key already exists
    pos->second |= ( 1 << (lumiSection - superLS) );
    if ( pos->second == pow(2,superLumiSectionLength)-1 )
    {
      _incompleteSuperLS.erase(pos);
      return true;
    }
  }
  else
  {
    uint32_t lsComplete = ( 1 << (lumiSection - superLS) );
    _incompleteSuperLS.insert(pos, IncompleteSuperLSMap::value_type(superLS, lsComplete));
  }

  return false;
}


void StreamHandler::writeEvent(const I2OChain& event)
{
  FileHandlerPtr handler = getFileHandler(event);
  handler->writeEvent(event);
  _streamRecord->addSizeInBytes(event.totalDataSize());
  _statReporter->getThroughputMonitorCollection().addDiskWriteSample(event.totalDataSize());
}


StreamHandler::FileHandlerPtr StreamHandler::getFileHandler(const I2OChain& event)
{
  for (
    FileHandlers::iterator it = _fileHandlers.begin(), itEnd = _fileHandlers.end();
    it != itEnd;
    ++it
  ) 
  {
    if ( (*it)->superLumiSection() == getSuperLumiSection( event.lumiSection() ) )
    {
      if ( (*it)->tooLarge(event.totalDataSize()) )
      { 
        _fileHandlers.erase(it);
        break;
      }
      else
      {
        return (*it);
      }
    }
  }    
  return newFileHandler(event);
}


FilesMonitorCollection::FileRecordPtr
StreamHandler::getNewFileRecord(const I2OChain& event)
{
  FilesMonitorCollection::FileRecordPtr fileRecord =
    _statReporter->getFilesMonitorCollection().getNewFileRecord();
  
  fileRecord->runNumber = event.runNumber();
  fileRecord->superLumiSection = getSuperLumiSection( event.lumiSection() );
  fileRecord->streamLabel = streamLabel();
  fileRecord->baseFilePath = getBaseFilePath(fileRecord->runNumber, fileRecord->entryCounter);
  fileRecord->coreFileName = getCoreFileName(fileRecord->runNumber, fileRecord->superLumiSection);
  fileRecord->fileCounter = getFileCounter(fileRecord->coreFileName);
  fileRecord->whyClosed = FilesMonitorCollection::FileRecord::notClosed;
  fileRecord->isOpen = true;

  _streamRecord->incrementFileCount(fileRecord->superLumiSection);

  return fileRecord;
}


uint32_t StreamHandler::getSuperLumiSection(const uint32_t& lumiSection) const
{
  const uint32_t superLumiSectionLength = _diskWritingParams._superLumiSectionLength;
  return ( (lumiSection-1) / superLumiSectionLength * superLumiSectionLength ) + 1;
}


std::string StreamHandler::getBaseFilePath(const uint32& runNumber, uint32_t fileCount) const
{
  return _diskWritingParams._filePath + getFileSystem(runNumber, fileCount);
}


std::string StreamHandler::getFileSystem(const uint32& runNumber, uint32_t fileCount) const
{
  // if the number of logical disks is not specified, don't
  // add a file system subdir to the path
  if (_diskWritingParams._nLogicalDisk <= 0) {return "";}

  unsigned int fileSystemNumber =
    (runNumber
     + atoi(_diskWritingParams._smInstanceString.c_str())
     + fileCount)
    % _diskWritingParams._nLogicalDisk;

  std::ostringstream fileSystem;
  fileSystem << "/" << std::setfill('0') << std::setw(2) << fileSystemNumber; 

  return fileSystem.str();
}


std::string StreamHandler::getCoreFileName
(
  const uint32& runNumber,
  const uint32& superLumiSection
) const
{
  std::ostringstream coreFileName;
  coreFileName << _diskWritingParams._setupLabel
    << "." << std::setfill('0') << std::setw(8) << runNumber
    << "." << std::setfill('0') << std::setw(4) << superLumiSection;
  if ( _diskWritingParams._superLumiSectionLength > 1 )
  {
    coreFileName << "-" << std::setfill('0') << std::setw(4) << 
      superLumiSection+_diskWritingParams._superLumiSectionLength-1;
  }
  coreFileName << "." << streamLabel()
    << "." << _diskWritingParams._fileName
    << "." << std::setfill('0') << std::setw(2) << _diskWritingParams._smInstanceString;

  return coreFileName.str();
}

 

unsigned int StreamHandler::getFileCounter(const std::string& coreFileName)
{
  CoreFileNamesMap::iterator pos = _usedCoreFileNames.find(coreFileName);
  if (pos == _usedCoreFileNames.end())
  {
    _usedCoreFileNames.insert(pos, std::make_pair(coreFileName, 0));
    return 0;
  }
  else
  {
    ++(pos->second);
    return pos->second;
  }
}


unsigned long long StreamHandler::getMaxFileSize() const
{
  int maxFileSizeMB = _diskWritingParams._maxFileSizeMB > 0 ? 
    _diskWritingParams._maxFileSizeMB : getStreamMaxFileSize();

  return ( maxFileSizeMB * 1024 * 1024 );
}


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
