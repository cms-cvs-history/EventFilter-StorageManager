// $Id: StreamQueue.h,v 1.1.2.3 2009/01/30 10:49:40 mommsen Exp $

#ifndef StorageManager_StreamQueue_h
#define StorageManager_StreamQueue_h

#include "EventFilter/StorageManager/interface/Queue.h"


namespace stor {

  /**
   * Queue holding I2OChains of complete events waiting to be
   * written to the appropriate streams on disk.
   *
   * $Author: mommsen $
   * $Revision: 1.1.2.3 $
   * $Date: 2009/01/30 10:49:40 $
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
