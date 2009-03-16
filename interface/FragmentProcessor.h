// $Id: FragmentProcessor.h,v 1.1.2.9 2009/03/10 21:18:54 biery Exp $

#ifndef StorageManager_FragmentProcessor_h
#define StorageManager_FragmentProcessor_h

#include "toolbox/lang/Class.h"
#include "toolbox/task/WaitingWorkLoop.h"

#include "boost/shared_ptr.hpp"

#include "EventFilter/StorageManager/interface/EventDistributor.h"
#include "EventFilter/StorageManager/interface/FragmentQueue.h"
#include "EventFilter/StorageManager/interface/FragmentStore.h"
#include "EventFilter/StorageManager/interface/I2OChain.h"
#include "EventFilter/StorageManager/interface/QueueID.h"
#include "EventFilter/StorageManager/interface/SharedResources.h"
#include "EventFilter/StorageManager/interface/StateMachine.h"


namespace stor {

  /**
   * Processes I2O event fragments
   *
   * It pops the next fragment from the FragmentQueue and adds it to the
   * FragmentStore. If this completes the event, it hands it to the 
   * EventDistributor.
   *
   * $Author: biery $
   * $Revision: 1.1.2.9 $
   * $Date: 2009/03/10 21:18:54 $
   */

  class FragmentProcessor : public toolbox::lang::Class
  {
  public:
    
    FragmentProcessor( boost::shared_ptr<SharedResources> sr,
                       boost::shared_ptr<StateMachine> sm );

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


  private:

    /**
     * Processes all state machine events in the command queue
     */
    void processAllCommands();

    /**
     * Processes all consumer registrations in the registration queue
     */
    void processAllRegistrations();

    /**
       Process a single fragment, if there is  place to put it.
     */
    void processOneFragmentIfPossible();

    /**
       Process a single fragment. This should only be called if it has
       already been determined there is a place to put it.
     */
    void processOneFragment();

    boost::shared_ptr<SharedResources> _sharedResources;
    boost::shared_ptr<StateMachine>    _stateMachine;
    FragmentStore                      _fragmentStore;
    EventDistributor                   _eventDistributor;

    const unsigned int                 _timeout; // Waiting time in microseconds.
    bool                               _actionIsActive;

    // temporary!
    void closeDiskFilesIfNeeded();
    size_t _fileCheckIntervalStart;
    unsigned int _fileCheckEventCounter;
  };
  
} // namespace stor

#endif // StorageManager_FragmentProcessor_h 


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
