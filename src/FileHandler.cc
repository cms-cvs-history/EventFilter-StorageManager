// $Id: FileHandler.cc,v 1.1.2.4 2009/03/18 18:35:41 mommsen Exp $

#include <EventFilter/StorageManager/interface/FileHandler.h>

#include <FWCore/MessageLogger/interface/MessageLogger.h>
#include <FWCore/Utilities/interface/GetReleaseVersion.h>

#include <errno.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <sys/stat.h>

using namespace stor;
using namespace std;


FileHandler::FileHandler
(
  FilesMonitorCollection::FileRecordPtr fileRecord,
  const DiskWritingParams& dwParams
):
_fileRecord(fileRecord),
_firstEntry(utils::getCurrentTime()),
_diskWritingParams(dwParams),
_workingDir("/open/"),
_logPath(dwParams._filePath+"/log"),
_logFile(logFile(dwParams)),
_cmsver(edm::getReleaseVersion())
{
  // stripp quotes if present
  if(_cmsver[0]=='"') _cmsver=_cmsver.substr(1,_cmsver.size()-2);
}


//////////////////////
// File bookkeeping //
//////////////////////

void FileHandler::writeToSummaryCatalog() const
{
  ostringstream currentStat;
  string ind(":");
  currentStat << _fileRecord->filePath                 << ind
              << _fileRecord->fileName                 << ind
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
      << " --FILENAME "     << _fileRecord->fileName <<  ".dat"
    //      << " --FILECOUNTER "  << _fileCounter  Fix me: where to get that from?
      << " --NEVENTS "      << events()
      << " --FILESIZE "     << fileSize()                          
      << " --STARTTIME "    << (int) _firstEntry
      << " --STOPTIME "     << (int) _lastEntry
      << " --STATUS "       << "closed"
      << " --RUNNUMBER "    << _fileRecord->runNumber
      << " --LUMISECTION "  << _fileRecord->lumiSection
      << " --PATHNAME "     << _fileRecord->filePath
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

  ofstream of(_logFile.c_str(), ios_base::ate | ios_base::out | ios_base::app );
  of << oss.str().c_str();
  of.close();
}


void FileHandler::insertFileInDatabase() const
{
  std::ostringstream oss;
  oss << "./insertFile.pl "
      << " --FILENAME "     << _fileRecord->fileName <<  ".dat"
    //      << " --FILECOUNTER "  << _fileCounter  Fix me: where to get that from?
      << " --NEVENTS "      << events()
      << " --FILESIZE "     << fileSize()
      << " --STARTTIME "    << (int) _firstEntry
      << " --STOPTIME "     << (int) _lastEntry
      << " --STATUS "       << "open"
      << " --RUNNUMBER "    << _fileRecord->runNumber
      << " --LUMISECTION "  << _fileRecord->lumiSection
      << " --PATHNAME "     << _fileRecord->filePath
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

  ofstream of(_logFile.c_str(), ios_base::ate | ios_base::out | ios_base::app );
  of << oss.str().c_str();
  of.close();
}



/////////////////////////////
// File system interaction //
/////////////////////////////


void FileHandler::moveFileToClosed(const bool& useIndexFile)
{
  string openIndexFileName      = completeFileName() + ".ind";
  string openStreamerFileName   = completeFileName() + ".dat";

  size_t openStreamerFileSize = checkFileSizeMatch(openStreamerFileName, fileSize());

  makeFileReadOnly(openStreamerFileName);
  if (useIndexFile) makeFileReadOnly(openIndexFileName);

  _workingDir = "/closed/"; // Fix me!
  string closedIndexFileName    = completeFileName() + ".ind";
  string closedStreamerFileName = completeFileName() + ".dat";

  if (useIndexFile) renameFile(openIndexFileName, closedIndexFileName);
  renameFile(openStreamerFileName, closedStreamerFileName);

  checkFileSizeMatch(closedStreamerFileName, openStreamerFileSize);
}


size_t FileHandler::checkFileSizeMatch(const string& fileName, const size_t& size)
{
  struct stat64 statBuff;
  int statStatus = stat64(fileName.c_str(), &statBuff);
  if (statStatus != 0) {
    throw cms::Exception("FileHandler", "moveFileToClosed")
      << "Error checking the status of open file "
      << fileName << std::endl;
  }
  
  if ( sizeMismatch(size, statBuff.st_size) )
    throw cms::Exception("FileHandler", "moveFileToClosed")
      << "Found an unexpected file size when trying to move "
      << "the file to the closed state.  File " << fileName
      << " has an actual size of " << statBuff.st_size
      << " instead of the expected size of " << size << std::endl;

  return statBuff.st_size;
}


bool FileHandler::sizeMismatch(const double& initialSize, const double& finalSize)
{
  if (_diskWritingParams._exactFileSizeTest) {
    if (initialSize != finalSize) {
      return true;
    }
  }
  else {
    double pctDiff = calcPctDiff(initialSize, finalSize);
    if (pctDiff > 0.1) {return true;}
  }
  return false;
}


void FileHandler::makeFileReadOnly(const string& fileName)
{
  int ronly  = chmod(fileName.c_str(), S_IREAD|S_IRGRP|S_IROTH);
  if (ronly != 0) {
    throw cms::Exception("FileHandler", "moveFileToClosed")
      << "Unable to change permissions of " << fileName
      << " to read only." << std::endl;
  }
}


void FileHandler::renameFile(const string& openFileName, const string& closedFileName)
{
  int result = rename( openFileName.c_str(), closedFileName.c_str() );
  if (result != 0) {
    throw cms::Exception("FileHandler", "moveFileToClosed")
      << "Unable to move " << openFileName << " to "
      << closedFileName << ".  Possibly the storage manager "
      << "disk areas are full." << std::endl;
  }
}


const string FileHandler::logFile(const DiskWritingParams& dwp) const
{
  time_t rawtime = time(0);
  tm * ptm;
  ptm = localtime(&rawtime);

  ostringstream logfilename;
  logfilename << _logPath << "/"
              << setfill('0') << std::setw(4) << ptm->tm_year+1900
              << setfill('0') << std::setw(2) << ptm->tm_mon+1
              << setfill('0') << std::setw(2) << ptm->tm_mday
              << "-" << dwp._hostName
              << "-" << dwp._smInstanceString
              << ".log";
  return logfilename.str();
}


void FileHandler::checkDirectories() const
{
  checkDirectory(_diskWritingParams._filePath);
  checkDirectory(_fileRecord->filePath);
  checkDirectory(_fileRecord->filePath + "/open");
  checkDirectory(_fileRecord->filePath + "/closed");
  checkDirectory(_logPath);
}


void FileHandler::checkDirectory(const string& path) const
{
  struct stat64 buf;

  int retVal = stat64(path.c_str(), &buf);
  if(retVal !=0 )
    {
      edm::LogError("StorageManager") << "Directory " << path
				      << " does not exist. Error=" << errno ;
      throw cms::Exception("FileHandler","checkDirectory")
            << "Directory " << path << " does not exist. Error=" << errno << std::endl;
    }
}


const double FileHandler::calcPctDiff(const double& value1, const double& value2) const
{
  if (value1 == value2) {return 0.0;}
  double largerValue = value1;
  double smallerValue = value2;
  if (value1 < value2) {
    largerValue = value2;
    smallerValue = value1;
  }
  return (largerValue - smallerValue) / largerValue;
}



/////////////////////////////
// File information dumper //
/////////////////////////////

void FileHandler::info(ostream& os) const
{
  os // << _fileCounter << " "  Fix me
     << completeFileName() << " " 
     << events() << " "
     << fileSize();
}



/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
