// $Id: EventStreamConfigurationInfo.cc,v 1.1.2.3 2009/03/03 16:59:26 dshpakov Exp $

#include "EventFilter/StorageManager/interface/EventStreamConfigurationInfo.h"

using stor::EventStreamConfigurationInfo;
using namespace std;

ostream&
stor::operator << ( ostream& os,
		    const EventStreamConfigurationInfo& ci )
{

  os << "EventStreamConfigurationInfo:" << endl
     << " Stream label: " << ci.streamLabel() << endl
     << " Maximum file size, MB: " << ci.maxFileSize() << endl
     << " HLT output: " << ci.outputModuleLabel() << endl
     << " Maximum event size, Bytes: " << ci.maxEventSize() << endl
     << " Stream Id: " << ci.streamId() << endl;

  if( ci.useCompression() )
    {
      os << " Compression applied at level " << ci.compressionLevel() << endl;
    }
  else
    {
      os << " Compression not applied" << endl;
    }

  os << " Event filters:";
  for( unsigned int i = 0; i < ci.selEvents().size(); ++i )
    {
      os << endl << "  " << ci.selEvents()[i];
    }

    return os;

}
