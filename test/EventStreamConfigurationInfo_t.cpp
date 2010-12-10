// $Id: EventStreamConfigurationInfo_t.cpp,v 1.6 2010/08/06 20:24:32 wmtan Exp $

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
                                     1 );

  std::cout << esci << std::endl;

  return 0;

}
