// $Id: RegistrationInfoBase.cc,v 1.3.2.6 2011/02/25 09:13:48 mommsen Exp $
/// @file: RegistrationInfoBase.cc

#include "EventFilter/StorageManager/interface/RegistrationInfoBase.h"
#include "EventFilter/StorageManager/interface/Exception.h"
#include "FWCore/Utilities/interface/EDMException.h"

using std::string;

namespace stor
{
  RegistrationInfoBase::RegistrationInfoBase
  (
    const std::string& consumerName,
    const std::string& remoteHost,
    const int& queueSize,
    const enquing_policy::PolicyTag& queuePolicy,
    const utils::duration_t& secondsToStale
  ) :
  _remoteHost(remoteHost),
  _consumerName(consumerName),
  _queueSize(queueSize),
  _queuePolicy(queuePolicy),
  _secondsToStale(secondsToStale),
  _consumerId(0)
  { }

  RegistrationInfoBase::RegistrationInfoBase
  (
    const edm::ParameterSet& pset,
    const std::string& remoteHost,
    const EventServingParams& eventServingParams,
    const bool useEventServingParams
  ) :
  _remoteHost(remoteHost),
  _consumerId(0)
  {
    try
    {
      _consumerName = pset.getUntrackedParameter<std::string>("consumerName");
    }
    catch( edm::Exception& e )
    {
      _consumerName = pset.getUntrackedParameter<std::string>("DQMconsumerName", "Unknown");
    }

    try
    {
      _sourceURL = pset.getParameter<std::string>("sourceURL");
    }
    catch( edm::Exception& e )
    {
      _sourceURL = pset.getUntrackedParameter<std::string>("sourceURL", "Unknown");
    }

    const double maxEventRequestRate = pset.getUntrackedParameter<double>("maxEventRequestRate", 0);
    if ( maxEventRequestRate > 0 )
      _minEventRequestInterval = utils::seconds_to_duration(1 / maxEventRequestRate);
    else
      _minEventRequestInterval = boost::posix_time::not_a_date_time;

    _maxConnectTries = pset.getUntrackedParameter<int>("maxConnectTries", 300);

    _connectTrySleepTime = pset.getUntrackedParameter<int>("connectTrySleepTime", 10);

    _retryInterval = pset.getUntrackedParameter<int>("retryInterval", 5);

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

  edm::ParameterSet RegistrationInfoBase::getPSet() const
  {
    edm::ParameterSet pset;

    if ( _consumerName != "Unknown" )
      pset.addUntrackedParameter<std::string>("consumerName", _consumerName);

    if ( _sourceURL  != "Unknown" )
      pset.addParameter<std::string>("sourceURL", _sourceURL);

    if ( _maxConnectTries != 300 )
      pset.addUntrackedParameter<int>("maxConnectTries", _maxConnectTries);
    
    if ( _connectTrySleepTime != 10 )
      pset.addUntrackedParameter<int>("connectTrySleepTime", _connectTrySleepTime);

    if ( _retryInterval != 5 )
      pset.addUntrackedParameter<int>("retryInterval", _retryInterval);
    
    if ( _queueSize > 0 )
      pset.addUntrackedParameter<int>("queueSize", _queueSize);
    
    if ( ! _minEventRequestInterval.is_not_a_date_time() )
    {
      const double rate = 1 / utils::duration_to_seconds(_minEventRequestInterval);
      pset.addUntrackedParameter<double>("maxEventRequestRate", rate);
    }

    const double secondsToStale = utils::duration_to_seconds(_secondsToStale);
    if ( secondsToStale > 0 )
      pset.addUntrackedParameter<double>("consumerTimeOut", secondsToStale);

    if ( _queuePolicy == enquing_policy::DiscardNew )
      pset.addUntrackedParameter<std::string>("queuePolicy", "DiscardNew");
    if ( _queuePolicy == enquing_policy::DiscardOld )
      pset.addUntrackedParameter<std::string>("queuePolicy", "DiscardOld");

    do_appendToPSet(pset);

    return pset;
  }

  bool RegistrationInfoBase::operator<(const RegistrationInfoBase& other) const
  {
    if ( queueSize() != other.queueSize() )
      return ( queueSize() < other.queueSize() );
    if ( queuePolicy() != other.queuePolicy() )
      return ( queuePolicy() < other.queuePolicy() );
    return ( secondsToStale() < other.secondsToStale() );
  }

  bool RegistrationInfoBase::operator==(const RegistrationInfoBase& other) const
  {
    return (
      queueSize() == other.queueSize() &&
      queuePolicy() == other.queuePolicy() &&
      secondsToStale() == other.secondsToStale()
    );
  }

  bool RegistrationInfoBase::operator!=(const RegistrationInfoBase& other) const
  {
    return ! ( *this == other );
  }

  void RegistrationInfoBase::queueInfo(std::ostream& os) const
  {
    os << "Queue type: " << _queuePolicy <<
      ", size " << _queueSize << 
      ", timeout " << _secondsToStale.total_seconds() << "s";
  }

  std::ostream& operator<< (std::ostream& os,
                            RegistrationInfoBase const& ri)
  {
    os << "\n Consumer name: " << ri.consumerName()
      << "\n Consumer id: " << ri.consumerId()
      << "\n Source URL: " << ri.sourceURL()
      << "\n Remote Host: " << ri.remoteHost()
      << "\n Queue id: " << ri.queueId()
      << "\n Maximum size of queue: " << ri.queueSize()
      << "\n Policy used if queue is full: " << ri.queuePolicy()
      << "\n Time until queue becomes stale (seconds): " << ri.secondsToStale().total_seconds();
    return os;
  }
}


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
