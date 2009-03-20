// $Id: StreamHandler.h,v 1.1.2.2 2009/03/20 11:16:45 mommsen Exp $

#ifndef StorageManager_StreamHandler_h
#define StorageManager_StreamHandler_h

#include "EventFilter/StorageManager/interface/FileHandler.h"
#include "EventFilter/StorageManager/interface/FilesMonitorCollection.h"
#include "EventFilter/StorageManager/interface/I2OChain.h"
#include "EventFilter/StorageManager/interface/SharedResources.h"


namespace stor {

  /**
   * Abstract class to handle one stream written to disk.
   *
   * $Author: mommsen $
   * $Revision: 1.1.2.2 $
   * $Date: 2009/03/20 11:16:45 $
   */
  
  class StreamHandler
  {
  public:
    
    explicit StreamHandler(SharedResourcesPtr);

    virtual ~StreamHandler();


    /**
     * Gracefully close all open files
     */    
    void closeAllFiles();

    /**
     * Close all files which are have not seen any recent events
     */    
    void closeTimedOutFiles();

    /**
     * Write the event to the stream file
     */    
    void writeEvent(const I2OChain& event);


  protected:

    typedef boost::shared_ptr<FileHandler> FileHandlerPtr;

    /**
     * Return the stream label
     */
    virtual std::string streamLabel() = 0;

    /**
     * Return a new file handler for the provided event
     */    
    virtual FileHandlerPtr newFileHandler(const I2OChain& event) = 0;

    /**
     * Return a new file record for the event
     */    
    FilesMonitorCollection::FileRecordPtr getNewFileRecord(const I2OChain& event);


  private:

    /**
     * Get the file handler responsible for the event
     */    
    FileHandlerPtr getFileHandler(const I2OChain& event);

    /**
     * Return true if the file would become too large when
     * adding dataSize MB (WHAT'S THE UNIT OF DATASIZE?)
     */    
    const bool fileTooLarge(const FileHandlerPtr, const unsigned long& dataSize);

    /**
     * Get path w/o working directory
     */    
    const std::string getBaseFilePath(const uint32& runNumber);

    /**
     * Get file system string
     */    
    const std::string getFileSystem(const uint32& runNumber);

    /**
     * Get the core file name
     */    
    const std::string getCoreFileName(const uint32& runNumber, const uint32& lumiSection);
    
    /**
     * Get the instance count of this core file name
     */    
    const unsigned int getFileCounter(const std::string& coreFileName);


  protected:

    FilesMonitorCollection& _filesMonitorCollection;
    const DiskWritingParams _diskWritingParams;

    typedef std::vector<FileHandlerPtr> FileHandlers;
    FileHandlers _fileHandlers;

    typedef std::map<std::string, unsigned int> CoreFileNamesMap;
    CoreFileNamesMap _usedCoreFileNames;
    

  private:

    //Prevent copying of the StreamHandler
    StreamHandler(StreamHandler const&);
    StreamHandler& operator=(StreamHandler const&);

  };
  
} // namespace stor

#endif // StorageManager_StreamHandler_h 


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
