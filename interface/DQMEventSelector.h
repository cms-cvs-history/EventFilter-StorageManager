// -*- c++ -*-
// $Id: DQMEventSelector.h,v 1.1.2.3 2009/03/10 11:29:32 dshpakov Exp $

#ifndef DQMEVENTSELECTOR_H
#define DQMEVENTSELECTOR_H

#include <boost/shared_ptr.hpp>

#include "FWCore/Framework/interface/EventSelector.h"
#include "EventFilter/StorageManager/interface/I2OChain.h"
#include "EventFilter/StorageManager/interface/DQMEventConsumerRegistrationInfo.h"

namespace stor
{
  /**
   * DQM event selector
   *
   * $Author: dshpakov $
   * $Revision: 1.1.2.3 $
   * $Date: 2009/03/10 11:29:32 $
   */

  class DQMEventSelector
  {

  public:

    // Constructor:
    DQMEventSelector( const DQMEventConsumerRegistrationInfo& ri ):
      _regInfo( ri )
    {}

    // Destructor:
    ~DQMEventSelector() {}

    // Accept event:
    bool acceptEvent( const I2OChain& );

    // Accessors:
    const DQMEventConsumerRegistrationInfo& regInfo() const { return _regInfo; }

  private:

    DQMEventConsumerRegistrationInfo _regInfo;

    boost::shared_ptr<edm::EventSelector> _eventSelector;

  };

}

#endif
