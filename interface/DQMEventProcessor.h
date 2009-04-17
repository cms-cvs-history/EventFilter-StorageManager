// $Id: DQMEventProcessor.h,v 1.1.2.6 2009/04/16 12:57:04 mommsen Exp $

#ifndef StorageManager_DQMEventProcessor_h
#define StorageManager_DQMEventProcessor_h

#include "toolbox/lang/Class.h"
#include "toolbox/task/WaitingWorkLoop.h"
#include "xdaq/Application.h"

#include <boost/shared_ptr.hpp>

#include "EventFilter/StorageManager/interface/DQMEventConsumerRegistrationInfo.h"
#include "EventFilter/StorageManager/interface/DQMEventStore.h"
#include "EventFilter/StorageManager/interface/QueueID.h"
#include "EventFilter/StorageManager/interface/SharedResources.h"


namespace stor {

  /**
   * Processes the DQM event (histograms)
   *
   * It retrieves the next DQM event from the DQMEventQueue,
   * adds up the histograms belonging to one lumi-section, and
   * puts it into the appropriate DQMConsumerQueues.
   * Depending on the configuration, it also writes the histograms
   * to disk every N lumi-sections.
   *
   * $Author: mommsen $
   * $Revision: 1.1.2.6 $
   * $Date: 2009/04/16 12:57:04 $
   */
  
  class DQMEventProcessor : public toolbox::lang::Class
  {
  public:
    
    DQMEventProcessor(xdaq::Application*, SharedResourcesPtr sr);
    
    ~DQMEventProcessor();

    /**
     * The workloop action taking the next DQM event from the
     * DQMEventQueue, processes it, and puts it into the
     * appropriate DQMConsumerQueues when the lumi-section has 
     * finished.
     */    
    bool processDQMEvents(toolbox::task::WorkLoop*);

    /**
     * Creates and starts the DQM event processing workloop
     */
    void startWorkLoop(std::string workloopName);

    /**
     * Register a new DQM event consumer
     */
    QueueID registerDQMEventConsumer
    (
     DQMEventConsumerRegistrationInfo const&
    );

    /**
     * Create a new DQM event selector
     */
    void makeDQMEventSelector
    (
      QueueID,
      DQMEventConsumerRegistrationInfo const&
    );


  private:

    //Prevent copying of the DQMEventProcessor
    DQMEventProcessor(DQMEventProcessor const&);
    DQMEventProcessor& operator=(DQMEventProcessor const&);

    /**
     * Pops the next DQM event from the DQMEventQueue and
     * adds it to the DQMStore
     */    
    void processNextDQMEvent();

    /**
     * Retrieves the next available complete DQMRecord
     * adds it to the relevant consumer queues
     */    
    void processNextCompletedDQMRecord();


    xdaq::Application*        _app;
    SharedResourcesPtr        _sharedResources;

    unsigned int              _timeout; // Waiting time in seconds.
    bool                      _actionIsActive;

    toolbox::task::WorkLoop*  _processWL;      

    DQMEventStore             _dqmEventStore;

  };
  
} // namespace stor

#endif // StorageManager_DQMEventProcessor_h 


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
