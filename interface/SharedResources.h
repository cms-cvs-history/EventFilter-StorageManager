// $Id: SharedResources.h,v 1.1.2.5 2009/03/13 17:37:02 biery Exp $

#ifndef StorageManager_SharedResources_h
#define StorageManager_SharedResources_h

#include "EventFilter/StorageManager/interface/CommandQueue.h"
#include "EventFilter/StorageManager/interface/Configuration.h"
#include "EventFilter/StorageManager/interface/DQMEventQueue.h"
#include "EventFilter/StorageManager/interface/FragmentQueue.h"
#include "EventFilter/StorageManager/interface/RegistrationQueue.h"

#include "EventFilter/StorageManager/interface/DiscardManager.h"
#include "EventFilter/StorageManager/interface/InitMsgCollection.h"

#include "EventFilter/StorageManager/interface/EventServer.h"
#include "EventFilter/StorageManager/interface/DQMEventServer.h"
#include "EventFilter/StorageManager/interface/SMFUSenderList.h"
#include "EventFilter/StorageManager/interface/ServiceManager.h"
#include "EventFilter/StorageManager/interface/DQMServiceManager.h"

namespace stor {

  /**
   * Container for shared resources.
   *
   * $Author: biery $
   * $Revision: 1.1.2.5 $
   * $Date: 2009/03/13 17:37:02 $
   */

  struct SharedResources
  {

    // queues
    boost::shared_ptr<CommandQueue> _commandQueue;
    boost::shared_ptr<DQMEventQueue> _dqmEventQueue;
    boost::shared_ptr<FragmentQueue> _fragmentQueue;
    boost::shared_ptr<FragmentQueue> _fragmentQueue2;  // extremely temporary
    boost::shared_ptr<RegistrationQueue> _registrationQueue;

    // other
    boost::shared_ptr<Configuration> _configuration;

    // temporary?
    boost::shared_ptr<DiscardManager> _discardManager;
    boost::shared_ptr<InitMsgCollection> _initMsgCollection;

    // definitely temporary!
    boost::shared_ptr<EventServer> _oldEventServer;
    boost::shared_ptr<DQMEventServer> _oldDQMEventServer;
    SMFUSenderList* _smRBSenderList;
    boost::shared_ptr<edm::ServiceManager> _serviceManager;
    boost::shared_ptr<stor::DQMServiceManager> _dqmServiceManager;

  };
  
} // namespace stor

#endif // StorageManager_SharedResources_h 


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
