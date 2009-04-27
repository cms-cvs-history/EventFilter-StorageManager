// $Id: EventConsumerRegistrationInfo_t.cpp,v 1.1.2.4 2009/04/03 12:22:09 dshpakov Exp $

#include <iostream>

#include "EventFilter/StorageManager/interface/EventConsumerRegistrationInfo.h"
#include "EventFilter/StorageManager/interface/QueueID.h"


typedef stor::EventConsumerRegistrationInfo ECRI;
using stor::QueueID;
using namespace std;

int main()
{

  ECRI::FilterList fl;
  fl.push_back( "DQM1" );
  fl.push_back( "DQM2" );
  QueueID id1(stor::enquing_policy::DiscardOld, 3);

  ECRI ecri( 1,
	     1,
	     "Test Consumer",
	     fl,
	     "out4DQM",
             id1.index(),
             id1.policy(),
	     10 );
  ecri.setQueueID( id1 );
	     

  cout << ecri << endl;

  return 0;

}
