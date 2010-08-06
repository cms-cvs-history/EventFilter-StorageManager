// $Id: EventStreamConfigurationInfo_t.cpp,v 1.5 2009/12/01 13:58:09 mommsen Exp $

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

  std::string f2 = "DiMuon || CalibPath || DiElectron || HighPT";

  EventStreamConfigurationInfo esci( "A",
				     100,
				     f2,
				     fl,
				     "PhysicsOModule",
				     true,
				     7,
                                     7000000,
                                     1 );

  std::cout << esci << std::endl;

  return 0;

}
