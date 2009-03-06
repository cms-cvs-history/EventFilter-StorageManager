#include "EventFilter/StorageManager/interface/EventSelector.h"
#include "EventFilter/StorageManager/interface/EventStreamConfigurationInfo.h"

using stor::EventStreamConfigurationInfo;
using stor::EventSelector;

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

  EventSelector<EventStreamConfigurationInfo> es( esci );

  return 0;

}
