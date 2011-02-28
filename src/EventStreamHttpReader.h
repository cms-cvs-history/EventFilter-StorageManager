// $Id: EventStreamHttpReader.h,v 1.22.8.6 2011/02/25 09:13:48 mommsen Exp $
/// @file: EventStreamHttpReader.h

#ifndef StorageManager_EventStreamHttpReader_h
#define StorageManager_EventStreamHttpReader_h

#include "EventFilter/StorageManager/interface/EventConsumerRegistrationInfo.h"
#include "EventFilter/StorageManager/interface/EventServerProxy.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Framework/interface/InputSourceDescription.h"
#include "IOPool/Streamer/interface/StreamerInputSource.h"


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
    $Revision: 1.22.8.6 $
    $Date: 2011/02/25 09:13:48 $
  */

  class EventStreamHttpReader : public edm::StreamerInputSource
  {
  public:
    EventStreamHttpReader
    (
      edm::ParameterSet const&,
      edm::InputSourceDescription const&
    );
    virtual ~EventStreamHttpReader() {};

    virtual EventPrincipal* read();

  private:
    void readHeader();
    
    stor::EventServerProxy<stor::EventConsumerRegistrationInfo> eventServerProxy_;

  };

} // namespace edm

#endif // StorageManager_EventStreamHttpReader_h

/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
