// $Id: DQMEventProcessor.h,v 1.5.2.2 2011/02/23 09:27:07 mommsen Exp $
/// @file: DQMEventProcessor.h 

#ifndef EventFilter_StorageManager_DQMEventProcessor_h
#define EventFilter_StorageManager_DQMEventProcessor_h

#include "toolbox/lang/Class.h"
#include "toolbox/task/WaitingWorkLoop.h"
#include "xdaq/Application.h"

#include <boost/shared_ptr.hpp>

#include "EventFilter/StorageManager/interface/DQMEventStore.h"
#include "EventFilter/StorageManager/interface/I2OChain.h"
#include "EventFilter/StorageManager/interface/SharedResources.h"


namespace stor {

  class QueueID;
  class StatisticsReporter;


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
   * $Revision: 1.5.2.2 $
   * $Date: 2011/02/23 09:27:07 $
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
     * Retrieves all available completed top level folders
     * adds it to the consumer queues
     */    
    void processCompletedTopLevelFolders();

    /**
     * Purge instances and process all completed DQM records
     */    
    void endOfRun();
 

    xdaq::Application*        _app;
    SharedResourcesPtr        _sharedResources;

    boost::posix_time::time_duration _timeout;
    bool                      _actionIsActive;

    toolbox::task::WorkLoop*  _processWL;      

    DQMEventStore<I2OChain,InitMsgCollection> _dqmEventStore;

  };
  
} // namespace stor

#endif // EventFilter_StorageManager_DQMEventProcessor_h 


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
