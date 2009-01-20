// $Id: FragmentProcessor.h,v 1.1.2.1 2009/01/19 18:12:17 mommsen Exp $

/**
 * @file
 * Processes I2O event fragments
 *
 * It pops the next fragment from the FragmentQueue and adds it to the
 * FragmentStore. If this completes the event, it hands it to the 
 * EventDistributor.
 */

#ifndef StorageManager_FragmentProcessor_h
#define StorageManager_FragmentProcessor_h

#include "toolbox/lang/Class.h"

#include "EventFilter/StorageManager/interface/EventDistributor.h"
#include "EventFilter/StorageManager/interface/FragmentQueue.h"
#include "EventFilter/StorageManager/interface/FragmentStore.h"
#include "EventFilter/StorageManager/interface/I2OChain.h"


namespace stor {
  
  class FragmentProcessor : public toolbox::lang::Class
  {
  public:
    
    FragmentProcessor();
    
    ~FragmentProcessor();
    
    /**
     * Pops the next I2O fragment from the FragmentQueue
     */
    void processNextI2OFragment();

    /**
     * Hands the fragment to the current state
     */
    void haveStateProcessFragment(I2OChain&);

    /**
     * Updates the statistics of processed fragments
     */
    void updateStatistics();


  private:

    FragmentQueue _fragmentQueue;
    EventDistributor _eventDistributor;
    FragmentStore _fragmentStore;

    
  };
  
} // namespace stor

#endif // StorageManager_FragmentProcessor_h 


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
