// $Id: FilesMonitorCollection.h,v 1.1.2.3 2009/04/02 13:56:05 mommsen Exp $

#ifndef StorageManager_FilesMonitorCollection_h
#define StorageManager_FilesMonitorCollection_h

#include <sstream>
#include <iomanip>
#include <vector>

#include <boost/shared_ptr.hpp>

#include "xdata/UnsignedInteger32.h"

#include "EventFilter/StorageManager/interface/MonitorCollection.h"


namespace stor {

  /**
   * A collection of MonitoredQuantities of open and closed files
   *
   * $Author: mommsen $
   * $Revision: 1.1.2.3 $
   * $Date: 2009/04/02 13:56:05 $
   */
  
  class FilesMonitorCollection : public MonitorCollection
  {
  public:

    struct FileRecord
    {
      enum ClosingReason
      {
        notClosed = 0,
        stop,
        Nminus2lumi,
        timeout,
        size
      };

      uint32_t          runNumber;          // run number
      uint32_t          lumiSection;        // luminosity section 
      std::string       streamLabel;        // datastream label
      std::string       baseFilePath;       // file path w/o the working directory
      std::string       coreFileName;       // file name w/o instance & file ending
      unsigned int      fileCounter;        // counter of number of coreFileNames used
      ClosingReason     whyClosed;          // reason why the given file was closed
      MonitoredQuantity fileSize;           // file size
      MonitoredQuantity eventCount;         // number of events
      std::string closingReason();          // reason why file was closed
      std::string filePath();               // complete file path
      std::string fileName();               // full file name w/o file ending
      std::string completeFileName()
      { return ( filePath() + "/" + fileName() ); }

    };

    // We do not know how many files there will be.
    // Thus, we need a vector of them.
    typedef boost::shared_ptr<FileRecord> FileRecordPtr;
    typedef std::vector<FileRecordPtr> FileRecordList;


    explicit FilesMonitorCollection(xdaq::Application*);

    FileRecordPtr getNewFileRecord(double timeWindowForRecentResults = 60);

    const FileRecordList& getFileRecordsMQ() const {
      return _fileRecords;
    }
    FileRecordList& getFileRecordsMQ() {
      return _fileRecords;
    }


  private:

    //Prevent copying of the FilesMonitorCollection
    FilesMonitorCollection(FilesMonitorCollection const&);
    FilesMonitorCollection& operator=(FilesMonitorCollection const&);

    virtual void do_calculateStatistics();
    
    virtual void do_updateInfoSpace();

    FileRecordList _fileRecords;


    // InfoSpace items which were defined in the old SM


  };
  
} // namespace stor

#endif // StorageManager_FilesMonitorCollection_h 


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
