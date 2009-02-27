// $Id: EventStreamConfigurationInfo_t.cpp,v 1.1.2.1 2009/02/25 12:38:16 dshpakov Exp $

#include "EventFilter/StorageManager/interface/EventStreamConfigurationInfo.h"

using stor::EventStreamConfigurationInfo;
using namespace std;

int main()
{

  EventStreamConfigurationInfo::FilterList fl;
  fl.push_back( "DiMuon" );
  fl.push_back( "CalibPath" );
  fl.push_back( "DiElectron" );
  fl.push_back( "HighPT" );

  EventStreamConfigurationInfo esci( "A",
				     100,
				     fl,
				     "PhysicsOModule",
				     true,
				     7,
				     7000000 );

  cout << esci << endl;

  return 0;

}
