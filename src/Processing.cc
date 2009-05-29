// $Id: Processing.cc,v 1.1.2.23 2009/05/15 15:57:36 dshpakov Exp $

#include "EventFilter/StorageManager/interface/EventDistributor.h"
#include "EventFilter/StorageManager/interface/FragmentStore.h"
#include "EventFilter/StorageManager/interface/I2OChain.h"
#include "EventFilter/StorageManager/interface/Notifier.h"
#include "EventFilter/StorageManager/interface/SharedResources.h"
#include "EventFilter/StorageManager/interface/StateMachine.h"

#include <iostream>
#include <sstream>


using namespace std;
using namespace stor;

Processing::Processing( my_context c ): my_base(c)
{

  TransitionRecord tr( stateName(), true );
  outermost_context().updateHistory( tr );
  outermost_context().setExternallyVisibleState( "Enabled" );
  outermost_context().getNotifier()->reportNewState( "Enabled" );

}

Processing::~Processing()
{
  TransitionRecord tr( stateName(), false );
  outermost_context().updateHistory( tr );
}

string Processing::do_stateName() const
{
  return string( "Processing" );
}

void Processing::logQueuesEmptyRequest( const QueuesEmpty& request )
{
  outermost_context().unconsumed_event( request );
}

void Processing::logEndRunRequest( const EndRun& request )
{
  outermost_context().unconsumed_event( request );
}

void
Processing::do_processI2OFragment( I2OChain& frag ) const
{
  static unsigned int noFragmentCount;

  bool completed = outermost_context().getFragmentStore()->addFragment(frag);
  if ( completed )
  {
    outermost_context().getSharedResources()->_discardManager->sendDiscardMessage(frag);
    outermost_context().getEventDistributor()->addEventToRelevantQueues(frag);
  }
  else
  {
    // Only do the check every 100th fragment
    // TODO: shall we make this number configurable?
    ++noFragmentCount;
    if ( noFragmentCount % 100 )
    {
      this->noFragmentToProcess();
    }
  }
}

void
Processing::do_noFragmentToProcess() const
{
  //std::cout << stateName() << "::noFragmentToProcess()" << std::endl;
  I2OChain staleEvent;
  bool gotStaleEvent = 
    outermost_context().getFragmentStore()->getStaleEvent(staleEvent, 5);
    // TODO: make the timeout configurable
  if ( gotStaleEvent )
  {
    outermost_context().getSharedResources()->_discardManager->sendDiscardMessage(staleEvent);
    outermost_context().getEventDistributor()->addEventToRelevantQueues(staleEvent);
  }
  outermost_context().getEventDistributor()->checkForStaleConsumers();
}

/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
