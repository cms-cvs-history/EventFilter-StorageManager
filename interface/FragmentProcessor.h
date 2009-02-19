// $Id: FragmentProcessor.h,v 1.1.2.4 2009/02/19 13:15:44 mommsen Exp $

#ifndef StorageManager_FragmentProcessor_h
#define StorageManager_FragmentProcessor_h

#include "toolbox/lang/Class.h"
#include "toolbox/task/WaitingWorkLoop.h"

#include "boost/shared_ptr.hpp"

#include "EventFilter/StorageManager/interface/EventDistributor.h"
#include "EventFilter/StorageManager/interface/FragmentQueue.h"
#include "EventFilter/StorageManager/interface/FragmentStore.h"
#include "EventFilter/StorageManager/interface/I2OChain.h"
#include "EventFilter/StorageManager/interface/StateMachine.h"
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
   * $Revision: 1.1.2.4 $
   * $Date: 2009/02/19 13:15:44 $
   */

  class FragmentProcessor : public toolbox::lang::Class
  {
  public:
    
    FragmentProcessor();

    ~FragmentProcessor();
    
    /**
     * The workloop action processing state machine commands from the
     * command queue and handling I2O messages retrieved from the
     * FragmentQueue
     */
    bool processMessages(toolbox::task::WorkLoop*);

    /**
     * Updates the statistics of processed fragments
     */
    void updateStatistics();

    /**
     * Registers a new event consumer with the EventDistributor
     */
    const QueueID registerEventConsumer
    (
      boost::shared_ptr<EventConsumerRegistrationInfo>
    );


  private:

    /**
     * Processes all state machine events in the command queue
     */
    void processAllCommands();

    /**
       Process a single fragment, if there is  place to put it.
     */
    void processOneFragmentIfPossible();

    /**
       Process a single fragment. This should only be called if it has
       already been determined there is a place to put it.
     */
    void processOneFragment();

    EventDistributor                  _eventDistributor;
    StateMachine                      _stateMachine;
    boost::shared_ptr<FragmentQueue>  _fragmentQueue;
    FragmentStore                     _fragmentStore;

    const unsigned int                _timeout; // Waiting time in microseconds.
    bool                              _actionIsActive;
  };
  
} // namespace stor

#endif // StorageManager_FragmentProcessor_h 


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
