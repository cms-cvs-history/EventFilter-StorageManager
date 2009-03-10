// $Id: DQMEventConsumerRegistrationInfo.cc,v 1.1.2.3 2009/03/10 12:37:50 dshpakov Exp $

#include "EventFilter/StorageManager/interface/DQMEventConsumerRegistrationInfo.h"
#include "EventFilter/StorageManager/interface/EventDistributor.h"

using stor::DQMEventConsumerRegistrationInfo;
using namespace std;

void stor::DQMEventConsumerRegistrationInfo::
registerMe( stor::EventDistributor* evtDist )
{
  evtDist->registerDQMEventConsumer( this );
}

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
