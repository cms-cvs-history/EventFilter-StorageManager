// $Id: EventConsumerRegistrationInfo.cc,v 1.14.2.11 2011/02/25 13:32:41 mommsen Exp $
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
    const edm::ParameterSet& pset,
    const EventServingParams& eventServingParams,
    const std::string& remoteHost
  ) :
  RegistrationInfoBase(pset, remoteHost, eventServingParams, true)
  {
    parsePSet(pset);
  }

  EventConsumerRegistrationInfo::EventConsumerRegistrationInfo
  (
    const edm::ParameterSet& pset,
    const std::string& remoteHost
  ) :
  RegistrationInfoBase(pset, remoteHost, EventServingParams(), false)
  {
    parsePSet(pset);
  }

  void 
  EventConsumerRegistrationInfo::parsePSet(const edm::ParameterSet& pset)
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

    _prescale = pset.getUntrackedParameter<int>("prescale", 1);
    _uniqueEvents = pset.getUntrackedParameter<bool>("uniqueEvents", false);
    _headerRetryInterval = pset.getUntrackedParameter<int>("headerRetryInterval", 5);
  }
  
  
  void
  EventConsumerRegistrationInfo::do_appendToPSet(edm::ParameterSet& pset) const
  {
    pset.addUntrackedParameter<std::string>("SelectHLTOutput", _outputModuleLabel);
    pset.addUntrackedParameter<std::string>("TriggerSelector", _triggerSelection);
    pset.addParameter<Strings>("TrackedEventSelection", _eventSelection);
    pset.addUntrackedParameter<bool>("uniqueEvents", _uniqueEvents);
    pset.addUntrackedParameter<int>("prescale", _prescale);

    if ( _headerRetryInterval != 5 )
      pset.addUntrackedParameter<int>("headerRetryInterval", _headerRetryInterval);
  }

  void 
  EventConsumerRegistrationInfo::do_registerMe(EventDistributor* evtDist)
  {
    evtDist->registerEventConsumer(shared_from_this());
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
    queueInfo(os);
  }
  
  bool
  EventConsumerRegistrationInfo::operator<(const EventConsumerRegistrationInfo& other) const
  {
    if ( outputModuleLabel() != other.outputModuleLabel() )
      return ( outputModuleLabel() < other.outputModuleLabel() );
    if ( triggerSelection() != other.triggerSelection() )
      return ( triggerSelection() < other.triggerSelection() );
    if ( eventSelection() != other.eventSelection() )
      return ( eventSelection() < other.eventSelection() );
    if ( prescale() != other.prescale() )
      return ( prescale() < other.prescale() );
    if ( uniqueEvents() != other.uniqueEvents() )
      return ( uniqueEvents() < other.uniqueEvents() );
    return RegistrationInfoBase::operator<(other);
  }

  bool
  EventConsumerRegistrationInfo::operator==(const EventConsumerRegistrationInfo& other) const
  {
    return (
      outputModuleLabel() == other.outputModuleLabel() &&
      triggerSelection() == other.triggerSelection() &&
      eventSelection() == other.eventSelection() &&
      prescale() == other.prescale() &&
      uniqueEvents() == other.uniqueEvents() &&
      RegistrationInfoBase::operator==(other)
    );
  }

  bool
  EventConsumerRegistrationInfo::operator!=(const EventConsumerRegistrationInfo& other) const
  {
    return ! ( *this == other );
  }

}

/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
