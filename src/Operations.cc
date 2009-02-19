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
Operations::do_processI2OFragment( I2OChain& /* unused */ ) const
{
  // What should we really do here? 
  std::cerr << "Error: "
	    << stateName()
	    << " state cannot handle I2O messages\n";
}


void 
Operations::do_noFragmentToProcess() const
{}



/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -

