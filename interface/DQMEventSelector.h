// -*- c++ -*-
// $Id: DQMEventSelector.h,v 1.1.2.4 2009/03/10 12:37:50 dshpakov Exp $

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
   * $Revision: 1.1.2.4 $
   * $Date: 2009/03/10 12:37:50 $
   */

  class DQMEventSelector
  {

  public:

    // Constructor:
    DQMEventSelector( const DQMEventConsumerRegistrationInfo& ri ):
      _topLevelFolderName( ri.topLevelFolderName() ),
      _queueId( ri.queueId() )
    {}

    // Destructor:
    ~DQMEventSelector() {}

    // Accept event:
    bool acceptEvent( const I2OChain& );

    /**
     * Returns the ID of the queue corresponding to this selector.
     */
    const QueueID& queueId() const { return _queueId; }

  private:

    std::string _topLevelFolderName;
    QueueID _queueId;

    boost::shared_ptr<edm::EventSelector> _eventSelector;

  };

}

#endif
