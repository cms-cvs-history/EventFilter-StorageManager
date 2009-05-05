// $Id: Enabled.cc,v 1.1.2.36 2009/05/05 10:40:39 mommsen Exp $

#include "EventFilter/StorageManager/interface/I2OChain.h"
#include "EventFilter/StorageManager/interface/StateMachine.h"

#include <iostream>

#include <boost/statechart/event_base.hpp>

using namespace std;
using namespace stor;
using namespace boost::statechart;

Operations::Operations()
{
  //TransitionRecord tr( stateName(), true );
  //outermost_context().updateHistory( tr );
}

Operations::~Operations()
{
  //TransitionRecord tr( stateName(), false );
  //outermost_context().updateHistory( tr );
}

void Operations::processI2OFragment( I2OChain& frag ) const
{
  do_processI2OFragment( frag );
}

void Operations::noFragmentToProcess() const
{
  do_noFragmentToProcess();
}

std::string Operations::stateName() const
{
  return do_stateName();
}

////////////////////////////////////////////////////////////
// Default implementation for (some) virtual functions.
////////////////////////////////////////////////////////////

void 
Operations::do_processI2OFragment( I2OChain& frag ) const
{
  //std::cout << stateName() << "::processI2OFragment()" << std::endl;

  // 20-Mar-2009, KAB: if we get a fragment when we are not supposed to get
  // one, should we still send a discard message to the resource broker?
  // If we don't, couldn't that cause problems upstream?  If we do, though,
  // we could end up sending duplicate discard messages (one per fragment).
  // At a minimum, we should make sure that we don't try to use the discard
  // manager before it is available.
  // Of course, if we want to do this, we need to implement a way to get
  // a handle to the discard manager since outermost_context() doesn't
  // actually work here.
  //
  //if ( outermost_context().getSharedResources().get() != 0 &&
  //     outermost_context().getSharedResources()->_discardManager.get() != 0)
  //  {
  //    outermost_context().getSharedResources()->_discardManager->sendDiscardMessage(frag);
  //  }
}


void 
Operations::do_noFragmentToProcess() const
{
  //std::cout << stateName() << "::noFragmentToProcess()" << std::endl;
}



/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -

