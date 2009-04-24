// $Id: DQMEventProcessorResources.h,v 1.1.2.1 2009/04/23 19:17:56 mommsen Exp $


#ifndef EventFilter_StorageManager_DQMEventProcessorResources_h
#define EventFilter_StorageManager_DQMEventProcessorResources_h

#include "EventFilter/StorageManager/interface/Configuration.h"

#include "boost/thread/condition.hpp"
#include "boost/thread/mutex.hpp"

namespace stor
{

  /**
   * Container class for resources that are needed by the DQMEventProcessor
   * and need to be accessed from multiple threads.
   *
   * $Author: mommsen $
   * $Revision: 1.1.2.1 $
   * $Date: 2009/04/23 19:17:56 $
   */

  class DQMEventProcessorResources
  {
  public:

    /**
     * Constructs a DQMEventProcessorResources instance.
     */
    DQMEventProcessorResources();

    /**
     * Destructor.
     */
    virtual ~DQMEventProcessorResources() {}

    /**
     * Requests that the DQMEventProcessor be configured with the
     * specified DQMProcessingParams.  Also allows a new dequeue timeout
     * value to be specified.
     */
    void requestConfiguration(DQMProcessingParams, double timeoutValue);

    /**
     * Checks if a request has been made to configure the DQMEventProcessor
     * *and* clears any pending request.  Supplies the new
     * DQMProcessingParams and a new dequeue timeout value.
     */
    bool configurationRequested(DQMProcessingParams&, double& timeoutValue);

    /**
     * Waits until a requested configuration has been completed.
     */
    virtual void waitForConfiguration();

    /**
     * Indicates that the configuration operation is done.
     */
    void configurationDone();

    /**
     * Requests that the DQMEventStore is emptied.
     */
    void requestStoreDestruction();

    /**
     * Checks if a request has been made to empty the DQMEventStore.
     */
    bool storeDestructionRequested();

    /**
     * Waits until a requested storeDestruction has been completed.
     */
    virtual void waitForStoreDestruction();

    /**
     * Indicates that the storeDestruction operation is done.
     */
    void storeDestructionDone();

    /**
     * Requests the end-of-run processing
     */
    void requestEndOfRun();

    /**
     * Checks if a request has been made to end the run
     * *and* clears any pending request.
     */
    bool endOfRunRequested();

    /**
     * Waits until a requested end-of-run processing has been completed.
     */
    virtual void waitForEndOfRun();

    /**
     * Returns true if a requested end-of-run processing has been done.
     */
    virtual bool isEndOfRunDone();

    /**
     * Indicates that the stream destruction operation is done.
     */
    void endOfRunDone();


  private:

    bool _configurationIsNeeded;
    bool _endOfRunIsNeeded;
    bool _storeDestructionIsNeeded;

    DQMProcessingParams _requestedDQMProcessingParams;
    double _requestedTimeout;

    bool _configurationInProgress;
    boost::condition _configurationCondition;
    bool _endOfRunInProgress;
    boost::condition _endOfRunCondition;
    bool _storeDestructionInProgress;
    boost::condition _storeDestructionCondition;

    mutable boost::mutex _generalMutex;
  };

}

#endif

/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
