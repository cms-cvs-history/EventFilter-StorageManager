// $Id: EventStreamHttpReader.cc,v 1.42.2.2 2011/01/14 12:01:57 mommsen Exp $
/// @file: EventStreamHttpReader.cc

#include "EventFilter/StorageManager/src/EventStreamHttpReader.h"
#include "IOPool/Streamer/interface/EventMessage.h"
#include "IOPool/Streamer/interface/InitMessage.h"

#include <string>

namespace edm
{  
  EventStreamHttpReader::EventStreamHttpReader
  (
    ParameterSet const& ps,
    InputSourceDescription const& desc
  ):
  StreamerInputSource(ps, desc),
  stor::EventServerProxy(ps)
  {
    // Default in StreamerInputSource is 'false'
    inputFileTransitionsEachEvent_ =
      ps.getUntrackedParameter<bool>("inputFileTransitionsEachEvent", true);

    readHeader();
  }

  EventStreamHttpReader::~EventStreamHttpReader()
  {
  }

  EventPrincipal* EventStreamHttpReader::read()
  {
    std::string data;

    getOneEvent(data);
    EventMsgView eventView(&data[0]);
    if (eventView.code() == Header::DONE) setEndRun();
    return deserializeEvent(eventView);
  }
  
  
  void EventStreamHttpReader::readHeader()
  {
    std::string data;
    
    getInitMsg(data);
    InitMsgView initView(&data[0]);
    deserializeAndMergeWithRegistry(initView);
  }

} //namespace edm


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
