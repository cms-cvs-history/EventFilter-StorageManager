// $Id: FRDStreamHandler.h,v 1.1.2.1 2009/03/20 10:30:16 mommsen Exp $

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
   * $Author: mommsen $
   * $Revision: 1.1.2.1 $
   * $Date: 2009/03/20 10:30:16 $
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

    /**
     * Return the maximum file size for the stream in MB
     */
    virtual const int getStreamMaxFileSize()
    { return _streamConfig.maxFileSizeMB(); }


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
