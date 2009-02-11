// $Id: FragmentQueue.h,v 1.1.2.2 2009/01/30 10:49:40 mommsen Exp $

#ifndef StorageManager_FragmentQueue_h
#define StorageManager_FragmentQueue_h

#include "EventFilter/StorageManager/interface/Queue.h"


namespace stor {

  /**
   * Queue holding I2OChains of event fragments 
   *
   * $Author: mommsen $
   * $Revision: 1.1.2.2 $
   * $Date: 2009/01/30 10:49:40 $
   */
  
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
