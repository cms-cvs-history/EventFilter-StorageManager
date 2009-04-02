// $Id: DiskWriterResources.h,v 1.1.2.10 2009/04/01 20:02:18 biery Exp $


#ifndef EventFilter_StorageManager_DiskWriterResources_h
#define EventFilter_StorageManager_DiskWriterResources_h

#include "EventFilter/StorageManager/interface/ErrorStreamConfigurationInfo.h"
#include "EventFilter/StorageManager/interface/EventStreamConfigurationInfo.h"

#include "boost/thread/mutex.hpp"

namespace stor
{

  /**
   * Container class for resources that are needed by the DiskWriter
   * and need to be accessed from multiple threads.
   *
   * $Author: biery $
   * $Revision: 1.1.2.10 $
   * $Date: 2009/04/01 20:02:18 $
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
     * specified configurations..
     */
    void requestStreamConfiguration(EvtStrConfigList, ErrStrConfigList);

    /**
     * Checks if a request has been made to configure the DiskWriter
     * streams *and* clears any pending request.
     */
    bool streamConfigurationRequested(EvtStrConfigList&, ErrStrConfigList&);

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

    EvtStrConfigList _requestedEventStreamConfig;
    ErrStrConfigList _requestedErrorStreamConfig;

    mutable boost::mutex _generalMutex;
  };

}

#endif

/// emacs DiskWriterResources
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
