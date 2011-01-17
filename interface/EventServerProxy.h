// $Id: EventServerProxy.h,v 1.2 2010/05/11 17:57:27 mommsen Exp $
/// @file: EventServerProxy.h

#ifndef StorageManager_EventServerProxy_h
#define StorageManager_EventServerProxy_h

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
   * $Revision: 1.2 $
   * $Date: 2010/05/11 17:57:27 $
   */
 
  class EventServerProxy
  {

  public:

    EventServerProxy(edm::ParameterSet const&);
    virtual ~EventServerProxy() {};

    /**
     * Get one event from the the event server.
     */
    void getOneEvent(std::string& data);

    /**
     * Get the init message from the the event server.
     */
    void getInitMsg(std::string& data);

    
  private:

    void getOneEventFromEventServer(std::string&);
    void checkEvent(std::string&);
    void sleepUntilNextRequest();
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

#endif // StorageManager_EventServerProxy_h


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
