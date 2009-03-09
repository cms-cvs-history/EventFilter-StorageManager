// -*- c++ -*-
// $Id: ErrorStreamSelector.h,v 1.1.2.1 2009/03/06 19:17:21 biery Exp $

#ifndef ERRORSTREAMSELECTOR_H
#define ERRORSTREAMSELECTOR_H

#include <boost/shared_ptr.hpp>

#include "EventFilter/StorageManager/interface/ErrorStreamConfigurationInfo.h"
#include "EventFilter/StorageManager/interface/I2OChain.h"

namespace stor {

  class ErrorStreamSelector
  {

  public:

    // Constructor:
    ErrorStreamSelector( const ErrorStreamConfigurationInfo& configInfo ):
      _configInfo( configInfo )
    {}

    // Destructor:
    ~ErrorStreamSelector() {}

    // Accept event:
    bool acceptEvent( const I2OChain& );

    // Accessors:
    const ErrorStreamConfigurationInfo& configInfo() const { return _configInfo; }

  private:

    ErrorStreamConfigurationInfo _configInfo;

  };

} // namespace stor

#endif
