// $Id: EventConsumerRegistrationInfo.cc,v 1.14.2.6 2011/02/04 13:58:29 mommsen Exp $
/// @file: EventConsumerRegistrationInfo.cc

#include "EventFilter/StorageManager/interface/EventConsumerRegistrationInfo.h"
#include "EventFilter/StorageManager/interface/EventDistributor.h"
#include "EventFilter/StorageManager/interface/Exception.h"
#include "FWCore/Utilities/interface/EDMException.h"

#include <algorithm>
#include <iterator>
#include <ostream>


namespace stor
{
  EventConsumerRegistrationInfo::EventConsumerRegistrationInfo
  (
    const std::string& consumerName,
    const std::string& remoteHost,
    const std::string& triggerSelection,
    const Strings& eventSelection,
    const std::string& outputModuleLabel,
    const int& prescale,
    const bool& uniqueEvents,
    const int& queueSize,
    const enquing_policy::PolicyTag& queuePolicy,
    const utils::duration_t& secondsToStale,
    const utils::duration_t& minEventRequestInterval
  ) :
  _common(consumerName, remoteHost, queueSize, queuePolicy, secondsToStale),
  _triggerSelection( triggerSelection ),
  _eventSelection( eventSelection ),
  _outputModuleLabel( outputModuleLabel ),
  _prescale( prescale ),
  _uniqueEvents( uniqueEvents ),
  _minEventRequestInterval( minEventRequestInterval )
  { }

  EventConsumerRegistrationInfo::EventConsumerRegistrationInfo
  (
    const std::string& consumerName,
    const std::string& remoteHost,
    const edm::ParameterSet& pset,
    const EventServingParams& eventServingParams
  ) :
  _common(consumerName, remoteHost, pset, eventServingParams)
  {
    parsePSet(pset);
  }

  EventConsumerRegistrationInfo::EventConsumerRegistrationInfo
  (
    const std::string& consumerName,
    const std::string& remoteHost,
    const edm::ParameterSet& pset
  ) :
  _common(consumerName, remoteHost, pset, EventServingParams(), false)
  {
    parsePSet(pset);
  }

  void EventConsumerRegistrationInfo::parsePSet(const edm::ParameterSet& pset)
  {
    try
    {
      _outputModuleLabel = pset.getUntrackedParameter<std::string>("SelectHLTOutput");
    }
    catch( edm::Exception& e )
    {
      XCEPT_RAISE( stor::exception::ConsumerRegistration,
        "No HLT output module specified" );
    }

    _triggerSelection = pset.getUntrackedParameter<std::string>("TriggerSelector", "");

    try
    {
      _eventSelection = pset.getParameter<Strings>("TrackedEventSelection");
    }
    catch( edm::Exception& e )
    {
      edm::ParameterSet tmpPSet1 =
        pset.getUntrackedParameter<edm::ParameterSet>("SelectEvents", edm::ParameterSet());
      if ( ! tmpPSet1.empty() )
      {
        _eventSelection = tmpPSet1.getParameter<Strings>("SelectEvents");
      }
    }

    _uniqueEvents = pset.getUntrackedParameter<bool>("uniqueEvents", false);
    _prescale = pset.getUntrackedParameter<int>("prescale", 1);

    double maxEventRequestRate = pset.getUntrackedParameter<double>("maxEventRequestRate", 0);
    if ( maxEventRequestRate > 0 )
      _minEventRequestInterval = utils::seconds_to_duration(1 / maxEventRequestRate);
    else
      _minEventRequestInterval = boost::posix_time::not_a_date_time;
  }

  EventConsumerRegistrationInfo::~EventConsumerRegistrationInfo()
  { }

  edm::ParameterSet
  EventConsumerRegistrationInfo::getPSet() const
  {
    edm::ParameterSet pset;
    pset.addUntrackedParameter<std::string>("SelectHLTOutput", _outputModuleLabel);
    pset.addUntrackedParameter<std::string>("TriggerSelector", _triggerSelection);
    pset.addParameter<Strings>("TrackedEventSelection", _eventSelection);
    pset.addUntrackedParameter<bool>("uniqueEvents", _uniqueEvents);
    pset.addUntrackedParameter<int>("prescale", _prescale);

    if ( ! _minEventRequestInterval.is_not_a_date_time() )
    {
      const double rate = 1 / utils::duration_to_seconds(_minEventRequestInterval);
      pset.addUntrackedParameter<double>("maxEventRequestRate", rate);
    }
    
    if ( _common._queueSize > 0 )
      pset.addUntrackedParameter<int>("queueSize", _common._queueSize);

    const double secondsToStale = utils::duration_to_seconds(_common._secondsToStale);
    if ( secondsToStale > 0 )
      pset.addUntrackedParameter<double>("consumerTimeOut", secondsToStale);

    if ( _common._queuePolicy == enquing_policy::DiscardNew )
      pset.addUntrackedParameter<std::string>("queuePolicy", "DiscardNew");
    if ( _common._queuePolicy == enquing_policy::DiscardOld )
      pset.addUntrackedParameter<std::string>("queuePolicy", "DiscardOld");

    return pset;
  }

  void 
  EventConsumerRegistrationInfo::do_registerMe(EventDistributor* evtDist)
  {
    evtDist->registerEventConsumer(this);
  }

  QueueID
  EventConsumerRegistrationInfo::do_queueId() const
  {
    return _common._queueId;
  }

  void
  EventConsumerRegistrationInfo::do_setQueueId(QueueID const& id)
  {
    _common._queueId = id;
  }

  std::string
  EventConsumerRegistrationInfo::do_consumerName() const
  {
    return _common._consumerName;
  }

  std::string
  EventConsumerRegistrationInfo::do_remoteHost() const
  {
    return _common._remoteHost;
  }

  ConsumerID
  EventConsumerRegistrationInfo::do_consumerId() const
  {
    return _common._consumerId;
  }

  void
  EventConsumerRegistrationInfo::do_setConsumerId(ConsumerID const& id)
  {
    _common._consumerId = id;
  }

  int
  EventConsumerRegistrationInfo::do_queueSize() const
  {
    return _common._queueSize;
  }

  enquing_policy::PolicyTag
  EventConsumerRegistrationInfo::do_queuePolicy() const
  {
    return _common._queuePolicy;
  }

  utils::duration_t
  EventConsumerRegistrationInfo::do_secondsToStale() const
  {
    return _common._secondsToStale;
  }

  bool
  EventConsumerRegistrationInfo::operator<(const EventConsumerRegistrationInfo& other) const
  {
    if ( _outputModuleLabel != other.outputModuleLabel() )
      return ( _outputModuleLabel < other.outputModuleLabel() );
    if ( _triggerSelection != other.triggerSelection() )
      return ( _triggerSelection < other.triggerSelection() );
    if ( _eventSelection != other.eventSelection() )
      return ( _eventSelection < other.eventSelection() );
    if ( _prescale != other.prescale() )
      return ( _prescale < other.prescale() );
    if ( _uniqueEvents != other.uniqueEvents() )
      return ( _uniqueEvents < other.uniqueEvents() );
    if ( _common._queueSize != other.queueSize() )
      return ( _common._queueSize < other.queueSize() );
    if ( _common._queuePolicy != other.queuePolicy() )
      return ( _common._queuePolicy < other.queuePolicy() );
    return ( _common._secondsToStale < other.secondsToStale() );
  }

  bool
  EventConsumerRegistrationInfo::operator==(const EventConsumerRegistrationInfo& other) const
  {
    return (
      _outputModuleLabel == other.outputModuleLabel() &&
      _triggerSelection == other.triggerSelection() &&
      _eventSelection == other.eventSelection() &&
      _prescale == other.prescale() &&
      _uniqueEvents == other.uniqueEvents() &&
      _common._queueSize == other.queueSize() &&
      _common._queuePolicy == other.queuePolicy() &&
      _common._secondsToStale == other.secondsToStale()
    );
  }

  bool
  EventConsumerRegistrationInfo::operator!=(const EventConsumerRegistrationInfo& other) const
  {
    return ! ( *this == other );
  }

  void
  EventConsumerRegistrationInfo::do_eventType(std::ostream& os) const
  {
    os << "Output module: " << _outputModuleLabel << "\n";

    if ( _triggerSelection.empty() )
    {
      if ( ! _eventSelection.empty() )
      {
        os  << "Event Selection: ";
        std::copy(_eventSelection.begin(), _eventSelection.end()-1,
          std::ostream_iterator<Strings::value_type>(os, ","));
        os << *(_eventSelection.end()-1);
      }
    }
    else
      os << "Trigger Selection: " << _triggerSelection;

    if ( _prescale != 1 )
      os << "; prescale: " << _prescale;

    if ( _uniqueEvents != false )
      os << "; uniqueEvents";

    os << "\n";
    _common.queueInfo(os);
  }

  std::ostream& 
  EventConsumerRegistrationInfo::write(std::ostream& os) const
  {
    os << "EventConsumerRegistrationInfo:"
       << _common
       << "\n HLT output: " << _outputModuleLabel
       << "\n Event filters:\n";
    /*
    if (_triggerSelection.size()) {
      os << std::endl << _triggerSelection;
    }
    else 
    */
    std::copy(_eventSelection.begin(), 
              _eventSelection.end(),
              std::ostream_iterator<Strings::value_type>(os, "\n"));
    
    //     for( unsigned int i = 0; i < _eventSelection.size(); ++i )
    //       {
    //         os << '\n' << "  " << _eventSelection[i];
    //       }

    return os;
  }

}

/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
