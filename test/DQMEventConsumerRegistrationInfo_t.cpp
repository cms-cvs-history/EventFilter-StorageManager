// $Id: DQMEventConsumerRegistrationInfo_t.cpp,v 1.4 2010/08/06 20:24:32 wmtan Exp $

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
              boost::posix_time::seconds(1024),
	      "localhost" );

  std::cout << ecri << std::endl;
  return 0;
}
