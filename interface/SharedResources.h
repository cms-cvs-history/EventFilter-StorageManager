// $Id: SharedResources.h,v 1.1.2.4 2009/02/20 20:54:52 biery Exp $

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
   * $Revision: 1.1.2.4 $
   * $Date: 2009/02/20 20:54:52 $
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
