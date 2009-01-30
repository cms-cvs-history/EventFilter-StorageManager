// $Id: StreamQueue.h,v 1.1.2.2 2009/01/20 10:54:04 mommsen Exp $

#ifndef StorageManager_StreamQueue_h
#define StorageManager_StreamQueue_h

#include "EventFilter/StorageManager/interface/Queue.h"


namespace stor {

  /**
   * Queue holding I2OChains of complete events waiting to be
   * written to the appropriate streams on disk.
   *
   * $Author:$
   * $Revision:$
   * $Date:$
   */
  
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
