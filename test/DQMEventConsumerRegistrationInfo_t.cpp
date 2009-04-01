// $Id: DQMEventConsumerRegistrationInfo_t.cpp,v 1.1.2.3 2009/03/12 03:46:18 paterno Exp $

#include "EventFilter/StorageManager/interface/DQMEventConsumerRegistrationInfo.h"
#include "EventFilter/StorageManager/interface/QueueID.h"

using stor::DQMEventConsumerRegistrationInfo;
using namespace std;
using stor::QueueID;

int main()
{
  typedef DQMEventConsumerRegistrationInfo DECRI;
  QueueID id1(stor::enquing_policy::DiscardOld, 2);
  DECRI ecri( "http://cmsmon.cms:50082/urn:xdaq-application:lid=29",
	      "Test Consumer",
	      5,
	      1.,
	      "*",
	      id1,
	      1024);

  cout << ecri << endl;
  return 0;
}
