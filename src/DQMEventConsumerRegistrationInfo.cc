// $Id: DQMEventConsumerRegistrationInfo.cc,v 1.9.2.8 2011/02/24 13:36:30 mommsen Exp $
/// @file: DQMEventConsumerRegistrationInfo.cc

#include "EventFilter/StorageManager/interface/DQMEventConsumerRegistrationInfo.h"
#include "EventFilter/StorageManager/interface/EventDistributor.h"
#include "EventFilter/StorageManager/interface/Exception.h"
#include "FWCore/Utilities/interface/EDMException.h"


namespace stor
{

  DQMEventConsumerRegistrationInfo::DQMEventConsumerRegistrationInfo
  (
    const edm::ParameterSet& pset,
    const EventServingParams& eventServingParams,
    const std::string& remoteHost
  ) :
  RegistrationInfoBase(pset, remoteHost, eventServingParams, true)
  {
    parsePSet(pset);
  }

  DQMEventConsumerRegistrationInfo::DQMEventConsumerRegistrationInfo
  (
    const edm::ParameterSet& pset,
    const std::string& remoteHost
  ) :
  RegistrationInfoBase(pset, remoteHost, EventServingParams(), false)
  {
    parsePSet(pset);
  }

  void
  DQMEventConsumerRegistrationInfo::parsePSet(const edm::ParameterSet& pset)
  {
    _topLevelFolderName = pset.getUntrackedParameter<std::string>("topLevelFolderName", "*");
  }
  
  void
  DQMEventConsumerRegistrationInfo::do_appendToPSet(edm::ParameterSet& pset) const
  {
    if ( _topLevelFolderName != "*" )
      pset.addUntrackedParameter<std::string>("topLevelFolderName", _topLevelFolderName);
  }

  void 
  DQMEventConsumerRegistrationInfo::do_registerMe(EventDistributor* evtDist)
  {
    evtDist->registerDQMEventConsumer(this);
  }

  void
  DQMEventConsumerRegistrationInfo::do_eventType(std::ostream& os) const
  {
    os << "Top level folder: " << _topLevelFolderName << "\n";
    queueInfo(os);
  }

  bool
  DQMEventConsumerRegistrationInfo::operator<(const DQMEventConsumerRegistrationInfo& other) const
  {
    if ( topLevelFolderName() != other.topLevelFolderName() )
      return ( topLevelFolderName() < other.topLevelFolderName() );
    return RegistrationInfoBase::operator<(other);
  }

  bool
  DQMEventConsumerRegistrationInfo::operator==(const DQMEventConsumerRegistrationInfo& other) const
  {
    return (
      topLevelFolderName() == other.topLevelFolderName() &&
      RegistrationInfoBase::operator==(other)
    );
  }
  
  bool
  DQMEventConsumerRegistrationInfo::operator!=(const DQMEventConsumerRegistrationInfo& other) const
  {
    return ! ( *this == other );
  }

} // namespace stor

/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
