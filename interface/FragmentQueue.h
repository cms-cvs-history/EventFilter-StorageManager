// $Id: FragmentQueue.h,v 1.1.2.1 2009/01/19 18:12:17 mommsen Exp $

/**
 * @file
 * Queue holding I2OChains of event fragments 
 *
 */

#ifndef StorageManager_FragmentQueue_h
#define StorageManager_FragmentQueue_h

#include "EventFilter/StorageManager/interface/Queue.h"


namespace stor {
  
  class FragmentQueue : public Queue
  {
  public:
    
    FragmentQueue();
    
    ~FragmentQueue();
    
    
  private:
    
  };
  
} // namespace stor

#endif // StorageManager_FragmentQueue_h 


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
