// $Id: $

#include "EventFilter/StorageManager/interface/DQMEventConsumerRegistrationInfo.h"

using stor::DQMEventConsumerRegistrationInfo;
using namespace std;

ostream&
stor::operator << ( ostream& os,
                    const DQMEventConsumerRegistrationInfo& ri )
{

  os << "DQMEventConsumerRegistrationInfo:" << endl
     << " Source URL: " << ri.sourceURL() << endl
     << " Consumer name: " << ri.consumerName() << endl
     << " Header retry interval, seconds: "
     << ri.headerRetryInterval() << endl
     << " Maximum event request rate, Hz: "
     << ri.maxEventRequestRate() << endl
     << " Top level folder name: " << ri.topLevelFolderName();

  return os;

}
