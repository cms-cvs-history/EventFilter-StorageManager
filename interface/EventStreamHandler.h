// $Id: EventStreamHandler.h,v 1.1.2.5 2009/03/01 20:36:29 biery Exp $

#ifndef StorageManager_EventStreamHandler_h
#define StorageManager_EventStreamHandler_h

#include <string>

#include "IOPool/Streamer/interface/InitMessage.h"

#include "EventFilter/StorageManager/interface/Configuration.h"
#include "EventFilter/StorageManager/interface/EventStreamConfigurationInfo.h"
#include "EventFilter/StorageManager/interface/InitMsgCollection.h"
#include "EventFilter/StorageManager/interface/StreamHandler.h"


namespace stor {

  /**
   * Handle one event stream written to disk.
   *
   * $Author: biery $
   * $Revision: 1.1.2.5 $
   * $Date: 2009/03/01 20:36:29 $
   */
  
  class EventStreamHandler : public StreamHandler
  {
  public:
    
    EventStreamHandler
    (
      const EventStreamConfigurationInfo&,
      SharedResourcesPtr
    );


  private:

    /**
     * Return the stream label
     */
    virtual std::string streamLabel()
    { return _streamConfig.streamLabel(); }

    /**
     * Return a new file handler for the provided event
     */    
    virtual FileHandlerPtr newFileHandler(const I2OChain& event);

    EventStreamConfigurationInfo _streamConfig;
    InitMsgSharedPtr _initMsgView;

  };
  
} // namespace stor

#endif // StorageManager_EventStreamHandler_h 


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
