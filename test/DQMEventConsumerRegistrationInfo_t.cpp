// $Id: DQMEventConsumerRegistrationInfo_t.cpp,v 1.1.2.4 2009/04/01 18:44:56 paterno Exp $

#include "EventFilter/StorageManager/interface/DQMEventConsumerRegistrationInfo.h"
#include "EventFilter/StorageManager/interface/QueueID.h"

using stor::DQMEventConsumerRegistrationInfo;
using namespace std;
using stor::QueueID;

int main()
{
  typedef DQMEventConsumerRegistrationInfo DECRI;
  QueueID id1(stor::enquing_policy::DiscardOld, 2);
  DECRI ecri( "Test Consumer",
	      5,
	      1.,
	      "*",
	      id1,
	      1024 );

  cout << ecri << endl;
  return 0;
}
