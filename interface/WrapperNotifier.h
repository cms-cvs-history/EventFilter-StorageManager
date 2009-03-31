// -*- c++ -*-
// $Id: WrapperNotifier.h,v 1.1.2.2 2009/03/31 11:58:00 dshpakov Exp $

#ifndef WRAPPERNOTIFIER_H
#define WRAPPERNOTIFIER_H

// Notifier implementation to be used by StorageManager

#include "EventFilter/StorageManager/interface/Notifier.h"

#include "xdaq/Application.h"
#include "xdaq2rc/RcmsStateNotifier.h"


namespace stor
{

  class WrapperNotifier: public Notifier
  {
    
  public:

    WrapperNotifier( xdaq::Application* app );
    
    void reportNewState( const std::string& stateName );


  private:

    xdaq2rc::RcmsStateNotifier _rcms_notifier;

  };

}

#endif // WRAPPERNOTIFIER_H



/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
