// $Id: SharedResources.h,v 1.1.2.1 2009/02/22 18:16:34 biery Exp $

#ifndef StorageManager_SharedResources_h
#define StorageManager_SharedResources_h

#include "EventFilter/StorageManager/interface/CommandQueue.h"
#include "EventFilter/StorageManager/interface/DQMEventQueue.h"
#include "EventFilter/StorageManager/interface/FragmentQueue.h"

namespace stor {

  /**
   * Container for shared resources.
   *
   * $Author: biery $
   * $Revision: 1.1.2.1 $
   * $Date: 2009/02/22 18:16:34 $
   */

  struct SharedResources
  {

    boost::shared_ptr<CommandQueue> _commandQueue;
    boost::shared_ptr<DQMEventQueue> _dqmEventQueue;
    boost::shared_ptr<FragmentQueue> _fragmentQueue;

  };
  
} // namespace stor

#endif // StorageManager_SharedResources_h 


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
