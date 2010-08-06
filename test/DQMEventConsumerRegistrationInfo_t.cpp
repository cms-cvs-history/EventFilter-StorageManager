// $Id: DQMEventConsumerRegistrationInfo_t.cpp,v 1.3 2009/09/16 09:53:24 dshpakov Exp $

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
	      1024,
	      "localhost" );

  std::cout << ecri << std::endl;
  return 0;
}
