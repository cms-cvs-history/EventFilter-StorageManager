// $Id: DQMEventConsumerRegistrationInfo.cc,v 1.9.2.5 2011/02/11 12:10:30 mommsen Exp $
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
  RegistrationInfoBase( consumerName, remoteHost, queueSize, queuePolicy, secondsToStale),
  _topLevelFolderName( topLevelFolderName )
  { }

  DQMEventConsumerRegistrationInfo::~DQMEventConsumerRegistrationInfo() 
  { }

  void 
  DQMEventConsumerRegistrationInfo::do_registerMe(EventDistributor* evtDist)
  {
    evtDist->registerDQMEventConsumer(this);
  }

  bool
  DQMEventConsumerRegistrationInfo::operator<(const DQMEventConsumerRegistrationInfo& other) const
  {
    if ( topLevelFolderName() != other.topLevelFolderName() )
      return ( topLevelFolderName() < other.topLevelFolderName() );
    if ( queueSize() != other.queueSize() )
      return ( queueSize() < other.queueSize() );
    if ( queuePolicy() != other.queuePolicy() )
      return ( queuePolicy() < other.queuePolicy() );
    return ( secondsToStale() < other.secondsToStale() );
  }

  bool
  DQMEventConsumerRegistrationInfo::operator==(const DQMEventConsumerRegistrationInfo& other) const
  {
    return (
      topLevelFolderName() == other.topLevelFolderName() &&
      queueSize() == other.queueSize() &&
      queuePolicy() == other.queuePolicy() &&
      secondsToStale() == other.secondsToStale()
    );
  }

  bool
  DQMEventConsumerRegistrationInfo::operator!=(const DQMEventConsumerRegistrationInfo& other) const
  {
    return ! ( *this == other );
  }

  void
  DQMEventConsumerRegistrationInfo::do_eventType(std::ostream& os) const
  {
    os << "Top level folder: " << _topLevelFolderName << "\n";
    queueInfo(os);
  }

} // namespace stor

/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
