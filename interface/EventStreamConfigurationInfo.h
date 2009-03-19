// -*- c++ -*-
// $Id: EventStreamConfigurationInfo.h,v 1.1.2.5 2009/03/06 18:52:11 biery Exp $

#ifndef EVENTSTREAMCONFIGURATIONINFO_H
#define EVENTSTREAMCONFIGURATIONINFO_H

#include "EventFilter/StorageManager/interface/StreamID.h"

#include <string>
#include <vector>
#include <iostream>

namespace stor
{

  class EventStreamConfigurationInfo
  {

  public:

    typedef std::vector<std::string> FilterList;

    // Constructor:
    EventStreamConfigurationInfo( const std::string& streamLabel,
				  long long maxFileSize,
				  const FilterList& selEvents,
				  const std::string& outputModuleLabel,
				  bool useCompression,
				  unsigned int compressionLevel,
				  unsigned int maxEventSize ):
      _streamLabel( streamLabel ),
      _maxFileSize( maxFileSize ),
      _selEvents( selEvents ),
      _outputModuleLabel( outputModuleLabel ),
      _useCompression( useCompression ),
      _compressionLevel( compressionLevel ),
      _maxEventSize( maxEventSize ),
      _streamId(0)
    {}

    // Destructor:
    ~EventStreamConfigurationInfo() {}

    // Accessors:
    const std::string& streamLabel() const { return _streamLabel; }
    long long maxFileSize() const { return _maxFileSize; }
    const FilterList& selEvents() const { return _selEvents; }
    const std::string& outputModuleLabel() const { return _outputModuleLabel; }
    bool useCompression() const { return _useCompression; }
    unsigned int compressionLevel() const { return _compressionLevel; }
    unsigned int maxEventSize() const { return _maxEventSize; }
    StreamID streamId() const { return _streamId; }

    // Set stream Id:
    void setStreamId( StreamID sid ) { _streamId = sid; }

    // Output:
    friend std::ostream& operator <<
      ( std::ostream&, const EventStreamConfigurationInfo& );

  private:

    std::string _streamLabel;
    long long _maxFileSize;
    FilterList _selEvents;
    std::string _outputModuleLabel;
    bool _useCompression;
    unsigned int _compressionLevel;
    unsigned int _maxEventSize;
    StreamID _streamId;

  };

}

#endif
