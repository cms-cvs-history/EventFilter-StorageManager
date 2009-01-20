// $Id: DiskWriter.h,v 1.1.2.1 2009/01/19 18:12:17 mommsen Exp $

#ifndef StorageManager_DiskWriter_h
#define StorageManager_DiskWriter_h

#include "EventFilter/StorageManager/interface/StreamQueue.h"


namespace stor {
  
  class DiskWriter
  {
  public:
    
    DiskWriter();
    
    ~DiskWriter();
    
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
