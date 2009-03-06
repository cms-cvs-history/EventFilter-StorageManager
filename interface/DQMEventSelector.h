// -*- c++ -*-
// $Id: $

#ifndef DQMEVENTSELECTOR_H
#define DQMEVENTSELECTOR_H

#include <boost/shared_ptr.hpp>

#include "FWCore/Framework/interface/EventSelector.h"
#include "IOPool/Streamer/interface/InitMessage.h"

#include "EventFilter/StorageManager/interface/I2OChain.h"

namespace stor
{

  class DQMEventSelector
  {

  public:

    DQMEventSelector() {}
    ~DQMEventSelector() {}
  };

}

#endif
