// $Id: DiskWriter.h,v 1.1.2.1 2009/01/19 18:12:17 mommsen Exp $

/**
 * @file
 * Writes events to disk
 *
 * It gets the next event from the StreamQueue and writes it
 * to the appropriate stream file(s) on disk. 
 */

#ifndef StorageManager_DiskWriter_h
#define StorageManager_DiskWriter_h

#include "EventFilter/StorageManager/interface/StreamQueue.h"


namespace stor {
  
  class DiskWriter
  {
  public:
    
    DiskWriter();
    
    ~DiskWriter();

    /**
     * Takes the next event from the StreamQueue and writes it to disk
     */    
    void writeNextEvent();


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
