// $Id: EventFileHandler.h,v 1.1.2.5 2009/03/20 17:53:53 mommsen Exp $

#ifndef StorageManager_EventFileHandler_h
#define StorageManager_EventFileHandler_h

#include "EventFilter/StorageManager/interface/FileHandler.h"
#include "EventFilter/StorageManager/interface/InitMsgCollection.h"

#include <IOPool/Streamer/src/StreamerFileWriter.h>


namespace stor {
  
  /**
   * Represents a file holding event data
   *
   * $Author: mommsen $
   * $Revision: 1.1.2.5 $
   * $Date: 2009/03/20 17:53:53 $
   */
  
  class EventFileHandler : public FileHandler
  {
  public:
    EventFileHandler
    (
      InitMsgSharedPtr,
      FilesMonitorCollection::FileRecordPtr,
      const DiskWritingParams&,
      const long long& maxFileSize
    );

    virtual ~EventFileHandler();
        
    /**
     * Write the event contained in the I2OChain
     */
    virtual void writeEvent(const I2OChain&);
 
    /**
     * Return the number of events in the file.
     * The event fileRecord has one entry more than than events
     * as the Init message is counted, too.
     */
    const int events() const;
     
    //      void   report(std::ostream &os, int indentation) const;
    
  private:
    
    /**
     * Close the file
     */
    virtual void closeFile();

    /**
     * Write the init message to the head of the file
     */
    void writeHeader(InitMsgSharedPtr);
    

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
