// $Id: DQMEventConsumerRegistrationInfo_t.cpp,v 1.1.2.2 2009/03/10 12:37:51 dshpakov Exp $

#include "EventFilter/StorageManager/interface/DQMEventConsumerRegistrationInfo.h"

using stor::DQMEventConsumerRegistrationInfo;
using namespace std;

int main()
{
  typedef DQMEventConsumerRegistrationInfo DECRI;
  DECRI ecri( "http://cmsmon.cms:50082/urn:xdaq-application:lid=29",
	      "Test Consumer",
	      5,
	      1.,
	      "*",
	      stor::enquing_policy::DiscardOld,
	      1024);

  cout << ecri << endl;
  return 0;
}
