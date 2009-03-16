// $Id: FileHandler.h,v 1.1.2.1 2009/03/16 10:47:12 mommsen Exp $

#ifndef StorageManager_FileHandler_h
#define StorageManager_FileHandler_h

#include <EventFilter/StorageManager/interface/Configuration.h>
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
   * $Revision: 1.1.2.1 $
   * $Date: 2009/03/16 10:47:12 $
   */

  class FileHandler
  {
  public:
    
    /**
     * Enumaration why the file was closed
     */
    enum ClosingReason
    {
      notClosed = 0,
      stop,
      Nminus2lumi,
      timeout,
      size
    };
    
    FileHandler
    (
      const uint32_t lumiSection,
      const std::string &file,
      DiskWritingParams dwParams
    );
    
    virtual ~FileHandler() {};
    
    /**
     * Write the event contained in a buffer
     * This is the old, obsolete API.
     */
    virtual void writeEvent(const uint8 * const bufPtr) = 0;
 
    /**
     * Write the event contained in the I2OChain
     */
    virtual void writeEvent(const I2OChain&) = 0;



    ////////////////////////////
    // File parameter setters //
    ////////////////////////////
    
    /**
     * Set the run number
     */
    void setRunNumber(const uint32_t i)
    { _runNumber = i; }
    
    /**
     * Set the current file system
     */
    void setFileSystem(const unsigned int i);
    
    /**
     * Set the file counter
     */
    void setFileCounter(const unsigned int i)
    { _fileCounter = i; }
    
    /**
     * Set the stream label
     */
    void setStreamLabel(const std::string &s)
    { _streamLabel = s;}
    
    /**
     * Set the reason why the file was closed
     */
    void setWhyClosed(ClosingReason w)
    { _whyClosed  = w; }
    
    /**
     * Set the adler checksum for the file
     */
    void setadler(unsigned int s, unsigned int i)
    { _adlerstream = s; _adlerindex = i; }
    
    /**
     * Add number of bytes to file size
     */
    void increaseFileSize(const long long size)
    { _fileSize += size; }
    
    /**
     * Increment number of events stored in the file
     */
    void increaseEventCount()
    { _events++; }
    
    /**
     * Time when first event was added
     */
    void firstEntry(utils::time_point_t d)
    { _firstEntry = d; }
    
    /**
     * Time when latest event was added
     */
    void lastEntry(utils::time_point_t d)
    { _lastEntry = d; }
    
    
    
    ////////////////////////////
    // File parameter getters //
    ////////////////////////////
    
    /**
     * Return the number of events in the file
    */
    const int events() const
    { return _events; } 
    
    /**
     * Return the luminosity section the file belongs to
     */
    const uint32_t lumiSection() const
    { return _lumiSection; }
    
    /**
     * Return the size of the file in bytes
     */
    const long long fileSize() const
    { return _fileSize; }
    
    /**
     * Return the full path where the file resides
     */
    const std::string filePath() const;
    
    /**
     * Return the complete file name and path w/o file ending
     */
    const std::string completeFileName() const;
    
    /**
     * Return the time when first event was added
     */
    const utils::time_point_t firstEntry() const
    { return _firstEntry; }
    
    /**
     * Return the time when latest event was added
     */
    const utils::time_point_t lastEntry() const
    { return _lastEntry; }
    

        
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
    void moveFileToClosed();
    
    /**
     * Move error event file to "closed" directory
     */
    void moveErrorFileToClosed();
    
    /**
     * Check if all directories needed for the file output are available.
     * Throws a cms::Exception when a directory does not exist.
     */
    void checkDirectories() const;
    
    /**
     * Throw a cms::Exception when the directory does not exist
     */
    void checkDirectory(const std::string &) const;
        
    /**
     * Return the _fileName and _fileCounter
     */
    const std::string qualifiedFileName() const;
    
    /**
     * Return the name of the log file
     */
    const std::string logFile(stor::DiskWritingParams const&) const;

    /**
     * Return the relative difference btw to file sizes
     */
    const double calcPctDiff(long long, long long) const;
    
    
  private:
    DiskWritingParams _diskWritingParams;
    
    std::string  _fileName;                         // file name (w/o ending)
    std::string  _basePath;                         // base path name
    std::string  _fileSystem;                       // file system directory
    std::string  _workingDir;                       // current working directory
    std::string  _logPath;                          // log path
    std::string  _logFile;                          // log file including path
    std::string  _streamLabel;                      // datastream label
    std::string  _cmsver;                           // CMSSW version string
    uint32_t     _lumiSection;                      // luminosity section  
    uint32_t     _runNumber;                        // runNumber
    unsigned int _fileCounter;                      // number of files with _fileName as name
    long long    _fileSize;                         // current file size in ??? bytes
    uint32_t     _events;                           // total number of events
    utils::time_point_t _firstEntry;                // time when first event was writen
    utils::time_point_t _lastEntry;                 // time when latest event was writen
    
    ClosingReason _whyClosed;                       // record why file was closed 
    
    unsigned int _adlerstream;                      // adler32 checksum for streamer file
    unsigned int _adlerindex;                       // adler32 checksum for index file

  };
  
} // stor namespace

#endif // StorageManager_FileHandler_h


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
