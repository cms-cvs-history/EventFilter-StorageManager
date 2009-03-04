// $Id: EventStreamConfigurationInfo_t.cpp,v 1.1.2.2 2009/02/27 12:31:39 dshpakov Exp $

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
