// $Id: FragmentStore.h,v 1.1.2.4 2009/01/30 20:32:21 paterno Exp $

#ifndef StorageManager_FragmentStore_h
#define StorageManager_FragmentStore_h

#include <map>

#include "IOPool/Streamer/interface/HLTInfo.h"

#include "EventFilter/StorageManager/interface/I2OChain.h"


namespace stor {
  
  /**
   * Stores incomplete events
   *
   * Uses a map of I2OChains to store incomplete events.
   *
   * $Author: paterno $
   * $Revision: 1.1.2.4 $
   * $Date: 2009/01/30 20:32:21 $
   */
  
  class FragmentStore
  {
  public:
    
    FragmentStore() {};

    /**
     * Adds fragments of the I2OChain to the fragment store.
     * If the passed fragments completes an event, it returns true.
     * In this case, the passed I2OChain contains the completed event.
     */
    const bool addFragment(I2OChain&);

    
  private:

    //Prevent copying of the FragmentStore
    FragmentStore(FragmentStore const&);
    FragmentStore& operator=(FragmentStore const&);

    typedef std::map<FragKey, I2OChain> fragmentMap;
    fragmentMap _chains;
    
    
  };
  
} // namespace stor

#endif // StorageManager_FragmentStore_h 


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
