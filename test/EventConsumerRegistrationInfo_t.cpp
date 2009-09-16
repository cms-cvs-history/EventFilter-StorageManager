// $Id: EventConsumerRegistrationInfo_t.cpp,v 1.2 2009/06/10 08:15:29 dshpakov Exp $

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
	     "hltOutputDQM",
             id1.index(),
             id1.policy(),
	     10,
	     "localhost" );
  ecri.setQueueID( id1 );
	     

  cout << ecri << endl;

  return 0;

}
