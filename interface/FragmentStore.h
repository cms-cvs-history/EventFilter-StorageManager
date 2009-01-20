// $Id: FragmentStore.h,v 1.1.2.1 2009/01/19 18:12:17 mommsen Exp $

#ifndef StorageManager_FragmentStore_h
#define StorageManager_FragmentStore_h

#include "EventFilter/StorageManager/interface/Chain.h"


namespace stor {
  
  class FragmentStore
  {
  public:
    
    FragmentStore();
    
    ~FragmentStore();
    
    Chain addFragment();

    
  private:
    
  };
  
} // namespace stor

#endif // StorageManager_FragmentStore_h 


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
