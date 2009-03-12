// $Id: DQMEventProcessor.h,v 1.1.2.4 2009/03/10 15:32:59 mommsen Exp $

#ifndef StorageManager_DQMEventProcessor_h
#define StorageManager_DQMEventProcessor_h

#include <boost/shared_ptr.hpp>

#include "EventFilter/StorageManager/interface/DQMEventConsumerRegistrationInfo.h"
#include "EventFilter/StorageManager/interface/DQMEventConsumerQueue.h"
#include "EventFilter/StorageManager/interface/EventQueueCollection.h"
#include "EventFilter/StorageManager/interface/DQMEventQueue.h"
#include "EventFilter/StorageManager/interface/QueueID.h"


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
   * $Revision: 1.1.2.4 $
   * $Date: 2009/03/10 15:32:59 $
   */
  
  class DQMEventProcessor
  {
  public:
    
    DQMEventProcessor();
    
    ~DQMEventProcessor();

    /**
     * Pops the next DQM event from the DQMEventQueue, processes it,
     * and puts it in the appropriate DQMConsumerQueues when the
     * lumi-section has finished.
     */    
    void processNextDQMEvent();

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

    EventQueueCollection _dqmEventConsumerQueueCollection;
    DQMEventQueue _dqmEventQueue;

    
  };
  
} // namespace stor

#endif // StorageManager_DQMEventProcessor_h 


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
