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

      // old disk writing begin-run processing
      if ( sharedResources->_serviceManager.get() != 0 )
        {
          sharedResources->_serviceManager->start();
        }

      // disk writer and event distributor begin-run processing
      EvtStrConfigList evtCfgList = sharedResources->_configuration->
        getCurrentEventStreamConfig();
      ErrStrConfigList errCfgList = sharedResources->_configuration->
        getCurrentErrorStreamConfig();

      sharedResources->_diskWriterResources->
        requestStreamConfiguration(&evtCfgList, &errCfgList);
      sharedResources->_diskWriterResources->waitForStreamConfiguration();

      EventDistributor* ed = outermost_context().getEventDistributor();
      ed->registerEventStreams(evtCfgList);
      ed->registerErrorStreams(errCfgList);
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

  // request that the streams that are currently configured in the disk
  // writer be destroyed (this has the side effect of closing files)
  outermost_context().getSharedResources()->
    _diskWriterResources->requestStreamDestruction();
  outermost_context().getSharedResources()->
    _diskWriterResources->waitForStreamDestruction();

  // clear the stream selections in the event distributor
  outermost_context().getEventDistributor()->clearStreams();

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
