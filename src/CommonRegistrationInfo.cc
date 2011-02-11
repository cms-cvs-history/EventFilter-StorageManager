// $Id: CommonRegistrationInfo.cc,v 1.5.2.3 2011/02/11 12:10:30 mommsen Exp $
/// @file: CommonRegistrationInfo.cc

#include "EventFilter/StorageManager/interface/CommonRegistrationInfo.h"
#include "EventFilter/StorageManager/interface/Exception.h"

using std::string;

namespace stor
{
  CommonRegistrationInfo::CommonRegistrationInfo
  (
    const std::string& consumerName,
    const std::string& remoteHost,
    const int& queueSize,
    const enquing_policy::PolicyTag& queuePolicy,
    const utils::duration_t& secondsToStale
  ) :
  _consumerName(consumerName),
  _remoteHost(remoteHost),
  _queueSize(queueSize),
  _queuePolicy(queuePolicy),
  _secondsToStale(secondsToStale),
  _consumerId(0)
  { }

  CommonRegistrationInfo::CommonRegistrationInfo
  (
    const std::string& consumerName,
    const std::string& remoteHost,
    const edm::ParameterSet& pset,
    const EventServingParams& eventServingParams,
    const bool useEventServingParams
  ) :
  _consumerName(consumerName),
  _remoteHost(remoteHost),
  _consumerId(0)
  {
    _queueSize = pset.getUntrackedParameter<int>("queueSize",
      useEventServingParams ? eventServingParams._consumerQueueSize : 0);

    const std::string policy =
      pset.getUntrackedParameter<std::string>("queuePolicy",
        useEventServingParams ? eventServingParams._consumerQueuePolicy : "Default");
    if ( policy == "DiscardNew" )
    {
      _queuePolicy = enquing_policy::DiscardNew;
    }
    else if ( policy == "DiscardOld" )
    {
      _queuePolicy = enquing_policy::DiscardOld;
    }
    else if ( policy == "Default" )
    {
      _queuePolicy = enquing_policy::Max;
    }
    else
    {
      XCEPT_RAISE( stor::exception::ConsumerRegistration,
        "Unknown enqueuing policy: " + policy );
    }

    _secondsToStale = utils::seconds_to_duration(
      pset.getUntrackedParameter<double>("consumerTimeOut", 0)
    );
    if ( useEventServingParams && _secondsToStale < boost::posix_time::seconds(1) )
      _secondsToStale = eventServingParams._activeConsumerTimeout;
  }

  void CommonRegistrationInfo::queueInfo(std::ostream& os) const
  {
    os << "Queue type: " << _queuePolicy <<
      ", size " << _queueSize << 
      ", timeout " << _secondsToStale.total_seconds() << "s";
  }

  std::ostream& operator<< (std::ostream& os,
                            CommonRegistrationInfo const& ri)
  {
    os << "\n Consumer name: " << ri._consumerName
       << "\n Consumer id: " << ri._consumerId
       << "\n Remote Host: " << ri._remoteHost
       << "\n Queue id: " << ri._queueId
       << "\n Maximum size of queue: " << ri._queueSize
       << "\n Policy used if queue is full: " << ri._queuePolicy
       << "\n Time until queue becomes stale (seconds): " << ri._secondsToStale.total_seconds();
    return os;
  }
}


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
