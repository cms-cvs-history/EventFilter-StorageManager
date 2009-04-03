// -*- c++ -*-
// $Id: MockDiskWriterResources.h,v 1.1.2.1 2009/03/31 19:22:08 mommsen Exp $

#ifndef MOCKDISKWRITERRESOURCES_H
#define MOCKDISKWRITERRESOURCES_H

#include "EventFilter/StorageManager/interface/DiskWriterResources.h"

namespace stor
{

  class MockDiskWriterResources : public DiskWriterResources
  {

  public:

    MockDiskWriterResources() {}

    ~MockDiskWriterResources() {}

    void waitForStreamConfiguration() { return; }

    void waitForStreamDestruction() { return; }

  };

}

#endif // MOCKDISKWRITERRESOURCES_H


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
