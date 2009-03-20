// $Id: FRDStreamHandler.h,v 1.1.2.5 2009/03/01 20:36:29 biery Exp $

#ifndef StorageManager_FRDStreamHandler_h
#define StorageManager_FRDStreamHandler_h

#include "EventFilter/StorageManager/interface/ErrorStreamConfigurationInfo.h"
#include "EventFilter/StorageManager/interface/I2OChain.h"
#include "EventFilter/StorageManager/interface/SharedResources.h"
#include "EventFilter/StorageManager/interface/StreamHandler.h"


namespace stor {

  /**
   * Handle one FED Raw Data (error) event stream written to disk.
   *
   * $Author: biery $
   * $Revision: 1.1.2.5 $
   * $Date: 2009/03/01 20:36:29 $
   */
  
  class FRDStreamHandler : public StreamHandler
  {
  public:
    
    FRDStreamHandler
    (
      const ErrorStreamConfigurationInfo&,
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


    ErrorStreamConfigurationInfo _streamConfig;
    
  };
  
} // namespace stor

#endif // StorageManager_FRDStreamHandler_h 


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
