#include "EventFilter/StorageManager/interface/StateMachine.h"

#include <iostream>

using namespace std;

DrainingQueues::DrainingQueues( my_context c ): my_base(c)
{
  cout << "Entering " << stateName() << " state" << endl;
}

DrainingQueues::~DrainingQueues()
{
  cout << "Exiting " << stateName() << " state, sending StopDone" << endl;
  post_event( StopDone() );
}

string DrainingQueues::stateName() const
{
  return string( "DrainingQueues" );
}

void DrainingQueues::handleI2OEventMessage() const
{
  cerr << "Error: " << stateName() << " state cannot handle I2O messages" << endl;
}
