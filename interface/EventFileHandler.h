// $Id: EventOutputService.h,v 1.1.12.1 2009/03/12 14:38:19 mommsen Exp $

#ifndef StorageManager_EventFileHandler_h
#define StorageManager_EventFileHandler_h

#include <EventFilter/StorageManager/interface/FileHandler.h>
#include <IOPool/Streamer/interface/InitMessage.h>
#include <IOPool/Streamer/src/StreamerFileWriter.h>

namespace stor {
  
  /**
   * Represents a file holding event data
   *
   * $Author: mommsen $
   * $Revision: 1.9.4.2 $
   * $Date: 2009/03/12 14:33:22 $
   */
  
  class EventFileHandler : public FileHandler
  {
  public:
    EventFileHandler
    (
      InitMsgView const&,
      const uint32_t lumiSection,
      const std::string &file,
     DiskWritingParams dwParams
    );

    virtual ~EventFileHandler();
    
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

    /**
     * Write the init message to the head of the file
     */
    void writeHeader(InitMsgView const&);
    

    edm::StreamerFileWriter _writer; // writes streamer and index file
  };
  
} // stor namespace

#endif // StorageManager_EventFileHandler_h


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
