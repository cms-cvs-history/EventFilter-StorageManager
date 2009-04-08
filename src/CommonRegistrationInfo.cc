#include "EventFilter/StorageManager/interface/CommonRegistrationInfo.h"

using std::string;

namespace stor
{
  CommonRegistrationInfo::CommonRegistrationInfo( string const& name,
						  unsigned int hri,
						  double merr,
						  QueueID id ):
    consumerName(name),
    headerRetryInterval(hri),
    maxEventRequestRate(merr),
    queueId(id),
    consumerId(0)
  { }

  std::ostream& operator<< (std::ostream& os,
			    CommonRegistrationInfo const& ri)
  {
    os << "EventConsumerRegistrationInfo:"
       << "\n Consumer name: " << ri.consumerName
       << "\n Consumer id: " << ri.consumerId
       << "\n Header retry interval, seconds: " << ri.headerRetryInterval
       << "\n Maximum event request rate, Hz: " << ri.maxEventRequestRate
       << '\n';
    return os;
  }
}
