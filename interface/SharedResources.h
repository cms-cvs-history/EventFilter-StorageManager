// $Id: SharedResources.h,v 1.5 2009/08/28 16:41:50 mommsen Exp $
/// @file: SharedResources.h 

#ifndef StorageManager_SharedResources_h
#define StorageManager_SharedResources_h

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
   * $Revision: 1.5 $
   * $Date: 2009/08/28 16:41:50 $
   */

  struct SharedResources
  {

    // queues
    boost::shared_ptr<CommandQueue> _commandQueue;
    boost::shared_ptr<DQMEventQueue> _dqmEventQueue;
    boost::shared_ptr<FragmentQueue> _fragmentQueue;
    boost::shared_ptr<StreamQueue> _streamQueue;
    boost::shared_ptr<RegistrationQueue> _registrationQueue;
    boost::shared_ptr<EventQueueCollection> _eventConsumerQueueCollection;
    boost::shared_ptr<DQMEventQueueCollection> _dqmEventConsumerQueueCollection;

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
    void moveToFailedState( const std::string& reason );

  };

  typedef boost::shared_ptr<SharedResources> SharedResourcesPtr;
  
} // namespace stor

#endif // StorageManager_SharedResources_h 


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
