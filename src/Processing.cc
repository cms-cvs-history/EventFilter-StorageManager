#include "EventFilter/StorageManager/interface/EventDistributor.h"
#include "EventFilter/StorageManager/interface/FragmentStore.h"
#include "EventFilter/StorageManager/interface/I2OChain.h"
#include "EventFilter/StorageManager/interface/StateMachine.h"

#include <iostream>

using namespace std;
using namespace stor;

unsigned int Processing::_counter = 0;

Processing::Processing( my_context c ): my_base(c)
{

  ++_counter;
  if( _counter > 4 )
    {
      cerr << "Error: " << stateName() << " state created too many times" << endl;
      post_event( Fail() );
      return;
    }

  TransitionRecord tr( stateName(), true );
  outermost_context().updateHistory( tr );

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

void Processing::logStopDoneRequest( const StopDone& request )
{
  outermost_context().unconsumed_event( request );
}

void
Processing::do_processI2OFragment( I2OChain& frag ) const
{
  //std::cout << stateName() << "::processI2OFragment()" << std::endl;

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
}

/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
