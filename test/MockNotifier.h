// -*- c++ -*-
// $Id: MockNotifier.h,v 1.1.2.1 2009/03/25 13:22:35 dshpakov Exp $

#ifndef MOCKNOTIFIER_H
#define MOCKNOTIFIER_H

// Notifier implementation to be used by the state machine unit test

#include "EventFilter/StorageManager/interface/Notifier.h"

namespace stor
{

  class MockNotifier: public Notifier
  {

  public:

    MockNotifier() {}
    ~MockNotifier() {}

    void reportNewState( const std::string& stateName ) {}

  };

}

#endif // MOCKNOTIFIER_H
