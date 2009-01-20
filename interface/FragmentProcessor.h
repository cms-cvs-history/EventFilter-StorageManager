// $Id: FragmentProcessor.h,v 1.1.2.1 2009/01/19 18:12:17 mommsen Exp $

#ifndef StorageManager_FragmentProcessor_h
#define StorageManager_FragmentProcessor_h

#include "EventFilter/StorageManager/interface/EventDistributor.h"
#include "EventFilter/StorageManager/interface/FragmentStore.h"


namespace stor {
  
  class FragmentProcessor
  {
  public:
    
    FragmentProcessor();
    
    ~FragmentProcessor();
    
    void processNextI2OFragment();

    void processNextStateMachineEvent();

    void updateStatistics();


  private:

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
