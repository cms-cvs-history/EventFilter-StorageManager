// $Id: DQMEventSelector.h,v 1.1.2.5 2009/03/10 21:19:38 biery Exp $

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
   * $Author: biery $
   * $Revision: 1.1.2.5 $
   * $Date: 2009/03/10 21:19:38 $
   */

  class DQMEventSelector
  {

  public:
    
    DQMEventSelector( const DQMEventConsumerRegistrationInfo* ri ):
    _topLevelFolderName( ri->topLevelFolderName() ),
    _queueId( ri->queueId() )
    {};
    
    /**
     * Returns true if the DQM event stored in the I2OChain
     * passes this event selection.
     */
    bool acceptEvent( const I2OChain& );
    
    /**
     * Returns the ID of the queue corresponding to this selector.
     */
    const QueueID& queueId() const { return _queueId; }
    
  private:

    std::string _topLevelFolderName;
    QueueID _queueId;

  };

}

#endif


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
