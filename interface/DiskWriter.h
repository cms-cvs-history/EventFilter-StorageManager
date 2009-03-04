// $Id: DiskWriter.h,v 1.1.2.5 2009/03/01 20:36:29 biery Exp $

#ifndef StorageManager_DiskWriter_h
#define StorageManager_DiskWriter_h

#include "EventFilter/StorageManager/interface/EventStreamConfigurationInfo.h"
#include "EventFilter/StorageManager/interface/StreamQueue.h"


namespace stor {

  /**
   * Writes events to disk
   *
   * It gets the next event from the StreamQueue and writes it
   * to the appropriate stream file(s) on disk. 
   *
   * $Author: biery $
   * $Revision: 1.1.2.5 $
   * $Date: 2009/03/01 20:36:29 $
   */
  
  class DiskWriter
  {
  public:
    
    DiskWriter();
    
    ~DiskWriter();

    /**
     * Takes the next event from the StreamQueue and writes it to disk
     */    
    void writeNextEvent();

    /**
     * Configures the streams to be written to disk
     */    
    void configureStreams(std::vector<EventStreamConfigurationInfo>& cfgList);

    /**
     * ???
     */    
    void makeEventStreams();

    /**
     * ???
     */    
    void destroyStreams();

    /**
     * Checks if the disk writer is currently not processing any events.
     */
    bool empty();

  private:

    StreamQueue _streamQueue;

    
  };
  
} // namespace stor

#endif // StorageManager_DiskWriter_h 


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
