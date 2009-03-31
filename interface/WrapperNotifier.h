// -*- c++ -*-
// $Id: WrapperNotifier.h,v 1.1.2.1 2009/03/25 13:22:35 dshpakov Exp $

#ifndef WRAPPERNOTIFIER_H
#define WRAPPERNOTIFIER_H

// Notifier implementation to be used by StorageManager

#include "EventFilter/StorageManager/interface/Notifier.h"

#include "xdaq2rc/RcmsStateNotifier.h"

class xdata::InfoSpace;

namespace stor
{

  class WrapperNotifier: public Notifier
  {

  public:

    WrapperNotifier( xdaq2rc::RcmsStateNotifier& rcmsNotifier );

    ~WrapperNotifier() {}

    void setupInfospace( xdata::InfoSpace* );

    void reportNewState( const std::string& stateName );

  private:

    xdaq2rc::RcmsStateNotifier _rcms_notifier;
    xdata::InfoSpace* _infospace;

  };

}

#endif // WRAPPERNOTIFIER_H
