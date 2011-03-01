// $Id: FileHandler.cc,v 1.26.2.1 2011/02/28 17:56:06 mommsen Exp $
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

namespace stor {
  
  FileHandler::FileHandler
  (
    FilesMonitorCollection::FileRecordPtr fileRecord,
    const DbFileHandlerPtr dbFileHandler,
    const DiskWritingParams& dwParams,
    const uint64_t& maxFileSize
  ):
  fileRecord_(fileRecord),
  dbFileHandler_(dbFileHandler),
  firstEntry_(utils::getCurrentTime()),
  lastEntry_(firstEntry_),
  diskWritingParams_(dwParams),
  maxFileSize_(maxFileSize),
  cmsver_(edm::getReleaseVersion()),
  adler_(0)
  {
    // stripp quotes if present
    if(cmsver_[0]=='"') cmsver_=cmsver_.substr(1,cmsver_.size()-2);
    
    checkDirectories();
    
    insertFileInDatabase();
  }
  
  void FileHandler::writeEvent(const I2OChain& event)
  {
    if ( ! fileRecord_->isOpen )
    {
      std::ostringstream msg;
    msg << "Tried to write an event to "
      << fileRecord_->completeFileName()
      << "which has already been closed.";
    XCEPT_RAISE(stor::exception::DiskWriting, msg.str());
    }
    
    do_writeEvent(event);
    
    fileRecord_->fileSize += event.totalDataSize();
    ++fileRecord_->eventCount;
    lastEntry_ = utils::getCurrentTime();
  }
  
  
  //////////////////////
  // File bookkeeping //
  //////////////////////
  
  void FileHandler::writeToSummaryCatalog() const
  {
    std::ostringstream currentStat;
    std::string ind(":");
    currentStat
      << fileRecord_->filePath() << ind
        << fileRecord_->fileName() << ind
        << fileSize() << ind 
        << events() << ind
        << utils::timeStamp(lastEntry_) << ind
        << (lastEntry_ - firstEntry_).total_seconds() << ind
        << fileRecord_->whyClosed << std::endl;
    std::string currentStatString (currentStat.str());
    std::ofstream of(diskWritingParams_.fileCatalog_.c_str(), std::ios_base::ate | std::ios_base::out | std::ios_base::app );
    of << currentStatString;
    of.close();
  }
  
  
  void FileHandler::updateDatabase() const
  {
    std::ostringstream oss;
    oss << "./closeFile.pl "
      << " --FILENAME "     << fileRecord_->fileName()
      << " --FILECOUNTER "  << fileRecord_->fileCounter
      << " --NEVENTS "      << events()
      << " --FILESIZE "     << fileSize()                          
      << " --STARTTIME "    << utils::secondsSinceEpoch(firstEntry_)
      << " --STOPTIME "     << utils::secondsSinceEpoch(lastEntry_)
      << " --STATUS "       << "closed"
      << " --RUNNUMBER "    << fileRecord_->runNumber
      << " --LUMISECTION "  << fileRecord_->lumiSection
      << " --PATHNAME "     << fileRecord_->filePath()
      << " --HOSTNAME "     << diskWritingParams_.hostName_
      << " --SETUPLABEL "   << diskWritingParams_.setupLabel_
      << " --STREAM "       << fileRecord_->streamLabel                      
      << " --INSTANCE "     << diskWritingParams_.smInstanceString_
      << " --SAFETY "       << diskWritingParams_.initialSafetyLevel_
      << " --APPVERSION "   << cmsver_
      << " --APPNAME CMSSW"
      << " --TYPE streamer"               
      << " --DEBUGCLOSE "   << fileRecord_->whyClosed
      << " --CHECKSUM "     << std::hex << adler_
      << " --CHECKSUMIND "  << std::hex << 0
      << "\n";
    
    dbFileHandler_->writeOld( lastEntry_, oss.str() );
  }
  
  
  void FileHandler::insertFileInDatabase() const
  {
    std::ostringstream oss;
    oss << "./insertFile.pl "
      << " --FILENAME "     << fileRecord_->fileName()
      << " --FILECOUNTER "  << fileRecord_->fileCounter
      << " --NEVENTS "      << events()
      << " --FILESIZE "     << fileSize()
      << " --STARTTIME "    << utils::secondsSinceEpoch(firstEntry_)
      << " --STOPTIME 0"
      << " --STATUS open"
      << " --RUNNUMBER "    << fileRecord_->runNumber
      << " --LUMISECTION "  << fileRecord_->lumiSection
      << " --PATHNAME "     << fileRecord_->filePath()
      << " --HOSTNAME "     << diskWritingParams_.hostName_
      << " --SETUPLABEL "   << diskWritingParams_.setupLabel_
      << " --STREAM "       << fileRecord_->streamLabel
      << " --INSTANCE "     << diskWritingParams_.smInstanceString_
      << " --SAFETY "       << diskWritingParams_.initialSafetyLevel_
      << " --APPVERSION "   << cmsver_
      << " --APPNAME CMSSW"
      << " --TYPE streamer"               
      << " --CHECKSUM 0"
      << " --CHECKSUMIND 0"
      << "\n";
    
    dbFileHandler_->writeOld( firstEntry_, oss.str() );
  }
  
  
  bool FileHandler::tooOld(const utils::TimePoint_t currentTime)
  {
    if (diskWritingParams_.lumiSectionTimeOut_ > boost::posix_time::seconds(0) && 
      (currentTime - lastEntry_) > diskWritingParams_.lumiSectionTimeOut_)
    {
      closeFile(FilesMonitorCollection::FileRecord::timeout);
      return true;
    }
    else
    {
      return false;
    }
  }
  
  
  bool FileHandler::isFromLumiSection(const uint32_t lumiSection)
  {
    if (lumiSection == fileRecord_->lumiSection)
    {
      closeFile(FilesMonitorCollection::FileRecord::LSended);
      return true;
    }
    else
    {
      return false;
    }
  }
  
  
  bool FileHandler::tooLarge(const uint64_t& dataSize)
  {
    if ( ((fileSize() + dataSize) > maxFileSize_) && (events() > 0) )
    {
      closeFile(FilesMonitorCollection::FileRecord::size);
      return true;
    }
    else
    {
      return false;
    }
  }
  
  
  uint32_t FileHandler::events() const
  {
    return fileRecord_->eventCount;
  }
  
  
  uint64_t FileHandler::fileSize() const
  {
    return fileRecord_->fileSize;
  }
  
  
  /////////////////////////////
  // File system interaction //
  /////////////////////////////
  
  
  void FileHandler::moveFileToClosed
  (
    const FilesMonitorCollection::FileRecord::ClosingReason& reason
  )
  {
    const std::string openFileName(fileRecord_->completeFileName(FilesMonitorCollection::FileRecord::open));
    const std::string closedFileName(fileRecord_->completeFileName(FilesMonitorCollection::FileRecord::closed));
    
    const uint64_t openFileSize = checkFileSizeMatch(openFileName, fileSize());
    
    makeFileReadOnly(openFileName);
    try
    {
      renameFile(openFileName, closedFileName);
    }
    catch (stor::exception::DiskWriting& e)
    {
      XCEPT_RETHROW(stor::exception::DiskWriting, 
        "Could not move streamer file to closed area.", e);
    }
    fileRecord_->isOpen = false;
    fileRecord_->whyClosed = reason;
    checkFileSizeMatch(closedFileName, openFileSize);
  }
  
  
  uint64_t FileHandler::checkFileSizeMatch(const std::string& fileName, const uint64_t& size) const
  {
    #if linux
    struct stat64 statBuff;
    int statStatus = stat64(fileName.c_str(), &statBuff);
    #else
    struct stat statBuff;
    int statStatus = stat(fileName.c_str(), &statBuff);
    #endif
    if ( statStatus != 0 )
    {
      fileRecord_->whyClosed = FilesMonitorCollection::FileRecord::inaccessible;
      std::ostringstream msg;
      msg << "Error checking the status of file "
        << fileName
        << ": " << strerror(errno);
      XCEPT_RAISE(stor::exception::DiskWriting, msg.str());
    }
    
    if ( sizeMismatch(size, statBuff.st_size) )
    {
      fileRecord_->whyClosed = FilesMonitorCollection::FileRecord::truncated;
      std::ostringstream msg;
      msg << "Found an unexpected file size when trying to move"
        << " the file to the closed state. File " << fileName
        << " has an actual size of " << statBuff.st_size
        << " (" << statBuff.st_blocks << " blocks)"
        << " instead of the expected size of " << size
        << " (" << (size/512)+1 << " blocks).";
      XCEPT_RAISE(stor::exception::FileTruncation, msg.str());
    }
    
    return static_cast<uint64_t>(statBuff.st_size);
  }
  
  
  bool FileHandler::sizeMismatch(const uint64_t& initialSize, const uint64_t& finalSize) const
  {
    double pctDiff = calcPctDiff(initialSize, finalSize);
    return (pctDiff > diskWritingParams_.fileSizeTolerance_);
  }
  
  
  void FileHandler::makeFileReadOnly(const std::string& fileName) const
  {
    int ronly  = chmod(fileName.c_str(), S_IREAD|S_IRGRP|S_IROTH);
    if (ronly != 0) {
      std::ostringstream msg;
      msg << "Unable to change permissions of " << fileName
        << " to read only: " << strerror(errno);
      XCEPT_RAISE(stor::exception::DiskWriting, msg.str());
    }
  }
  
  
  void FileHandler::renameFile
  (
    const std::string& openFileName,
    const std::string& closedFileName
  ) const
  {
    int result = rename( openFileName.c_str(), closedFileName.c_str() );
    if (result != 0) {
      fileRecord_->whyClosed = FilesMonitorCollection::FileRecord::notClosed;
      std::ostringstream msg;
      msg << "Unable to move " << openFileName << " to "
        << closedFileName << ": " << strerror(errno);
      XCEPT_RAISE(stor::exception::DiskWriting, msg.str());
    }
  }
  
  
  void FileHandler::checkDirectories() const
  {
    utils::checkDirectory(diskWritingParams_.filePath_);
    utils::checkDirectory(fileRecord_->baseFilePath);
    utils::checkDirectory(fileRecord_->baseFilePath + "/open");
    utils::checkDirectory(fileRecord_->baseFilePath + "/closed");
  }
  
  
  double FileHandler::calcPctDiff(const uint64_t& value1, const uint64_t& value2) const
  {
    if (value1 == value2) return 0;
    uint64_t largerValue = std::max(value1,value2);
    uint64_t smallerValue = std::min(value1,value2);
    return ( largerValue > 0 ? (largerValue - smallerValue) / largerValue : 0 );
  }
  
} // namespace stor

/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
