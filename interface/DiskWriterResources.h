// $Id: DiskWriterResources.h,v 1.1.2.3 2009/04/03 14:41:21 biery Exp $


#ifndef EventFilter_StorageManager_DiskWriterResources_h
#define EventFilter_StorageManager_DiskWriterResources_h

#include "EventFilter/StorageManager/interface/ErrorStreamConfigurationInfo.h"
#include "EventFilter/StorageManager/interface/EventStreamConfigurationInfo.h"

#include "boost/thread/condition.hpp"
#include "boost/thread/mutex.hpp"

namespace stor
{

  /**
   * Container class for resources that are needed by the DiskWriter
   * and need to be accessed from multiple threads.
   *
   * $Author: biery $
   * $Revision: 1.1.2.3 $
   * $Date: 2009/04/03 14:41:21 $
   */

  class DiskWriterResources
  {
  public:

    /**
     * Constructs a DiskWriterResources instance.
     */
    DiskWriterResources();

    /**
     * Destructor.
     */
    virtual ~DiskWriterResources() {}

    /**
     * Requests that the DiskWriter streams be configured with the
     * specified configurations.  Also allows a new dequeue timeout
     * value to be specified.
     */
    void requestStreamConfiguration(EvtStrConfigList*, ErrStrConfigList*,
                                    double timeoutValue);

    /**
     * Checks if a request has been made to configure the DiskWriter
     * streams *and* clears any pending request.  Supplies the new
     * configurations and a new dequeue timeout value.
     */
    bool streamConfigurationRequested(EvtStrConfigList*&, ErrStrConfigList*&,
                                      double& timeoutValue);

    /**
     * Waits until a requested stream configuration has been completed.
     */
    virtual void waitForStreamConfiguration();

    /**
     * Indicates that the stream configuration operation is done.
     */
    void streamConfigurationDone();

    /**
     * Requests that the DiskWriter streams be destroyed.
     */
    void requestStreamDestruction();

    /**
     * Checks if a request has been made to destroy the DiskWriter
     * streams *and* clears any pending request.
     */
    bool streamDestructionRequested();

    /**
     * Waits until a requested stream destruction has been completed.
     */
    virtual void waitForStreamDestruction();

    /**
     * Indicates that the stream destruction operation is done.
     */
    void streamDestructionDone();

    /**
     * Requests that the DiskWriter check if files need to be closed.
     */
    void requestFileClosingTest();

    /**
     * Checks if a request has been made to run the DiskWriter
     * file closing test *and* clears any pending request.
     */
    bool fileClosingTestRequested();

    /**
     * Sets the DiskWriter "busy" status to the specified state.
     */
    void setBusy(bool isBusyFlag);

    /**
     * Tests if the DiskWriter is currently busy..
     */
    bool isBusy();

  private:

    bool _configurationIsNeeded;
    bool _streamDestructionIsNeeded;
    bool _fileClosingTestIsNeeded;
    bool _diskWriterIsBusy;

    EvtStrConfigList* _requestedEventStreamConfig;
    ErrStrConfigList* _requestedErrorStreamConfig;
    double _requestedTimeout;

    bool _configurationInProgress;
    boost::condition _configurationCondition;
    bool _streamDestructionInProgress;
    boost::condition _destructionCondition;

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
