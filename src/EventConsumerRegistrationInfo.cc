// $Id: $

#include "EventFilter/StorageManager/interface/EventConsumerRegistrationInfo.h"

using stor::EventConsumerRegistrationInfo;
using namespace std;

ostream&
stor::operator << ( ostream& os,
                    const EventConsumerRegistrationInfo& ri )
{

  os << "EventConsumerRegistrationInfo:" << endl
     << " Source URL: " << ri.sourceURL() << endl
     << " Maximum number of connection attempts: "
     << ri.maxConnectRetries() << endl
     << " Connection retry interval, seconds: "
     << ri.connectRetryInterval() << endl
     << " Consumer name: " << ri.consumerName() << endl
     << " Header retry interval, seconds: "
     << ri.headerRetryInterval() << endl
     << " Maximum event request rate, Hz: "
     << ri.maxEventRequestRate() << endl
     << " HLT output: " << ri.selHLTOut() << endl
     << " Event filters:";

  for( unsigned int i = 0; i < ri.selEvents().size(); ++i )
    {
      os << endl << "  " << ri.selEvents()[i];
    }

  return os;

}
