// $Id: ErrorStreamConfigurationInfo.cc,v 1.1.2.1 2009/03/06 19:17:29 biery Exp $

#include "EventFilter/StorageManager/interface/ErrorStreamConfigurationInfo.h"

using stor::ErrorStreamConfigurationInfo;
using namespace std;

ostream&
stor::operator << ( ostream& os,
		    const ErrorStreamConfigurationInfo& ci )
{

  os << "ErrorStreamConfigurationInfo:" << endl
     << " Stream label: " << ci.streamLabel() << endl
     << " Maximum file size, MB: " << ci.maxFileSizeMB() << endl
     << " Stream Id: " << ci.streamId() << endl;

  return os;

}
