// $Id: DQMEventConsumerRegistrationInfo.cc,v 1.1.2.5 2009/04/01 18:44:55 paterno Exp $

#include "EventFilter/StorageManager/interface/DQMEventConsumerRegistrationInfo.h"
#include "EventFilter/StorageManager/interface/EventDistributor.h"

using namespace std;

namespace stor
{

  DQMEventConsumerRegistrationInfo::DQMEventConsumerRegistrationInfo
  ( const std::string& consumerName,
    unsigned int headerRetryInterval,// seconds
    double maxEventRequestRate, // Hz
    const string& topLevelFolderName,
    QueueID queueId,
    size_t maxQueueSize ) :
    _common( consumerName, headerRetryInterval, 
	     maxEventRequestRate, queueId ),
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
