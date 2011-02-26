// $Id: DQMEventSelector.h,v 1.5.2.2 2011/01/24 14:03:20 mommsen Exp $
/// @file: DQMEventSelector.h 

#ifndef EventFilter_StorageManager_DQMEventSelector_h
#define EventFilter_StorageManager_DQMEventSelector_h

#include <boost/shared_ptr.hpp>

#include "FWCore/Framework/interface/EventSelector.h"
#include "EventFilter/StorageManager/interface/I2OChain.h"
#include "EventFilter/StorageManager/interface/DQMEventConsumerRegistrationInfo.h"

namespace stor
{
  /**
   * DQM event selector
   *
   * $Author: mommsen $
   * $Revision: 1.5.2.2 $
   * $Date: 2011/01/24 14:03:20 $
   */

  class DQMEventSelector
  {

  public:
    
    DQMEventSelector( const DQMEventConsRegPtr registrationInfo ):
    _registrationInfo( registrationInfo )
    {};
    
    /**
     * Returns true if the DQM event stored in the I2OChain
     * passes this event selection.
     */
    bool acceptEvent
    (
      const I2OChain&,
      const utils::time_point_t&
    );
    
    /**
     * Returns the ID of the queue corresponding to this selector.
     */
    QueueID queueId() const { return _registrationInfo->queueId(); }

    /**
       Comparison:
    */
    bool operator<(const DQMEventSelector& other) const;

  private:

    const DQMEventConsRegPtr _registrationInfo;

  };

} // namespace stor

#endif // EventFilter_StorageManager_DQMEventSelector_h


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
