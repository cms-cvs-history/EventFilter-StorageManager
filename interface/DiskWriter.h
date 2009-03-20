// $Id: DiskWriter.h,v 1.1.2.5 2009/03/01 20:36:29 biery Exp $

#ifndef StorageManager_DiskWriter_h
#define StorageManager_DiskWriter_h

#include "boost/shared_ptr.hpp"

#include <vector>

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
   * $Revision: 1.1.2.5 $
   * $Date: 2009/03/01 20:36:29 $
   */
  
  class DiskWriter
  {
  public:

    typedef std::vector<EventStreamConfigurationInfo> EventStreamConfigurationInfoList;
    typedef std::vector<ErrorStreamConfigurationInfo> ErrorStreamConfigurationInfoList;
    

    DiskWriter(SharedResourcesPtr sr);


    /**
     * Takes the next event from the StreamQueue and writes it to disk
     */    
    void writeNextEvent();

    /**
     * Configures the event streams to be written to disk
     */    
    void configureEventStreams(EventStreamConfigurationInfoList&);

    /**
     * Configures the error streams to be written to disk
     */    
    void configureErrorStreams(ErrorStreamConfigurationInfoList&);

    /**
     * Gracefully close all streams
     */    
    void destroyStreams();

    /**
     * Checks if the disk writer is currently not processing any events.
     */
    const bool empty() const;

  private:

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

    const unsigned int _timeout; // Timeout in microseconds on stream queue

    typedef boost::shared_ptr<StreamHandler> StreamHandlerPtr;
    typedef std::vector<StreamHandlerPtr> StreamHandlers;
    StreamHandlers _streamHandlers;
    
  };
  
} // namespace stor

#endif // StorageManager_DiskWriter_h 


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
