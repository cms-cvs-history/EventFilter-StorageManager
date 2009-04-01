// $Id: DQMEventConsumerRegistrationInfo.cc,v 1.1.2.4 2009/03/10 21:19:39 biery Exp $

#include "EventFilter/StorageManager/interface/DQMEventConsumerRegistrationInfo.h"
#include "EventFilter/StorageManager/interface/EventDistributor.h"

using namespace std;

namespace stor
{

  DQMEventConsumerRegistrationInfo::DQMEventConsumerRegistrationInfo
  (const std::string& sourceURL,
   const std::string& consumerName,
   unsigned int headerRetryInterval,// seconds
   double maxEventRequestRate, // Hz
   const string& topLevelFolderName,
   QueueID queueId,
   size_t maxQueueSize) :
    _common(sourceURL, consumerName, headerRetryInterval, 
	    maxEventRequestRate, queueId),
    _topLevelFolderName( topLevelFolderName ),
    _maxQueueSize( maxQueueSize )
  { }

  DQMEventConsumerRegistrationInfo::~DQMEventConsumerRegistrationInfo() 
  { }

  void 
  DQMEventConsumerRegistrationInfo::do_registerMe(EventDistributor* evtDist)
  {
    evtDist->registerDQMEventConsumer(this);
  }

  QueueID
  DQMEventConsumerRegistrationInfo::do_queueId() const
  {
    return _common.queueId;
  }

  string
  DQMEventConsumerRegistrationInfo::do_sourceURL() const
  {
    return _common.sourceURL;
  }

  string
  DQMEventConsumerRegistrationInfo::do_consumerName() const
  {
    return _common.consumerName;
  }

  unsigned int
  DQMEventConsumerRegistrationInfo::do_headerRetryInterval() const
  {
    return _common.headerRetryInterval;
  }

  double
  DQMEventConsumerRegistrationInfo::do_maxEventRequestRate() const
  {
    return _common.maxEventRequestRate;
  }

  ostream&
  DQMEventConsumerRegistrationInfo::write(ostream& os) const
  {
    os << "DQMEventConsumerRegistrationInfo:"
       << "\n Source URL: " << _common.sourceURL
       << "\n Consumer name: " << _common.consumerName
       << "\n Header retry interval, seconds: "
       << _common.headerRetryInterval
       << "\n Maximum event request rate, Hz: "
       << _common.maxEventRequestRate
       << "\n Top folder name: " << _topLevelFolderName
       << "\n Queue Id: " << _common.queueId;
    return os;
  }

} // namespace stor
