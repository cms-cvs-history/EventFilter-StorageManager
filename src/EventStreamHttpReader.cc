// $Id: EventStreamHttpReader.cc,v 1.42.2.5 2011/01/19 13:50:38 mommsen Exp $
/// @file: EventStreamHttpReader.cc

#include "EventFilter/StorageManager/interface/CurlInterface.h"
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
  _eventServerProxy(ps)
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
    stor::CurlInterface::Content data;

    _eventServerProxy.getOneEvent(data);
    if ( data.empty() ) return 0;

    HeaderView hdrView(&data[0]);
    if (hdrView.code() == Header::DONE)
    {
      setEndRun();
      return 0;
    }

    EventMsgView eventView(&data[0]);
    return deserializeEvent(eventView);
  }
  
  void EventStreamHttpReader::readHeader()
  {
    stor::CurlInterface::Content data;
    
    _eventServerProxy.getInitMsg(data);
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
