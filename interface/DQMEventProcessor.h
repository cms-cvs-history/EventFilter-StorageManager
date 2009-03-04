// $Id: DQMEventProcessor.h,v 1.1.2.3 2009/01/30 10:49:40 mommsen Exp $

#ifndef StorageManager_DQMEventProcessor_h
#define StorageManager_DQMEventProcessor_h

#include <boost/shared_ptr.hpp>

#include "EventFilter/StorageManager/interface/DQMEventConsumerRegistrationInfo.h"
#include "EventFilter/StorageManager/interface/DQMEventConsumerQueue.h"
#include "EventFilter/StorageManager/interface/DQMEventConsumerQueueCollection.h"
#include "EventFilter/StorageManager/interface/DQMEventQueue.h"
#include "EventFilter/StorageManager/interface/Types.h"


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
   * $Revision: 1.1.2.3 $
   * $Date: 2009/01/30 10:49:40 $
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
    const QueueID registerDQMEventConsumer
    (
      boost::shared_ptr<DQMEventConsumerRegistrationInfo>
    );

    /**
     * Create a new DQM event selector
     */
    void makeDQMEventSelector
    (
      const QueueID,
      boost::shared_ptr<DQMEventConsumerRegistrationInfo>
    );
    
  private:

    DQMEventConsumerQueueCollection _dqmEventConsumerQueueCollection;
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
