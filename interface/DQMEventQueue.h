// $Id: DQMEventQueue.h,v 1.1.2.4 2009/04/16 10:51:10 mommsen Exp $

#ifndef StorageManager_DQMEventQueue_h
#define StorageManager_DQMEventQueue_h

#include "EventFilter/StorageManager/interface/ConcurrentQueue.h"
#include "EventFilter/StorageManager/interface/I2OChain.h"

namespace stor {

  /**
   * Queue holding I2OChains of complete DQM events (histograms)
   * waiting to be processed by the DQMEventProcessor
   *
   * $Author: mommsen $
   * $Revision: 1.1.2.4 $
   * $Date: 2009/04/16 10:51:10 $
   */

  typedef ConcurrentQueue< I2OChain, RejectNewest<I2OChain> > DQMEventQueue;  
  
} // namespace stor

#endif // StorageManager_DQMEventQueue_h 


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
