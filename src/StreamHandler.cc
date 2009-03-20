// $Id: StreamHandler.cc,v 1.1.2.3 2009/03/20 15:16:57 mommsen Exp $

#include <sstream>
#include <iomanip>

#include "EventFilter/StorageManager/interface/StreamHandler.h"


using namespace stor;


StreamHandler::StreamHandler(SharedResourcesPtr sharedResources) :
_filesMonitorCollection(sharedResources->_statisticsReporter->getFilesMonitorCollection()),
_diskWritingParams(sharedResources->_configuration->getDiskWritingParams())
{}


StreamHandler::~StreamHandler()
{
  closeAllFiles();
}


void StreamHandler::closeAllFiles()
{
  _fileHandlers.clear();
}


void StreamHandler::closeTimedOutFiles()
{
  for (
    FileHandlers::iterator it = _fileHandlers.begin(), itEnd = _fileHandlers.end();
    it != itEnd;
    ++it
  ) 
  {
    if ( (*it)->tooOld() )
      it = _fileHandlers.erase(it); // Is this correct?
  }
}


void StreamHandler::writeEvent(const I2OChain& event)
{
  FileHandlerPtr handler = getFileHandler(event);
  handler->writeEvent(event);
}


StreamHandler::FileHandlerPtr StreamHandler::getFileHandler(const I2OChain& event)
{
  for (
    FileHandlers::iterator it = _fileHandlers.begin(), itEnd = _fileHandlers.end();
    it != itEnd;
    ++it
  ) 
  {
    if ( (*it)->lumiSection() == event.lumiSection() )
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


FilesMonitorCollection::FileRecordPtr StreamHandler::getNewFileRecord(const I2OChain& event)
{
  FilesMonitorCollection::FileRecordPtr fileRecord =
    _filesMonitorCollection.getNewFileRecord();
  
  fileRecord->runNumber = event.runNumber();
  fileRecord->lumiSection = event.lumiSection();
  fileRecord->streamLabel = streamLabel();
  fileRecord->baseFilePath = getBaseFilePath(event.runNumber());
  fileRecord->coreFileName = getCoreFileName(event.runNumber(), event.lumiSection());
  fileRecord->fileCounter = getFileCounter(fileRecord->coreFileName);
  fileRecord->whyClosed = FilesMonitorCollection::FileRecord::notClosed;
  
  return fileRecord;
}


const std::string StreamHandler::getBaseFilePath(const uint32& runNumber)
{
  return _diskWritingParams._filePath + getFileSystem(runNumber);
}


const std::string StreamHandler::getFileSystem(const uint32& runNumber)
{
  unsigned int fileSystemNumber = 0;

  if (_diskWritingParams._nLogicalDisk > 0)
    fileSystemNumber = (runNumber
      + atoi(_diskWritingParams._smInstanceString.c_str())
      + _fileHandlers.size())
      % _diskWritingParams._nLogicalDisk;

  std::ostringstream fileSystem;
  fileSystem << "/" << std::setfill('0') << std::setw(2) << fileSystemNumber; 

  return fileSystem.str();
}


const std::string StreamHandler::getCoreFileName
(
  const uint32& runNumber,
  const uint32& lumiSection
)
{
  std::ostringstream coreFileName;
  coreFileName << _diskWritingParams._setupLabel
    << "." << std::setfill('0') << std::setw(8) << runNumber
    << "." << std::setfill('0') << std::setw(4) << lumiSection
    << "." << streamLabel()
    << "." << _diskWritingParams._fileName
    << "." << std::setfill('0') << std::setw(2) << _diskWritingParams._smInstanceString;

  return coreFileName.str();
}

 

const unsigned int StreamHandler::getFileCounter(const std::string& coreFileName)
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



const long long StreamHandler::getMaxFileSize()
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
