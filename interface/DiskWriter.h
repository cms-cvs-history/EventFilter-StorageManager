// $Id: DiskWriter.h,v 1.1.2.8 2009/03/26 15:35:46 biery Exp $

#ifndef StorageManager_DiskWriter_h
#define StorageManager_DiskWriter_h

#include "boost/shared_ptr.hpp"
#include "boost/thread/mutex.hpp"

#include <vector>

#include "toolbox/lang/Class.h"
#include "toolbox/task/WaitingWorkLoop.h"

#include "EventFilter/StorageManager/interface/ErrorStreamConfigurationInfo.h"
#include "EventFilter/StorageManager/interface/EventStreamConfigurationInfo.h"
#include "EventFilter/StorageManager/interface/I2OChain.h"
#include "EventFilter/StorageManager/interface/StreamHandler.h"
#include "EventFilter/StorageManager/interface/SharedResources.h"


namespace stor {

  /**
   * Writes events to disk
   *
   * It gets the next event from the StreamQueue and writes it
   * to the appropriate stream file(s) on disk. 
   *
   * $Author: biery $
   * $Revision: 1.1.2.8 $
   * $Date: 2009/03/26 15:35:46 $
   */
  
  class DiskWriter : public toolbox::lang::Class
  {
  public:


    DiskWriter(SharedResourcesPtr sr);


    /**
     * The workloop action taking the next event from the StreamQueue
     * and writing it to disk
     */    
    bool writeNextEvent(toolbox::task::WorkLoop*);

    /**
     * Configures the event streams to be written to disk
     */    
    void configureEventStreams(EvtStrConfigList&);

    /**
     * Configures the error streams to be written to disk
     */    
    void configureErrorStreams(ErrStrConfigList&);

    /**
     * Gracefully close all streams
     */    
    void destroyStreams();

    /**
     * Checks if the disk writer is currently not processing any events.
     */
    const bool empty() const;

    /**
     * Creates and starts the disk writing workloop
     */
    void startWorkLoop(std::string applicationIdentifier);


  private:

    //Prevent copying of the DiskWriter
    DiskWriter(DiskWriter const&);
    DiskWriter& operator=(DiskWriter const&);


    /**
     * Writes the event to the appropriate streams
     */    
    void writeEventToStreams(const I2OChain&);

    /**
     * Creates the handler for the given event stream
     */    
    void makeEventStream(EventStreamConfigurationInfo&);

    /**
     * Creates the handler for the given error event stream
     */    
    void makeErrorStream(ErrorStreamConfigurationInfo&);

    SharedResourcesPtr _sharedResources;

    const unsigned int _timeout; // Timeout in seconds on stream queue

    typedef boost::shared_ptr<StreamHandler> StreamHandlerPtr;
    typedef std::vector<StreamHandlerPtr> StreamHandlers;
    StreamHandlers _streamHandlers;

    mutable boost::mutex _streamConfigMutex;
    
    bool _actionIsActive;
    toolbox::task::WorkLoop* _writingWL;      

  };
  
} // namespace stor

#endif // StorageManager_DiskWriter_h 


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
