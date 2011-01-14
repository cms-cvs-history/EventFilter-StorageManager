// $Id: EventStreamHttpReader.h,v 1.22.8.1 2011/01/13 11:15:48 mommsen Exp $

#ifndef STREAMER_EVENTSTREAMHTTPREADER_H
#define STREAMER_EVENTSTREAMHTTPREADER_H

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
  /**
    Input source for event consumers that will get events from the
    Storage Manager Event Server. This does uses a HTTP get using the
    CURL library. The Storage Manager Event Server responses with
    a binary octet-stream.  The product registry is also obtained
    through a HTTP get.

    There is currently no test of the product registry against
    the consumer client product registry within the code. It should
    already be done if this was inherenting from the standard
    framework input source. Currently we inherit from InputSource.

    $Author: mommsen $
    $Revision: 1.12 $
    $Date: 2010/12/10 19:38:48 $
  */

  class EventStreamHttpReader : public edm::StreamerInputSource
  {
  public:
    EventStreamHttpReader(edm::ParameterSet const& pset,
		 edm::InputSourceDescription const& desc);
    virtual ~EventStreamHttpReader();

    virtual edm::EventPrincipal* read();

  private:
    void readHeader();
    void registerWithEventServer();

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

} // namespace edm

#endif // STREAMER_EVENTSTREAMHTTPREADER_H

/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
