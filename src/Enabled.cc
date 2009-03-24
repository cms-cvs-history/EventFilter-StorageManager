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

  // clear the INIT message collection at begin run
  outermost_context().getSharedResources()->_initMsgCollection->clear();

  // disk writing begin-run processing
  if ( outermost_context().getSharedResources()->_serviceManager.get() != 0 )
    {
      outermost_context().getSharedResources()->_serviceManager->start();
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
