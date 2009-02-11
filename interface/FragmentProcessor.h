// $Id: FragmentProcessor.h,v 1.1.2.3 2009/01/30 10:49:40 mommsen Exp $

#ifndef StorageManager_FragmentProcessor_h
#define StorageManager_FragmentProcessor_h

#include "toolbox/lang/Class.h"

#include "EventFilter/StorageManager/interface/EventDistributor.h"
#include "EventFilter/StorageManager/interface/FragmentQueue.h"
#include "EventFilter/StorageManager/interface/FragmentStore.h"
#include "EventFilter/StorageManager/interface/I2OChain.h"
#include "EventFilter/StorageManager/interface/Types.h"


namespace stor {

  /**
   * Processes I2O event fragments
   *
   * It pops the next fragment from the FragmentQueue and adds it to the
   * FragmentStore. If this completes the event, it hands it to the 
   * EventDistributor.
   *
   * $Author: mommsen $
   * $Revision: 1.1.2.3 $
   * $Date: 2009/01/30 10:49:40 $
   */

  class FragmentProcessor : public toolbox::lang::Class
  {
  public:
    
    FragmentProcessor();
    
    ~FragmentProcessor();
    
    /**
     * Processes the message queued in the command and
     * fragment queue.
     */
    void processMessages();

    /**
     * Updates the statistics of processed fragments
     */
    void updateStatistics();

    /**
     * Hands the fragment to the current state
     */
    void haveStateProcessFragment(I2OChain&);

    /**
     * Registers a new event consumer with the EventDistributor
     */
    const QueueID registerEventConsumer
    (
      boost::shared_ptr<EventConsumerRegistrationInfo>
    );


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
