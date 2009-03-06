// -*- c++ -*-
// $Id: ErrorStreamConfigurationInfo.h,v 1.1.2.5 2009/03/06 18:52:11 biery Exp $

#ifndef ERRORSTREAMCONFIGURATIONINFO_H
#define ERRORSTREAMCONFIGURATIONINFO_H

#include "EventFilter/StorageManager/interface/StreamID.h"

#include <string>
#include <vector>
#include <iostream>

namespace stor
{

  class ErrorStreamConfigurationInfo
  {

  public:

    typedef std::vector<std::string> FilterList;

    // Constructor:
    ErrorStreamConfigurationInfo( const std::string& streamLabel,
				  long long maxFileSize ):
      _streamLabel( streamLabel ),
      _maxFileSize( maxFileSize ),
      _streamId(0)
    {}

    // Destructor:
    ~ErrorStreamConfigurationInfo() {}

    // Accessors:
    const std::string& streamLabel() const { return _streamLabel; }
    long long maxFileSize() const { return _maxFileSize; }
    StreamID streamId() const { return _streamId; }

    // Set stream Id:
    void setStreamId( StreamID sid ) { _streamId = sid; }

    // Output:
    friend std::ostream& operator <<
      ( std::ostream&, const ErrorStreamConfigurationInfo& );

  private:

    std::string _streamLabel;
    long long _maxFileSize;
    StreamID _streamId;

  };

}

#endif
