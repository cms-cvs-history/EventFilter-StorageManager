// $Id: FileHandler.h,v 1.1.2.3 2009/03/17 15:57:26 mommsen Exp $

#ifndef StorageManager_FileHandler_h
#define StorageManager_FileHandler_h

#include <EventFilter/StorageManager/interface/Configuration.h>
#include <EventFilter/StorageManager/interface/FilesMonitorCollection.h>
#include <EventFilter/StorageManager/interface/I2OChain.h>
#include <EventFilter/StorageManager/interface/Utils.h>
#include <IOPool/Streamer/interface/MsgHeader.h>

#include <boost/shared_ptr.hpp>

#include <string>


namespace stor {

  /**
   * Abstract representation of a physical file
   *
   * $Author: mommsen $
   * $Revision: 1.1.2.3 $
   * $Date: 2009/03/17 15:57:26 $
   */

  class FileHandler
  {
  public:
        
    FileHandler
    (
      FilesMonitorCollection::FileRecord&,
      const DiskWritingParams&
    );
    
    virtual ~FileHandler() {};
 
    /**
     * Write the event contained in the I2OChain
     */
    virtual void writeEvent(const I2OChain&) = 0;



    ////////////////////////////
    // File parameter setters //
    ////////////////////////////

    /**
     * Set the current file system
     */
    void setFileSystem(const unsigned int& i);
    
    /**
     * Set the adler checksum for the file
     */
    void setadler(uint32 s, uint32 i)
    { _adlerstream = s; _adlerindex = i; }
    
        
    /////////////////////////////
    // File information dumper //
    /////////////////////////////
    
    /**
     * Add file summary information to ostream
     */
    void info(std::ostream &os) const;
    
    /**
     * Dump status information to ostream
     */
    //void report(std::ostream &os, int indentation) const;
    


  protected:

    ////////////////////////////
    // File parameter getters //
    ////////////////////////////
    
    /**
     * Return the number of events in the file
    */
    const int events() const
    { return _fileRecord.fileSize.getSampleCount(); } 
    
    /**
     * Return the luminosity section the file belongs to
     */
    const uint32_t lumiSection() const
    { return _fileRecord.lumiSection; }
    
    /**
     * Return the size of the file in bytes
     */
    const long long fileSize() const
    { return static_cast<long long>(_fileRecord.fileSize.getValueSum()); }
    
    /**
     * Return the full path where the file resides
     */
    const std::string filePath() const
    { return _fileRecord.filePath; }
    
    /**
     * Return the complete file name and path w/o file ending
     */
    const std::string completeFileName() const
    { return _fileRecord.filePath + "/" + _fileRecord.fileName; }
    
    
    //////////////////////
    // File bookkeeping //
    //////////////////////
    
    /**
     * Close the file
     */
    virtual void closeFile() = 0;
    
    /**
     * Write summary information in file catalog
     */
    void writeToSummaryCatalog() const;

    /**
     * Write command to update the file information in the CMS_STOMGR.TIER0_INJECTION table
     * into the _logFile.
     */
    void updateDatabase() const;

    /**
     * Write command to insert a new file into the CMS_STOMGR.TIER0_INJECTION table
     * into the _logFile.
     */
    void insertFileInDatabase() const;

    
    /////////////////////////////
    // File system interaction //
    /////////////////////////////
    
    /**
     * Move index and streamer file to "closed" directory
     */
    void moveFileToClosed(const bool& useIndexFile);

    /**
     * Check that the file size matches the given size.
     * Returns the actual size.
     */
    size_t checkFileSizeMatch(const std::string& fileName, const size_t& size);

    /**
     * Check that the 2 sizes agree
     */
    bool sizeMismatch(const double& initialSize, const double& finalSize);

    /**
     * Changes the file permissions to read-only
     */
    void makeFileReadOnly(const std::string& fileName);

    /**
     * Rename the file
     */
    void renameFile(const std::string& openFileName, const std::string& closedFileName);
    
    /**
     * Check if all directories needed for the file output are available.
     * Throws a cms::Exception when a directory does not exist.
     */
    void checkDirectories() const;
    
    /**
     * Throw a cms::Exception when the directory does not exist
     */
    void checkDirectory(const std::string&) const;
        
    /**
     * Return the _fileName and _fileCounter
     */
    const std::string qualifiedFileName() const;
    
    /**
     * Return the name of the log file
     */
    const std::string logFile(const DiskWritingParams&) const;

    /**
     * Return the relative difference btw to file sizes
     */
    const double calcPctDiff(const double&, const double&) const;
    

  protected:
    FilesMonitorCollection::FileRecord& _fileRecord;

    utils::time_point_t _firstEntry;                // time when first event was writen
    utils::time_point_t _lastEntry;                 // time when latest event was writen

    
  private:
    DiskWritingParams _diskWritingParams;
    
    std::string  _workingDir;                       // current working directory
    std::string  _logPath;                          // log path
    std::string  _logFile;                          // log file including path
    std::string  _streamLabel;                      // datastream label
    std::string  _cmsver;                           // CMSSW version string
    
    uint32       _adlerstream;                      // adler32 checksum for streamer file
    uint32       _adlerindex;                       // adler32 checksum for index file

    unsigned int _fileCounter;                      // number of files with _fileName as name
    std::string  _fileSystem;                       // file system directory

  };
  
} // stor namespace

#endif // StorageManager_FileHandler_h


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
