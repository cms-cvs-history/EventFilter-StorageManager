// -*- c++ -*-
// $Id: $

#ifndef WRAPPERNOTIFIER_H
#define WRAPPERNOTIFIER_H

// Notifier implementation to be used by StorageManager

#include "EventFilter/StorageManager/interface/Notifier.h"

#include "xdaq2rc/RcmsStateNotifier.h"

namespace stor
{

  class WrapperNotifier: public Notifier
  {

  public:

    WrapperNotifier( xdaq2rc::RcmsStateNotifier& rcmsNotifier ):
      _rcms_notifier( rcmsNotifier ) {}

    ~WrapperNotifier() {}

    void reportNewState( const std::string& stateName )
    {
     _rcms_notifier.stateChanged( stateName,
				  std::string( "StorageManager is now " ) + stateName );
    }

  private:

    xdaq2rc::RcmsStateNotifier _rcms_notifier;

  };

}

#endif // WRAPPERNOTIFIER_H
