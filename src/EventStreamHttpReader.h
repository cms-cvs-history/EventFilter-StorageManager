#ifndef STREAMER_EVENTSTREAMHTTPREADER_H
#define STREAMER_EVENTSTREAMHTTPREADER_H

// $Id: EventStreamHttpReader.h,v 1.22 2009/11/05 12:47:40 mommsen Exp $

#include "IOPool/Streamer/interface/EventBuffer.h"
#include "IOPool/Streamer/interface/StreamerInputSource.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "DataFormats/Provenance/interface/ProductRegistry.h"
#include "EventFilter/StorageManager/interface/Utils.h"

#include <vector>
#include <memory>
#include <string>
#include <fstream>

namespace edm
{
  class EventStreamHttpReader : public edm::StreamerInputSource
  {
  public:
    EventStreamHttpReader(edm::ParameterSet const& pset,
		 edm::InputSourceDescription const& desc);
    virtual ~EventStreamHttpReader();

    virtual edm::EventPrincipal* read();
    void readHeader();
    void registerWithEventServer();

  private:
    edm::EventPrincipal* getOneEvent();
    void getOneEventFromEventServer(std::string&);
    edm::EventPrincipal* extractEvent(std::string&);
    void sleepUntilNextRequest();
    void connectToEventServer(std::string&);
    bool extractConsumerId(std::string&);
    void getHeaderFromEventServer(std::string&);
    bool extractInitMsg(std::string&);

    std::string sourceurl_;
    std::string consumerName_;
    std::string consumerPriority_;
    std::string consumerPSetString_;
    int headerRetryInterval_;
    unsigned int consumerId_;

    stor::utils::time_point_t nextRequestTime_;
    stor::utils::duration_t minEventRequestInterval_;
    
    bool endRunAlreadyNotified_;
    bool runEnded_;
    bool alreadySaidHalted_;
    enum
    {
      DEFAULT_MAX_CONNECT_TRIES = 360,
      DEFAULT_CONNECT_TRY_SLEEP_TIME = 10
    };
    int maxConnectTries_;
    int connectTrySleepTime_;
  };

}
#endif

/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
