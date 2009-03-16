// $Id: FileHandler.cc,v 1.1.2.1 2009/03/16 10:46:50 mommsen Exp $

#include <EventFilter/StorageManager/interface/FileHandler.h>
#include <EventFilter/StorageManager/interface/Configurator.h>
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


FileHandler::FileHandler(const uint32_t lumiSection, const string &file, DiskWritingParams dwParams):
_smParameter(stor::Configurator::instance()->getParameter()),
_diskWritingParams(dwParams),
_fileName(file),
_basePath(dwParams._filePath),
_fileSystem(""),
_workingDir("/open/"),
_logPath(dwParams._filePath+"/log"),
_logFile(logFile(dwParams)),
_streamLabel(""),
_cmsver(edm::getReleaseVersion()),
_lumiSection(lumiSection),
_runNumber(0),
_fileCounter(0),
_fileSize(0), 
_events(0), 
_firstEntry(0.0), 
_lastEntry(0.0),
_whyClosed(notClosed)
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
  currentStat << _basePath + _fileSystem + _workingDir << ind
              << qualifiedFileName()                   << ind
	      << fileSize()                            << ind 
	      << events()                              << ind
              << utils::timeStamp(lastEntry())         << ind
	      << (int) (lastEntry()-firstEntry())      << ind
	      << _whyClosed << endl;
  string currentStatString (currentStat.str());
  ofstream of(_diskWritingParams._fileCatalog.c_str(), ios_base::ate | ios_base::out | ios_base::app );
  of << currentStatString;
  of.close();
}


void FileHandler::updateDatabase() const
{
  std::ostringstream oss;
  oss << "./closeFile.pl "
      << " --FILENAME "     << qualifiedFileName() <<  ".dat"
      << " --FILECOUNTER "  << _fileCounter                       
      << " --NEVENTS "      << _events                            
      << " --FILESIZE "     << _fileSize                          
      << " --STARTTIME "    << (int) firstEntry()
      << " --STOPTIME "     << (int) lastEntry()
      << " --STATUS "       << "closed"
      << " --RUNNUMBER "    << _runNumber                         
      << " --LUMISECTION "  << _lumiSection                      
      << " --PATHNAME "     << filePath()
      << " --HOSTNAME "     << _diskWritingParams._hostName
      << " --SETUPLABEL "   << _diskWritingParams._setupLabel
      << " --STREAM "       << _streamLabel                      
      << " --INSTANCE "     << _diskWritingParams._smInstanceString
      << " --SAFETY "       << _diskWritingParams._initialSafetyLevel
      << " --APPVERSION "   << _cmsver
      << " --APPNAME CMSSW"
      << " --TYPE streamer"               
      << " --DEBUGCLOSE "   << _whyClosed
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
      << " --FILENAME "     << qualifiedFileName() <<  ".dat"
      << " --FILECOUNTER "  << _fileCounter                       
      << " --NEVENTS "      << _events                            
      << " --FILESIZE "     << _fileSize                          
      << " --STARTTIME "    << (int) firstEntry()
      << " --STOPTIME "     << (int) lastEntry()
      << " --STATUS "       << "open"
      << " --RUNNUMBER "    << _runNumber                         
      << " --LUMISECTION "  << _lumiSection                      
      << " --PATHNAME "     << filePath()
      << " --HOSTNAME "     << _diskWritingParams._hostName
      << " --SETUPLABEL "   << _diskWritingParams._setupLabel
      << " --STREAM "       << _streamLabel                      
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



////////////////////////////
// File parameter setters //
////////////////////////////

void FileHandler::setFileSystem(const unsigned int i)
{
  std::ostringstream oss;
  oss << "/" << setfill('0') << std::setw(2) << i; 
  _fileSystem = oss.str();
}


////////////////////////////
// File parameter getters //
////////////////////////////

const string FileHandler::filePath() const
{
  return ( _basePath + _fileSystem + _workingDir);
}


const string FileHandler::completeFileName() const
{
  return ( filePath() + qualifiedFileName() );
}


const string FileHandler::qualifiedFileName() const
{
  std::ostringstream oss;
  oss << _fileName;
  oss << "." << setfill('0') << std::setw(4) << _fileCounter;
  return oss.str();
}



/////////////////////////////
// File system interaction //
/////////////////////////////

void FileHandler::moveFileToClosed()
{
  struct stat64 initialStatBuff, finalStatBuff;
  int statStatus;
  double pctDiff;
  bool sizeMismatch;
  
  string openIndexFileName      = completeFileName() + ".ind";
  string openStreamerFileName   = completeFileName() + ".dat";
  statStatus = stat64(openStreamerFileName.c_str(), &initialStatBuff);
  if (statStatus != 0) {
    throw cms::Exception("FileHandler", "moveFileToClosed")
      << "Error checking the status of open file "
      << openStreamerFileName << ".  Has the file moved unexpectedly?"
      << std::endl;
  }
  sizeMismatch = false;
  if (_diskWritingParams._exactFileSizeTest) {
    if (_fileSize != initialStatBuff.st_size) {
      sizeMismatch = true;
    }
  }
  else {
    pctDiff = calcPctDiff(_fileSize, initialStatBuff.st_size);
    if (pctDiff > 0.1) {sizeMismatch = true;}
  }
  if (sizeMismatch) {
    throw cms::Exception("FileHandler", "moveFileToClosed")
      << "Found an unexpected open file size when trying to move "
      << "the file to the closed state.  File " << openStreamerFileName
      << " has an actual size of " << initialStatBuff.st_size
      << " instead of the expected size of " << _fileSize << std::endl;
  }

  int ronly  = chmod(openStreamerFileName.c_str(), S_IREAD|S_IRGRP|S_IROTH);
  ronly     += chmod(openIndexFileName.c_str(), S_IREAD|S_IRGRP|S_IROTH);
  if (ronly != 0) {
    throw cms::Exception("FileHandler", "moveFileToClosed")
      << "Unable to change permissions of " << openStreamerFileName << " and "
      << openIndexFileName << " to read only." << std::endl;
  }

  _workingDir = "/closed/";
  string closedIndexFileName    = completeFileName() + ".ind";
  string closedStreamerFileName = completeFileName() + ".dat";

  int result = rename( openIndexFileName.c_str()    , closedIndexFileName.c_str() );
  result    += rename( openStreamerFileName.c_str() , closedStreamerFileName.c_str() );
  if (result != 0) {
    throw cms::Exception("FileHandler", "moveFileToClosed")
      << "Unable to move " << openStreamerFileName << " to "
      << closedStreamerFileName << ".  Possibly the storage manager "
      << "disk areas are full." << std::endl;
  }

  statStatus = stat64(closedStreamerFileName.c_str(), &finalStatBuff);
  if (statStatus != 0) {
    throw cms::Exception("FileHandler", "moveFileToClosed")
      << "Error checking the status of closed file "
      << closedStreamerFileName << ".  This file was copied from "
      << openStreamerFileName << ", and the copy seems to have failed."
      << std::endl;
  }
  sizeMismatch = false;
  if (_diskWritingParams._exactFileSizeTest) {
    if (initialStatBuff.st_size != finalStatBuff.st_size) {
      sizeMismatch = true;
    }
  }
  else {
    pctDiff = calcPctDiff(initialStatBuff.st_size, finalStatBuff.st_size);
    if (pctDiff > 0.1) {sizeMismatch = true;}
  }
  if (sizeMismatch) {
    throw cms::Exception("FileHandler", "moveFileToClosed")
      << "Error moving " << openStreamerFileName << " to "
      << closedStreamerFileName << ".  The closed file size ("
      << finalStatBuff.st_size << ") is different than the open file size ("
      << initialStatBuff.st_size << ").  Possibly the storage manager "
      << "disk areas are full." << std::endl;
  }
}


void FileHandler::moveErrorFileToClosed()
{
  struct stat64 initialStatBuff, finalStatBuff;
  int statStatus;
  double pctDiff;
  bool sizeMismatch;

  string openErrorFileName   = completeFileName() + ".dat";
  statStatus = stat64(openErrorFileName.c_str(), &initialStatBuff);
  if (statStatus != 0) {
    throw cms::Exception("FileHandler", "moveErrorFileToClosed")
      << "Error checking the status of open error file "
      << openErrorFileName << ".  Has the file moved unexpectedly?"
      << std::endl;
  }
  sizeMismatch = false;
  if (_diskWritingParams._exactFileSizeTest) {
    if (_fileSize != initialStatBuff.st_size) {
      sizeMismatch = true;
    }
  }
  else {
    pctDiff = calcPctDiff(_fileSize, initialStatBuff.st_size);
    if (pctDiff > 0.1) {sizeMismatch = true;}
  }
  if (sizeMismatch) {
    throw cms::Exception("FileHandler", "moveErrorFileToClosed")
      << "Found an unexpected open file size when trying to move "
      << "the file to the closed state.  File " << openErrorFileName
      << " has an actual size of " << initialStatBuff.st_size
      << " instead of the expected size of " << _fileSize << std::endl;
  }

  int ronly  = chmod(openErrorFileName.c_str(), S_IREAD|S_IRGRP|S_IROTH);
  if (ronly != 0) {
    throw cms::Exception("FileHandler", "moveErrorFileToClosed")
      << "Unable to change permissions of " << openErrorFileName
      << " to read only." << std::endl;
  }

  _workingDir = "/closed/";
  string closedErrorFileName = completeFileName() + ".dat";

  int result = rename( openErrorFileName.c_str() , closedErrorFileName.c_str() );
  if (result != 0) {
    throw cms::Exception("FileHandler", "moveErrorFileToClosed")
      << "Unable to move " << openErrorFileName << " to "
      << closedErrorFileName << ".  Possibly the storage manager "
      << "disk areas are full." << std::endl;
  }

  statStatus = stat64(closedErrorFileName.c_str(), &finalStatBuff);
  if (statStatus != 0) {
    throw cms::Exception("FileHandler", "moveErrorFileToClosed")
      << "Error checking the status of closed file "
      << closedErrorFileName << ".  This file was copied from "
      << openErrorFileName << ", and the copy seems to have failed."
      << std::endl;
  }
  sizeMismatch = false;
  if (_diskWritingParams._exactFileSizeTest) {
    if (initialStatBuff.st_size != finalStatBuff.st_size) {
      sizeMismatch = true;
    }
  }
  else {
    pctDiff = calcPctDiff(initialStatBuff.st_size, finalStatBuff.st_size);
    if (pctDiff > 0.1) {sizeMismatch = true;}
  }
  if (sizeMismatch) {
    throw cms::Exception("FileHandler", "moveErrorFileToClosed")
      << "Error moving " << openErrorFileName << " to "
      << closedErrorFileName << ".  The closed file size ("
      << finalStatBuff.st_size << ") is different than the open file size ("
      << initialStatBuff.st_size << ").  Possibly the storage manager "
      << "disk areas are full." << std::endl;
  }
}


const string FileHandler::logFile(stor::DiskWritingParams const& dwp) const
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
  checkDirectory(_basePath);
  checkDirectory(_basePath + _fileSystem);
  checkDirectory(_basePath + _fileSystem + "/open");
  checkDirectory(_basePath + _fileSystem + "/closed");
  checkDirectory(_logPath);
}


void FileHandler::checkDirectory(const string &path) const
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


const double FileHandler::calcPctDiff(long long value1, long long value2) const
{
  if (value1 == value2) {return 0.0;}
  long long largerValue = value1;
  long long smallerValue = value2;
  if (value1 < value2) {
    largerValue = value2;
    smallerValue = value1;
  }
  return ((double) largerValue - (double) smallerValue) / (double) largerValue;
}



/////////////////////////////
// File information dumper //
/////////////////////////////

void FileHandler::info(ostream &os) const
{
  os << _fileCounter << " " 
     << completeFileName() << " " 
     << _events << " "
     << _fileSize;
}


// void FileHandler::report(ostream &os, int indentation) const
// {
//   string prefix(indentation, ' ');
//   os << "\n";
//   os << prefix << "------------- FileHandler -------------\n";
//   os << prefix << "fileName            " << _fileName                   << "\n";
//   os << prefix << "_basePath           " << _basePath                   << "\n";  
//   os << prefix << "_fileSystem         " << _fileSystem                 << "\n";
//   os << prefix << "_workingDir         " << _workingDir                 << "\n";
//   os << prefix << "_logPath            " << _logPath                    << "\n";
//   os << prefix << "_logFile            " << _logFile                    << "\n";
//   os << prefix << "_setupLabel         " << _diskWritingParams._setupLabel<< "\n";
//   os << prefix << "_streamLabel        " << _streamLabel                << "\n";
//   os << prefix << "_hostName           " << _diskWritingParams._hostName<< "\n";
//   os << prefix << "fileCatalog()       " << _diskWritingParams._fileCatalog<< "\n"; 
//   os << prefix << "_lumiSection        " << _lumiSection                << "\n";
//   os << prefix << "_runNumber          " << _runNumber                  << "\n";
//   os << prefix << "_fileCounter        " << _fileCounter                << "\n";
//   os << prefix << "fileSize            " << _fileSize                   << "\n";
//   os << prefix << "events              " << _events                     << "\n";
//   os << prefix << "first entry         " << _firstEntry                 << "\n";
//   os << prefix << "last entry          " << _lastEntry                  << "\n";
//   os << prefix << "why closed          " << _whyClosed                  << "\n";
//   os << prefix << "-----------------------------------------\n";  
// }

/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
