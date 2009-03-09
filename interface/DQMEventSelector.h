// -*- c++ -*-
// $Id: DQMEventSelector.h,v 1.1.2.1 2009/03/06 16:36:40 dshpakov Exp $

#ifndef DQMEVENTSELECTOR_H
#define DQMEVENTSELECTOR_H

#include <boost/shared_ptr.hpp>

#include "FWCore/Framework/interface/EventSelector.h"
#include "IOPool/Streamer/interface/InitMessage.h"

#include "EventFilter/StorageManager/interface/I2OChain.h"

namespace stor
{
  /**
   * DQM event selector (tbd: better description)
   *
   * $Author: biery $
   * $Revision: 1.1.2.5 $
   * $Date: 2009/03/01 20:36:29 $
   */

  class DQMEventSelector
  {

  public:

    DQMEventSelector() {}
    ~DQMEventSelector() {}
  };

}

#endif
