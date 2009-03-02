// $Id: EventConsumerRegistrationInfo_t.cpp,v 1.1.2.1 2009/02/27 13:25:00 dshpakov Exp $

#include <iostream>

#include "EventFilter/StorageManager/interface/EventConsumerRegistrationInfo.h"


typedef stor::EventConsumerRegistrationInfo ECRI;
using namespace std;

int main()
{

  ECRI::FilterList fl;
  fl.push_back( "DQM1" );
  fl.push_back( "DQM2" );

  ECRI ecri( "http://cmsmon.cms:50082/urn:xdaq-application:lid=29",
	     1,
	     1,
	     "Test Consumer",
	     3,
	     10.,
	     fl,
	     "out4DQM",
	     10,
	     stor::enquing_policy::DiscardOld);

  cout << ecri << endl;

  return 0;

}
