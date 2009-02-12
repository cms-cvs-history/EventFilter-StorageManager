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

void Operations::processI2OFragment( I2OChain& frag,
				     EventDistributor& ed,
				     FragmentStore& fs ) const
{
  do_processI2OFragment( frag, ed, fs );
}

std::string Operations::stateName() const
{
  return do_stateName();
}

////////////////////////////////////////////////////////////
// Default implementation for (some) virtual functions.
////////////////////////////////////////////////////////////

void 
Operations::do_processI2OFragment( I2OChain&         /* unused */ ,
				   EventDistributor& /* unused */ ,
				   FragmentStore&    /* unused */ ) const
{
  // What should we really do here? 
  std::cerr << "Error: "
	    << stateName()
	    << " state cannot handle I2O messages\n";
}

void
Operations::do_logInvalidEvent( const std::string& eventName,
                                const std::string& currentStateName ) const
{
  std::cerr << "The " << eventName << " event is not supported from the "
            << currentStateName << " state!" << std::endl;
}
