// $Id: FRDFileHandler.h,v 1.1.2.2 2009/03/17 15:57:26 mommsen Exp $

#ifndef StorageManager_FRDFileHandler_h
#define StorageManager_FRDFileHandler_h

#include <EventFilter/StorageManager/interface/FileHandler.h>
#include <IOPool/Streamer/interface/FRDEventFileWriter.h>

namespace stor {
  
  /**
   * Represents a file holding HLT error events in the
   * FED Raw Data (FRD) format.
   *
   * $Author: mommsen $
   * $Revision: 1.1.2.2 $
   * $Date: 2009/03/17 15:57:26 $
   */
  
  class FRDFileHandler : public FileHandler
  {
  public:
    FRDFileHandler
    (
      FilesMonitorCollection::FileRecord& fileRecord,
      const DiskWritingParams& dwParams
    );
    
    virtual ~FRDFileHandler();
        
    /**
     * Write the event contained in the I2OChain
     */
    virtual void writeEvent(const I2OChain&);
    
    //      void   report(std::ostream &os, int indentation) const;
    
  private:
    
    /**
     * Close the file
     */
    virtual void closeFile();
    
    FRDEventFileWriter _writer;
  };
  
} // stor namespace

#endif // StorageManager_FRDFileHandler_h


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -