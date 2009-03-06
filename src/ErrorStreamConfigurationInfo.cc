// $Id: ErrorStreamConfigurationInfo.cc,v 1.1.2.3 2009/03/03 16:59:26 dshpakov Exp $

#include "EventFilter/StorageManager/interface/ErrorStreamConfigurationInfo.h"

using stor::ErrorStreamConfigurationInfo;
using namespace std;

ostream&
stor::operator << ( ostream& os,
		    const ErrorStreamConfigurationInfo& ci )
{

  os << "ErrorStreamConfigurationInfo:" << endl
     << " Stream label: " << ci.streamLabel() << endl
     << " Maximum file size, MB: " << ci.maxFileSize() << endl
     << " Stream Id: " << ci.streamId() << endl;

  return os;

}
