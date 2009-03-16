// $Id: FRDFileHandler.h,v 1.1.12.1 2009/03/12 14:38:19 mommsen Exp $

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
   * $Revision: 1.9.4.2 $
   * $Date: 2009/03/12 14:33:22 $
   */
  
  class FRDFileHandler : public FileHandler
  {
  public:
    FRDFileHandler
    (
      const uint32_t lumiSection,
      const std::string &file,
      DiskWritingParams dwParams
    );
    
    virtual ~FRDFileHandler();
    
    /**
     * Write the event contained in a buffer
     * This is the old, obsolete API.
     */
    virtual void writeEvent(const uint8 * const bufPtr);
        
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
