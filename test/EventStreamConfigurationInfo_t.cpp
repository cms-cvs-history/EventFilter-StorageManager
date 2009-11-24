// $Id: EventStreamConfigurationInfo_t.cpp,v 1.3 2009/09/11 21:07:07 elmer Exp $

#include "EventFilter/StorageManager/interface/EventStreamConfigurationInfo.h"
#include <iostream>

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
                                     7000000,
                                     1 );

  cout << esci << endl;

  return 0;

}
