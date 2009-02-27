// $Id: $

#include "EventFilter/StorageManager/interface/EventConsumerRegistrationInfo.h"

using stor::EventConsumerRegistrationInfo;
using namespace std;

int main()
{

  EventConsumerRegistrationInfo::FilterList fl;
  fl.push_back( "DQM1" );
  fl.push_back( "DQM2" );

  EventConsumerRegistrationInfo ecri( "http://cmsmon.cms:50082/urn:xdaq-application:lid=29",
				      1,
				      1,
				      "Test Consumer",
				      3,
				      10.,
				      fl,
				      "out4DQM" );

  cout << ecri << endl;

  return 0;

}
