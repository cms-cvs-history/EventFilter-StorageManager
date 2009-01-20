// $Id: StreamQueue.h,v 1.1.2.1 2009/01/19 18:12:17 mommsen Exp $

/**
 * @file
 * Queue holding I2OChains of complete events waiting to be
 * written to the appropriate streams on disk.
 *
 */

#ifndef StorageManager_StreamQueue_h
#define StorageManager_StreamQueue_h

#include "EventFilter/StorageManager/interface/Queue.h"


namespace stor {
  
  class StreamQueue : public Queue
  {
  public:
    
    StreamQueue();
    
    ~StreamQueue();
    
    
  private:
    
  };
  
} // namespace stor

#endif // StorageManager_StreamQueue_h 


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
