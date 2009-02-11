// $Id: FragmentStore.h,v 1.1.2.3 2009/01/30 10:49:40 mommsen Exp $

#ifndef StorageManager_FragmentStore_h
#define StorageManager_FragmentStore_h

#include <map>

#include "EventFilter/StorageManager/interface/I2OChain.h"


namespace stor {
  
  /**
   * Stores incomplete events
   *
   * Uses a map of I2OChains to store incomplete events.
   *
   * $Author: mommsen $
   * $Revision: 1.1.2.3 $
   * $Date: 2009/01/30 10:49:40 $
   */
  
  class FragmentStore
  {
  public:
    
    FragmentStore();
    
    ~FragmentStore();
    
    /**
     * Adds fragments of the I2OChain to the fragment store.
     * If the passed fragments completes an event, it returns true.
     * In this case, the passed I2OChain contains the completed event.
     */
    bool addFragment(I2OChain&);

    
  private:

    std::map<unsigned int, I2OChain> _chains;
    
    
  };
  
} // namespace stor

#endif // StorageManager_FragmentStore_h 


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -