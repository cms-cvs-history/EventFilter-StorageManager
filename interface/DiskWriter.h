// $Id: DiskWriter.h,v 1.1.2.2 2009/01/20 10:54:04 mommsen Exp $

#ifndef StorageManager_DiskWriter_h
#define StorageManager_DiskWriter_h

#include "EventFilter/StorageManager/interface/StreamQueue.h"


namespace stor {

  /**
   * Writes events to disk
   *
   * It gets the next event from the StreamQueue and writes it
   * to the appropriate stream file(s) on disk. 
   *
   * $Author:$
   * $Revision:$
   * $Date:$
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
    void configureStreams(std::string configString);

    /**
     * ???
     */    
    void makeEventStreams();

    /**
     * ???
     */    
    void parseEventStreamConfig();

    /**
     * ???
     */    
    void destroyStreams();


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