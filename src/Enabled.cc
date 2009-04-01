#include "EventFilter/StorageManager/interface/DiskWriter.h"
#include "EventFilter/StorageManager/interface/EventDistributor.h"
#include "EventFilter/StorageManager/interface/FragmentStore.h"
#include "EventFilter/StorageManager/interface/StateMachine.h"
#include "EventFilter/StorageManager/interface/SharedResources.h"

#include <iostream>

using namespace std;
using namespace stor;

Enabled::Enabled( my_context c ): my_base(c)
{
  TransitionRecord tr( stateName(), true );
  outermost_context().updateHistory( tr );

  SharedResourcesPtr sharedResources =
    outermost_context().getSharedResources();

  // clear the INIT message collection at begin run
  sharedResources->_initMsgCollection->clear();

  if ( sharedResources->_configuration.get() != 0 )
    {
      // update the run-based configuration parameters
      sharedResources->_configuration->updateRunParams();

      // convert the SM configuration string into ConfigInfo objects
      // and store them for later use
      // !!! This should probably be in the Ready entry action.
      // !!! It is here to avoid accessing the PythonProcessPSet code
      // !!! in the old code and the new code at the same time
      DiskWritingParams dwParams =
        sharedResources->_configuration->getDiskWritingParams();
      EvtStrConfigList evtCfgList;
      ErrStrConfigList errCfgList;
      parseStreamConfiguration(dwParams._streamConfiguration, evtCfgList,
                               errCfgList);
      sharedResources->_configuration->setCurrentEventStreamConfig(evtCfgList);
      sharedResources->_configuration->setCurrentErrorStreamConfig(errCfgList);

      // disk writing begin-run processing
      if ( sharedResources->_serviceManager.get() != 0 )
        {
          sharedResources->_serviceManager->start();
        }
      if ( sharedResources->_diskWriter.get() != 0)
        {
          EventDistributor* ed = outermost_context().getEventDistributor();

          sharedResources->_diskWriter->destroyStreams();
          ed->clearStreams();

          EvtStrConfigList evtCfgList =
            sharedResources->_configuration->getCurrentEventStreamConfig();
          ErrStrConfigList errCfgList =
            sharedResources->_configuration->getCurrentErrorStreamConfig();

          sharedResources->_diskWriter->configureEventStreams(evtCfgList);
          sharedResources->_diskWriter->configureErrorStreams(errCfgList);

          ed->registerEventStreams(evtCfgList);
          ed->registerErrorStreams(errCfgList);
        }
    }
}

Enabled::~Enabled()
{
  TransitionRecord tr( stateName(), false );
  outermost_context().updateHistory( tr );

  // Clear any fragments left in the fragment store
  outermost_context().getFragmentStore()->clear();

  // disk writing end-run processing
  if ( outermost_context().getSharedResources()->_serviceManager.get() != 0 )
    {
      outermost_context().getSharedResources()->_serviceManager->stop();
    }

  // DQM end-run processing
  if ( outermost_context().getSharedResources()->_dqmServiceManager.get() != 0 )
    {
      outermost_context().getSharedResources()->_dqmServiceManager->stop();
    }
}

string Enabled::do_stateName() const
{
  return string( "Enabled" );
}

void Enabled::logReconfigureRequest( const Reconfigure& request )
{
  outermost_context().unconsumed_event( request );
}

/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
