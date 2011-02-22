// $Id: EventStreamHttpReader.cc,v 1.42.2.6 2011/02/14 16:53:49 mommsen Exp $
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
    ParameterSet const& pset,
    InputSourceDescription const& desc
  ):
  StreamerInputSource(pset, desc),
  _eventServerProxy(pset)
  {
    // Default in StreamerInputSource is 'false'
    inputFileTransitionsEachEvent_ =
      pset.getUntrackedParameter<bool>("inputFileTransitionsEachEvent", true);

    readHeader();
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
