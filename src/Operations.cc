#include "EventFilter/StorageManager/interface/StateMachine.h"

#include <iostream>

namespace stor
{

  Operations::~Operations() {}

  void
  Operations::processI2OFragment(I2OChain& frag,
				 EventDistributor& ed,
				 FragmentStore& fs)
  {
    do_processI2OFragment(frag, ed, fs);
  }

  std::string
  Operations::stateName() const
  {
    return do_stateName();
  }

  ////////////////////////////////////////////////////////////
  // Default implementation for (some) virtual functions.
  ////////////////////////////////////////////////////////////

  void 
  Operations::do_processI2OFragment(I2OChain&         /* unused */ ,
				    EventDistributor& /* unused */ ,
				    FragmentStore&    /* unused */ )
  {
    // What should we really do here? 
    std::cerr << "Error: "
	      << stateName()
	      << " state cannot handle I2O messages\n";
  }
}
