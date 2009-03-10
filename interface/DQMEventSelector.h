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
  /**
   * DQM event selector
   *
   * $Author: mommsen $
   * $Revision: 1.1.2.2 $
   * $Date: 2009/03/09 14:26:49 $
   */

  class DQMEventSelector
  {

  public:

    DQMEventSelector() {}
    ~DQMEventSelector() {}
  };

}

#endif
