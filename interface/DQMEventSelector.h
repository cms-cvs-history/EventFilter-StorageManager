// $Id: DQMEventSelector.h,v 1.5 2010/12/17 18:21:04 mommsen Exp $
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
   * $Revision: 1.5 $
   * $Date: 2010/12/17 18:21:04 $
   */

  class DQMEventSelector
  {

  public:
    
    DQMEventSelector( const DQMEventConsumerRegistrationInfo* registrationInfo ):
    _registrationInfo( *registrationInfo ),
    _stale( false )
    {};
    
    /**
     * Returns true if the DQM event stored in the I2OChain
     * passes this event selection.
     */
    bool acceptEvent( const I2OChain& );
    
    /**
     * Returns the ID of the queue corresponding to this selector.
     */
    QueueID queueId() const { return _registrationInfo.queueId(); }
    
    /**
       Check if stale:
    */
    bool isStale() const { return _stale; }

    /**
       Mark as stale:
    */
    void markAsStale() { _stale = true; }

    /**
       Mark as active:
    */
    void markAsActive() { _stale = false; }

    /**
       Comparison:
    */
    bool operator<(const DQMEventSelector& other) const;

  private:

    const DQMEventConsumerRegistrationInfo _registrationInfo;
    bool _stale;

  };

} // namespace stor

#endif // EventFilter_StorageManager_DQMEventSelector_h


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
