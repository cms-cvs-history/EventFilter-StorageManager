// $Id: FileRecord.cc,v 1.13.4.5 2009/03/16 19:05:35 biery Exp $

#include <EventFilter/StorageManager/interface/FileRecord.h>
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


FileRecord::FileRecord(const uint32_t lumiSection, const string &file, DiskWritingParams dwParams):
  fileName_(file),
  basePath_(dwParams._filePath),
  fileSystem_(""),
  workingDir_("/open/"),
  logPath_(dwParams._filePath+"/log"),
  logFile_(logFile(dwParams)),
  streamLabel_(""),
  cmsver_(edm::getReleaseVersion()),
  lumiSection_(lumiSection),
  runNumber_(0),
  fileCounter_(0),
  fileSize_(0), 
  events_(0), 
  firstEntry_(0.0), 
  lastEntry_(0.0),
  whyClosed_(notClosed),
  diskWritingParams_(dwParams)
{
   // stripp quotes if present
   if(cmsver_[0]=='"') cmsver_=cmsver_.substr(1,cmsver_.size()-2);
}


//////////////////////
// File bookkeeping //
//////////////////////

void FileRecord::writeToSummaryCatalog() const
{
  ostringstream currentStat;
  string ind(":");
  currentStat << basePath_ + fileSystem_ + workingDir_ << ind
              << qualifiedFileName()                   << ind
	      << fileSize()                            << ind 
	      << events()                              << ind
              << utils::timeStamp(lastEntry())         << ind
	      << (int) (lastEntry()-firstEntry())      << ind
	      << whyClosed_ << endl;
  string currentStatString (currentStat.str());
  ofstream of(diskWritingParams_._fileCatalog.c_str(), ios_base::ate | ios_base::out | ios_base::app );
  of << currentStatString;
  of.close();
}


void FileRecord::updateDatabase() const
{
  std::ostringstream oss;
  oss << "./closeFile.pl "
      << " --FILENAME "     << qualifiedFileName() <<  ".dat"
      << " --FILECOUNTER "  << fileCounter_                       
      << " --NEVENTS "      << events_                            
      << " --FILESIZE "     << fileSize_                          
      << " --STARTTIME "    << (int) firstEntry()
      << " --STOPTIME "     << (int) lastEntry()
      << " --STATUS "       << "closed"
      << " --RUNNUMBER "    << runNumber_                         
      << " --LUMISECTION "  << lumiSection_                      
      << " --PATHNAME "     << filePath()
      << " --HOSTNAME "     << diskWritingParams_._hostName
      << " --SETUPLABEL "   << diskWritingParams_._setupLabel
      << " --STREAM "       << streamLabel_                      
      << " --INSTANCE "     << diskWritingParams_._smInstanceString
      << " --SAFETY "       << diskWritingParams_._initialSafetyLevel
      << " --APPVERSION "   << cmsver_
      << " --APPNAME CMSSW"
      << " --TYPE streamer"               
      << " --DEBUGCLOSE "   << whyClosed_
      << " --CHECKSUM "     << hex << adlerstream_
      << " --CHECKSUMIND "  << hex << adlerindex_
      << "\n";

  ofstream of(logFile_.c_str(), ios_base::ate | ios_base::out | ios_base::app );
  of << oss.str().c_str();
  of.close();
}


void FileRecord::insertFileInDatabase() const
{
  std::ostringstream oss;
  oss << "./insertFile.pl "
      << " --FILENAME "     << qualifiedFileName() <<  ".dat"
      << " --FILECOUNTER "  << fileCounter_                       
      << " --NEVENTS "      << events_                            
      << " --FILESIZE "     << fileSize_                          
      << " --STARTTIME "    << (int) firstEntry()
      << " --STOPTIME "     << (int) lastEntry()
      << " --STATUS "       << "open"
      << " --RUNNUMBER "    << runNumber_                         
      << " --LUMISECTION "  << lumiSection_                      
      << " --PATHNAME "     << filePath()
      << " --HOSTNAME "     << diskWritingParams_._hostName
      << " --SETUPLABEL "   << diskWritingParams_._setupLabel
      << " --STREAM "       << streamLabel_                      
      << " --INSTANCE "     << diskWritingParams_._smInstanceString
      << " --SAFETY "       << diskWritingParams_._initialSafetyLevel
      << " --APPVERSION "  << cmsver_
      << " --APPNAME CMSSW"
      << " --TYPE streamer"               
      << " --CHECKSUM 0"
      << " --CHECKSUMIND 0"
      << "\n";

  ofstream of(logFile_.c_str(), ios_base::ate | ios_base::out | ios_base::app );
  of << oss.str().c_str();
  of.close();
}



////////////////////////////
// File parameter setters //
////////////////////////////

void FileRecord::setFileSystem(const unsigned int i)
{
  std::ostringstream oss;
  oss << "/" << setfill('0') << std::setw(2) << i; 
  fileSystem_ = oss.str();
}


////////////////////////////
// File parameter getters //
////////////////////////////

const string FileRecord::filePath() const
{
  return ( basePath_ + fileSystem_ + workingDir_);
}


const string FileRecord::completeFileName() const
{
  return ( filePath() + qualifiedFileName() );
}


const string FileRecord::qualifiedFileName() const
{
  std::ostringstream oss;
  oss << fileName_;
  oss << "." << setfill('0') << std::setw(4) << fileCounter_;
  return oss.str();
}



/////////////////////////////
// File system interaction //
/////////////////////////////

void FileRecord::moveFileToClosed()
{
  struct stat64 initialStatBuff, finalStatBuff;
  int statStatus;
  double pctDiff;
  bool sizeMismatch;

  string openIndexFileName      = completeFileName() + ".ind";
  string openStreamerFileName   = completeFileName() + ".dat";
  statStatus = stat64(openStreamerFileName.c_str(), &initialStatBuff);
  if (statStatus != 0) {
    throw cms::Exception("FileRecord", "moveFileToClosed")
      << "Error checking the status of open file "
      << openStreamerFileName << ".  Has the file moved unexpectedly?"
      << std::endl;
  }
  sizeMismatch = false;
  if (diskWritingParams_._exactFileSizeTest) {
    if (fileSize_ != initialStatBuff.st_size) {
      sizeMismatch = true;
    }
  }
  else {
    pctDiff = calcPctDiff(fileSize_, initialStatBuff.st_size);
    if (pctDiff > 0.1) {sizeMismatch = true;}
  }
  if (sizeMismatch) {
    throw cms::Exception("FileRecord", "moveFileToClosed")
      << "Found an unexpected open file size when trying to move "
      << "the file to the closed state.  File " << openStreamerFileName
      << " has an actual size of " << initialStatBuff.st_size
      << " instead of the expected size of " << fileSize_ << std::endl;
  }

  int ronly  = chmod(openStreamerFileName.c_str(), S_IREAD|S_IRGRP|S_IROTH);
  ronly     += chmod(openIndexFileName.c_str(), S_IREAD|S_IRGRP|S_IROTH);
  if (ronly != 0) {
    throw cms::Exception("FileRecord", "moveFileToClosed")
      << "Unable to change permissions of " << openStreamerFileName << " and "
      << openIndexFileName << " to read only." << std::endl;
  }

  workingDir_ = "/closed/";
  string closedIndexFileName    = completeFileName() + ".ind";
  string closedStreamerFileName = completeFileName() + ".dat";

  int result = rename( openIndexFileName.c_str()    , closedIndexFileName.c_str() );
  result    += rename( openStreamerFileName.c_str() , closedStreamerFileName.c_str() );
  if (result != 0) {
    throw cms::Exception("FileRecord", "moveFileToClosed")
      << "Unable to move " << openStreamerFileName << " to "
      << closedStreamerFileName << ".  Possibly the storage manager "
      << "disk areas are full." << std::endl;
  }

  statStatus = stat64(closedStreamerFileName.c_str(), &finalStatBuff);
  if (statStatus != 0) {
    throw cms::Exception("FileRecord", "moveFileToClosed")
      << "Error checking the status of closed file "
      << closedStreamerFileName << ".  This file was copied from "
      << openStreamerFileName << ", and the copy seems to have failed."
      << std::endl;
  }
  sizeMismatch = false;
  if (diskWritingParams_._exactFileSizeTest) {
    if (initialStatBuff.st_size != finalStatBuff.st_size) {
      sizeMismatch = true;
    }
  }
  else {
    pctDiff = calcPctDiff(initialStatBuff.st_size, finalStatBuff.st_size);
    if (pctDiff > 0.1) {sizeMismatch = true;}
  }
  if (sizeMismatch) {
    throw cms::Exception("FileRecord", "moveFileToClosed")
      << "Error moving " << openStreamerFileName << " to "
      << closedStreamerFileName << ".  The closed file size ("
      << finalStatBuff.st_size << ") is different than the open file size ("
      << initialStatBuff.st_size << ").  Possibly the storage manager "
      << "disk areas are full." << std::endl;
  }
}


void FileRecord::moveErrorFileToClosed()
{
  struct stat64 initialStatBuff, finalStatBuff;
  int statStatus;
  double pctDiff;
  bool sizeMismatch;

  string openErrorFileName   = completeFileName() + ".dat";
  statStatus = stat64(openErrorFileName.c_str(), &initialStatBuff);
  if (statStatus != 0) {
    throw cms::Exception("FileRecord", "moveErrorFileToClosed")
      << "Error checking the status of open error file "
      << openErrorFileName << ".  Has the file moved unexpectedly?"
      << std::endl;
  }
  sizeMismatch = false;
  if (diskWritingParams_._exactFileSizeTest) {
    if (fileSize_ != initialStatBuff.st_size) {
      sizeMismatch = true;
    }
  }
  else {
    pctDiff = calcPctDiff(fileSize_, initialStatBuff.st_size);
    if (pctDiff > 0.1) {sizeMismatch = true;}
  }
  if (sizeMismatch) {
    throw cms::Exception("FileRecord", "moveErrorFileToClosed")
      << "Found an unexpected open file size when trying to move "
      << "the file to the closed state.  File " << openErrorFileName
      << " has an actual size of " << initialStatBuff.st_size
      << " instead of the expected size of " << fileSize_ << std::endl;
  }

  int ronly  = chmod(openErrorFileName.c_str(), S_IREAD|S_IRGRP|S_IROTH);
  if (ronly != 0) {
    throw cms::Exception("FileRecord", "moveErrorFileToClosed")
      << "Unable to change permissions of " << openErrorFileName
      << " to read only." << std::endl;
  }

  workingDir_ = "/closed/";
  string closedErrorFileName = completeFileName() + ".dat";

  int result = rename( openErrorFileName.c_str() , closedErrorFileName.c_str() );
  if (result != 0) {
    throw cms::Exception("FileRecord", "moveErrorFileToClosed")
      << "Unable to move " << openErrorFileName << " to "
      << closedErrorFileName << ".  Possibly the storage manager "
      << "disk areas are full." << std::endl;
  }

  statStatus = stat64(closedErrorFileName.c_str(), &finalStatBuff);
  if (statStatus != 0) {
    throw cms::Exception("FileRecord", "moveErrorFileToClosed")
      << "Error checking the status of closed file "
      << closedErrorFileName << ".  This file was copied from "
      << openErrorFileName << ", and the copy seems to have failed."
      << std::endl;
  }
  sizeMismatch = false;
  if (diskWritingParams_._exactFileSizeTest) {
    if (initialStatBuff.st_size != finalStatBuff.st_size) {
      sizeMismatch = true;
    }
  }
  else {
    pctDiff = calcPctDiff(initialStatBuff.st_size, finalStatBuff.st_size);
    if (pctDiff > 0.1) {sizeMismatch = true;}
  }
  if (sizeMismatch) {
    throw cms::Exception("FileRecord", "moveErrorFileToClosed")
      << "Error moving " << openErrorFileName << " to "
      << closedErrorFileName << ".  The closed file size ("
      << finalStatBuff.st_size << ") is different than the open file size ("
      << initialStatBuff.st_size << ").  Possibly the storage manager "
      << "disk areas are full." << std::endl;
  }
}


const string FileRecord::logFile(stor::DiskWritingParams const& dwp) const
{
  time_t rawtime = time(0);
  tm * ptm;
  ptm = localtime(&rawtime);

  ostringstream logfilename;
  logfilename << logPath_ << "/"
              << setfill('0') << std::setw(4) << ptm->tm_year+1900
              << setfill('0') << std::setw(2) << ptm->tm_mon+1
              << setfill('0') << std::setw(2) << ptm->tm_mday
              << "-" << dwp._hostName
              << "-" << dwp._smInstanceString
              << ".log";
  return logfilename.str();
}


void FileRecord::checkDirectories() const
{
  checkDirectory(basePath_);
  checkDirectory(basePath_ + fileSystem_);
  checkDirectory(basePath_ + fileSystem_ + "/open");
  checkDirectory(basePath_ + fileSystem_ + "/closed");
  checkDirectory(logPath_);
}


void FileRecord::checkDirectory(const string &path) const
{
  struct stat64 buf;

  int retVal = stat64(path.c_str(), &buf);
  if(retVal !=0 )
    {
      edm::LogError("StorageManager") << "Directory " << path
				      << " does not exist. Error=" << errno ;
      throw cms::Exception("FileRecord","checkDirectory")
            << "Directory " << path << " does not exist. Error=" << errno << std::endl;
    }
}


const double FileRecord::calcPctDiff(long long value1, long long value2) const
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

void FileRecord::info(ostream &os) const
{
  os << fileCounter_ << " " 
     << completeFileName() << " " 
     << events_ << " "
     << fileSize_;
}


void FileRecord::report(ostream &os, int indentation) const
{
  string prefix(indentation, ' ');
  os << "\n";
  os << prefix << "------------- FileRecord -------------\n";
  os << prefix << "fileName            " << fileName_                   << "\n";
  os << prefix << "basePath_           " << basePath_                   << "\n";  
  os << prefix << "fileSystem_         " << fileSystem_                 << "\n";
  os << prefix << "workingDir_         " << workingDir_                 << "\n";
  os << prefix << "logPath_            " << logPath_                    << "\n";
  os << prefix << "logFile_            " << logFile_                    << "\n";
  os << prefix << "setupLabel_         " << diskWritingParams_._setupLabel<< "\n";
  os << prefix << "streamLabel_        " << streamLabel_                << "\n";
  os << prefix << "hostName_           " << diskWritingParams_._hostName<< "\n";
  os << prefix << "fileCatalog()       " << diskWritingParams_._fileCatalog<<"\n"; 
  os << prefix << "lumiSection_        " << lumiSection_                << "\n";
  os << prefix << "runNumber_          " << runNumber_                  << "\n";
  os << prefix << "fileCounter_        " << fileCounter_                << "\n";
  os << prefix << "fileSize            " << fileSize_                   << "\n";
  os << prefix << "events              " << events_                     << "\n";
  os << prefix << "first entry         " << firstEntry_                 << "\n";
  os << prefix << "last entry          " << lastEntry_                  << "\n";
  os << prefix << "why closed          " << whyClosed_                  << "\n";
  os << prefix << "-----------------------------------------\n";  
}

/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
