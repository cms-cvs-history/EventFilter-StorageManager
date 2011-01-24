// $Id: SharedResources.h,v 1.6 2009/09/29 07:53:30 mommsen Exp $
/// @file: SharedResources.h 

#ifndef EventFilter_StorageManager_SharedResources_h
#define EventFilter_StorageManager_SharedResources_h

#include <string>

#include "boost/shared_ptr.hpp"

#include "EventFilter/StorageManager/interface/CommandQueue.h"
#include "EventFilter/StorageManager/interface/DQMEventQueue.h"
#include "EventFilter/StorageManager/interface/DQMEventQueueCollection.h"
#include "EventFilter/StorageManager/interface/EventQueueCollection.h"
#include "EventFilter/StorageManager/interface/FragmentQueue.h"
#include "EventFilter/StorageManager/interface/RegistrationQueue.h"
#include "EventFilter/StorageManager/interface/StreamQueue.h"


namespace stor {

  class Configuration;
  class DiscardManager;
  class DiskWriterResources;
  class DQMEventProcessorResources;
  class InitMsgCollection;
  class RegistrationCollection;
  class SharedResources;
  class StatisticsReporter;


  /**
   * Container for shared resources.
   *
   * $Author: mommsen $
   * $Revision: 1.6 $
   * $Date: 2009/09/29 07:53:30 $
   */

  struct SharedResources
  {

    // queues
    CommandQueuePtr _commandQueue;
    DQMEventQueuePtr _dqmEventQueue;
    FragmentQueuePtr _fragmentQueue;
    StreamQueuePtr _streamQueue;
    RegistrationQueuePtr _registrationQueue;
    EventQueueCollectionPtr _eventQueueCollection;
    DQMEventQueueCollectionPtr _dqmEventQueueCollection;

    // other
    boost::shared_ptr<Configuration> _configuration;
    boost::shared_ptr<DiscardManager> _discardManager;
    boost::shared_ptr<DiskWriterResources> _diskWriterResources;
    boost::shared_ptr<DQMEventProcessorResources> _dqmEventProcessorResources;
    boost::shared_ptr<InitMsgCollection> _initMsgCollection;
    boost::shared_ptr<StatisticsReporter> _statisticsReporter;
    boost::shared_ptr<RegistrationCollection> _registrationCollection;

    /**
     * Add a Failed state-machine event to the command queue
     */
    void moveToFailedState( xcept::Exception& );

    /**
       Write message to a file in /tmp
       (last resort when everything else fails)
    */
    void localDebug( const std::string& message ) const;

  };

  typedef boost::shared_ptr<SharedResources> SharedResourcesPtr;
  
} // namespace stor

#endif // EventFilter_StorageManager_SharedResources_h 


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
