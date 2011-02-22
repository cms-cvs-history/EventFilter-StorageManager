// $Id: EventServerProxy.h,v 1.1.2.7 2011/02/17 13:17:31 mommsen Exp $
/// @file: EventServerProxy.h

#ifndef EventFilter_StorageManager_EventServerProxy_h
#define EventFilter_StorageManager_EventServerProxy_h

#include "EventFilter/StorageManager/interface/CurlInterface.h"
#include "EventFilter/StorageManager/interface/EventConsumerRegistrationInfo.h"
#include "EventFilter/StorageManager/interface/Utils.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include <string>


namespace stor {

  /**
   * Retrieve data events from the Storage Manager event server.
   *
   * This does uses a HTTP get using the CURL library. The Storage Manager
   * event server responses with a binary octet-stream. The init message
   * is also obtained through a HTTP get.
   *
   * $Author: mommsen $
   * $Revision: 1.1.2.7 $
   * $Date: 2011/02/17 13:17:31 $
   */
 
  class EventServerProxy
  {

  public:

    EventServerProxy(edm::ParameterSet const&);
    virtual ~EventServerProxy() {};

    /**
     * Get one event from the event server.
     */
    void getOneEvent(CurlInterface::Content& data);

    /**
     * Try to get one event from the event server.
     * If succesful, returns true.
     */
    bool getEventMaybe(CurlInterface::Content& data);

    /**
     * Get the init message from the the event server.
     */
    void getInitMsg(CurlInterface::Content& data);
    
    
  private:

    void getOneEventFromEventServer(CurlInterface::Content&);
    void checkEvent(CurlInterface::Content&);
    void getInitMsgFromEventServer(CurlInterface::Content&);
    void checkInitMsg(CurlInterface::Content&);
    void registerWithEventServer();
    void connectToEventServer(CurlInterface::Content&);
    bool extractConsumerId(CurlInterface::Content&);

    const EventConsumerRegistrationInfo ecri_;
    
    std::string consumerPSetString_;
    unsigned int consumerId_;
    
    stor::utils::time_point_t nextRequestTime_;
    stor::utils::duration_t minEventRequestInterval_;
    
    bool alreadySaidHalted_;
    bool alreadySaidWaiting_;
    
  };

} // namespace stor

#endif // EventFilter_StorageManager_EventServerProxy_h


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
