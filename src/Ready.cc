#include "EventFilter/StorageManager/interface/SharedResources.h"
#include "EventFilter/StorageManager/interface/StateMachine.h"

#include <iostream>

using namespace std;
using namespace stor;

Ready::Ready( my_context c ): my_base(c)
{
  TransitionRecord tr( stateName(), true );
  outermost_context().updateHistory( tr );

  SharedResourcesPtr sharedResources =
    outermost_context().getSharedResources();

  // update all configuration parameters
  sharedResources->_configuration->updateAllParams();
}

Ready::~Ready()
{
  TransitionRecord tr( stateName(), false );
  outermost_context().updateHistory( tr );
}

string Ready::do_stateName() const
{
  return string( "Ready" );
}

// void Ready::handleI2OEventMessage() const
// {
//   cerr << "Error: " << stateName() << " state cannot handle I2O messages" << endl;
// }

/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
