// -*- c++ -*-
// $Id: ErrorEventSelector.h,v 1.1.2.4 2009/03/05 22:29:48 biery Exp $

#ifndef ERROREVENTSELECTOR_H
#define ERROREVENTSELECTOR_H

#include <boost/shared_ptr.hpp>

#include "EventFilter/StorageManager/interface/ErrorStreamConfigurationInfo.h"
#include "EventFilter/StorageManager/interface/I2OChain.h"

namespace stor {

  class ErrorEventSelector
  {

  public:

    // Constructor:
    ErrorEventSelector( const ErrorStreamConfigurationInfo& configInfo ):
      _configInfo( configInfo )
    {}

    // Destructor:
    ~ErrorEventSelector() {}

    // Accept event:
    bool acceptEvent( const I2OChain& );

    // Accessors:
    const ErrorStreamConfigurationInfo& configInfo() const { return _configInfo; }

  private:

    ErrorStreamConfigurationInfo _configInfo;

  };

} // namespace stor

#endif
