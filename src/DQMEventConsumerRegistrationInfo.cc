// $Id: DQMEventConsumerRegistrationInfo.cc,v 1.1.2.2 2009/02/27 13:59:43 dshpakov Exp $

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
     << " Top folder name: " << ri.topLevelFolderName() << endl
     << " Queue Id: " << ri.queueId();
  return os;
}
