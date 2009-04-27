// $Id: DQMEventConsumerRegistrationInfo_t.cpp,v 1.1.2.5 2009/04/03 12:22:09 dshpakov Exp $

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
	      "*",
              id1.index(),
              id1.policy(),
	      1024 );

  cout << ecri << endl;
  return 0;
}
