// $Id: $

#include "EventFilter/StorageManager/interface/DQMEventConsumerRegistrationInfo.h"

using stor::DQMEventConsumerRegistrationInfo;
using namespace std;

int main()
{

  DQMEventConsumerRegistrationInfo ecri( "http://cmsmon.cms:50082/urn:xdaq-application:lid=29",
				      "Test Consumer",
				      5,
				      1.,
				      "*" );

  cout << ecri << endl;

  return 0;

}
