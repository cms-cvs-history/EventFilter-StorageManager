// $Id: DQMEventProcessor.h,v 1.1.2.1 2009/01/19 18:12:16 mommsen Exp $

/**
 * @file
 * Processes the DQM event (histograms)
 *
 * It retrieves the next DQM event from the DQMEventQueue,
 * adds up the histograms belonging to one lumi-section, and
 * puts it into the appropriate DQMConsumerQueues.
 * Depending on the configuration, it also writes the histograms
 * to disk every N lumi-sections.
 */

#ifndef StorageManager_DQMEventProcessor_h
#define StorageManager_DQMEventProcessor_h

#include "EventFilter/StorageManager/interface/DQMConsumerQueue.h"
#include "EventFilter/StorageManager/interface/DQMEventQueue.h"
#include "EventFilter/StorageManager/interface/QueueCollection.h"


namespace stor {
  
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

    
  private:

    QueueCollection<DQMConsumerQueue> _dqmConsumerQueueCollection;
    DQMEventQueue _daqEventQueue;

    
  };
  
} // namespace stor

#endif // StorageManager_DQMEventProcessor_h 


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
