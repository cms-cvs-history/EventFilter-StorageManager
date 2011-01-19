// $Id: DQMEventConsumerRegistrationInfo.cc,v 1.9.2.3 2011/01/14 18:30:22 mommsen Exp $
/// @file: DQMEventConsumerRegistrationInfo.cc

#include "EventFilter/StorageManager/interface/DQMEventConsumerRegistrationInfo.h"
#include "EventFilter/StorageManager/interface/EventDistributor.h"

namespace stor
{

  DQMEventConsumerRegistrationInfo::DQMEventConsumerRegistrationInfo
  (
    const std::string& consumerName,
    const std::string& remoteHost,
    const std::string& topLevelFolderName,
    const int& queueSize,
    const enquing_policy::PolicyTag& queuePolicy,
    const utils::duration_t& secondsToStale
  ) :
  _common( consumerName, remoteHost, queueSize, queuePolicy, secondsToStale),
  _topLevelFolderName( topLevelFolderName )
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
    return _common._queueId;
  }

  void
  DQMEventConsumerRegistrationInfo::do_setQueueId(QueueID const& id)
  {
    _common._queueId = id;
  }

  std::string
  DQMEventConsumerRegistrationInfo::do_consumerName() const
  {
    return _common._consumerName;
  }

  std::string
  DQMEventConsumerRegistrationInfo::do_remoteHost() const
  {
    return _common._remoteHost;
  }

  ConsumerID
  DQMEventConsumerRegistrationInfo::do_consumerId() const
  {
    return _common._consumerId;
  }

  void
  DQMEventConsumerRegistrationInfo::do_setConsumerId(ConsumerID const& id)
  {
    _common._consumerId = id;
  }

  int
  DQMEventConsumerRegistrationInfo::do_queueSize() const
  {
    return _common._queueSize;
  }

  enquing_policy::PolicyTag
  DQMEventConsumerRegistrationInfo::do_queuePolicy() const
  {
    return _common._queuePolicy;
  }

  utils::duration_t
  DQMEventConsumerRegistrationInfo::do_secondsToStale() const
  {
    return _common._secondsToStale;
  }

  bool
  DQMEventConsumerRegistrationInfo::operator<(const DQMEventConsumerRegistrationInfo& other) const
  {
    if ( _topLevelFolderName != other.topLevelFolderName() )
      return ( _topLevelFolderName < other.topLevelFolderName() );
    if ( _common._queueSize != other.queueSize() )
      return ( _common._queueSize < other.queueSize() );
    if ( _common._queuePolicy != other.queuePolicy() )
      return ( _common._queuePolicy < other.queuePolicy() );
    return ( _common._secondsToStale < other.secondsToStale() );
  }

  bool
  DQMEventConsumerRegistrationInfo::operator==(const DQMEventConsumerRegistrationInfo& other) const
  {
    return (
      _topLevelFolderName == other.topLevelFolderName() &&
      _common._queueSize == other.queueSize() &&
      _common._queuePolicy == other.queuePolicy() &&
      _common._secondsToStale == other.secondsToStale()
    );
  }

  bool
  DQMEventConsumerRegistrationInfo::operator!=(const DQMEventConsumerRegistrationInfo& other) const
  {
    return ! ( *this == other );
  }

  std::ostream&
  DQMEventConsumerRegistrationInfo::write(std::ostream& os) const
  {
    os << "DQMEventConsumerRegistrationInfo:"
       << _common
       << "\n Top folder name: " << _topLevelFolderName;
    return os;
  }

} // namespace stor

/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
