// $Id: FilesMonitorCollection.h,v 1.1.2.6 2009/03/02 18:08:21 biery Exp $

#ifndef StorageManager_FilesMonitorCollection_h
#define StorageManager_FilesMonitorCollection_h

#include <vector>

#include "xdata/UnsignedInteger32.h"

#include "EventFilter/StorageManager/interface/MonitorCollection.h"


namespace stor {

  /**
   * A collection of MonitoredQuantities of open and closed files
   *
   * $Author: biery $
   * $Revision: 1.1.2.6 $
   * $Date: 2009/03/02 18:08:21 $
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
      std::string       filePath;           // complete file path
      std::string       fileName;           // qualified file name w/o file ending
      ClosingReason     whyClosed;          // reason why the given file was closed
      MonitoredQuantity fileSize;           // file size
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
