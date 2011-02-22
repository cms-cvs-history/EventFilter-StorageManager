// $Id: DQMEventServerProxy.h,v 1.1.2.7 2011/02/17 13:17:31 mommsen Exp $
/// @file: DQMEventServerProxy.h

#ifndef EventFilter_StorageManager_DQMEventServerProxy_h
#define EventFilter_StorageManager_DQMEventServerProxy_h

#include "EventFilter/StorageManager/interface/CurlInterface.h"
#include "EventFilter/StorageManager/interface/DQMEventConsumerRegistrationInfo.h"
#include "EventFilter/StorageManager/interface/Utils.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include <string>


namespace stor {

  /**
   * Retrieve DQM (histogram) events from the Storage Manager event server.
   *
   * This does uses a HTTP get using the CURL library. The Storage Manager
   * event server responses with a binary octet-stream.
   *
   * $Author: mommsen $
   * $Revision: 1.1.2.7 $
   * $Date: 2011/02/17 13:17:31 $
   */
 
  class DQMEventServerProxy
  {

  public:

    DQMEventServerProxy(edm::ParameterSet const&);
    virtual ~DQMEventServerProxy() {};

    /**
     * Get one DQM (histogram) event from the event server.
     */
    void getOneDQMEvent(CurlInterface::Content& data);

    /**
     * Try to get one DQM (histogram) event from the event server.
     * If succesful, returns true.
     */
    bool getDQMEventMaybe(CurlInterface::Content& data);
    
    
  private:

    void getOneDQMEventFromDQMEventServer(CurlInterface::Content&);
    void checkDQMEvent(CurlInterface::Content&);
    void registerWithDQMEventServer();
    void connectToDQMEventServer(CurlInterface::Content&);
    bool extractConsumerId(CurlInterface::Content&);

    const DQMEventConsumerRegistrationInfo dcri_;
    
    std::string consumerPSetString_;
    unsigned int consumerId_;
    
    stor::utils::time_point_t nextRequestTime_;
    stor::utils::duration_t minEventRequestInterval_;
    
    bool alreadySaidHalted_;
    bool alreadySaidWaiting_;
    
  };

} // namespace stor

#endif // EventFilter_StorageManager_DQMEventServerProxy_h


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
