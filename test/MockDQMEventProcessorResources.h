// -*- c++ -*-
// $Id: MockDQMEventProcessorResources.h,v 1.1.2.1 2009/04/03 14:41:22 biery Exp $

#ifndef MOCKDQMEVENTPROCESSORRESOURCES_H
#define MOCKDQMEVENTPROCESSORRESOURCES_H

#include "EventFilter/StorageManager/interface/DQMEventProcessorResources.h"

namespace stor
{

  class MockDQMEventProcessorResources : public DQMEventProcessorResources
  {

  public:

    MockDQMEventProcessorResources() {}

    ~MockDQMEventProcessorResources() {}

    void waitForConfiguration() { return; }

    void waitForStoreDestruction() { return; }

    void waitForEndOfRun() { return; }

  };

}

#endif // MOCKDQMEVENTPROCESSORRESOURCES_H


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
