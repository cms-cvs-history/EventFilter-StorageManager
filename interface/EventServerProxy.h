// $Id: EventServerProxy.h,v 1.1.2.4 2011/01/26 14:28:28 mommsen Exp $
/// @file: EventServerProxy.h

#ifndef EventFilter_StorageManager_EventServerProxy_h
#define EventFilter_StorageManager_EventServerProxy_h

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
   * $Revision: 1.1.2.4 $
   * $Date: 2011/01/26 14:28:28 $
   */
 
  class EventServerProxy
  {

  public:

    EventServerProxy(edm::ParameterSet const&);
    virtual ~EventServerProxy() {};

    /**
     * Get one event from the event server.
     */
    void getOneEvent(std::string& data);

    /**
     * Try to get one event from the event server.
     * If succesful, returns true.
     */
    bool getEventMaybe(std::string& data);

    /**
     * Get the init message from the the event server.
     */
    void getInitMsg(std::string& data);

    /**
     * Get the source URL
     */
    const std::string& getSourceURL()
    { return sourceurl_; }
    
    
  private:

    void getOneEventFromEventServer(std::string&);
    void checkEvent(std::string&);
    void getInitMsgFromEventServer(std::string&);
    void checkInitMsg(std::string&);
    void registerWithEventServer();
    void connectToEventServer(std::string&);
    bool extractConsumerId(std::string&);

    std::string sourceurl_;
    std::string consumerName_;
    int maxConnectTries_;
    int connectTrySleepTime_;
    int headerRetryInterval_;
    
    std::string consumerPSetString_;
    unsigned int consumerId_;
    
    stor::utils::time_point_t nextRequestTime_;
    stor::utils::duration_t minEventRequestInterval_;
    
    bool endRunAlreadyNotified_;
    bool runEnded_;
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
