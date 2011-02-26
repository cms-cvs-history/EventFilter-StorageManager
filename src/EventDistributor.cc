// $Id: EventDistributor.cc,v 1.23.2.4 2011/02/04 13:57:45 mommsen Exp $
/// @file: EventDistributor.cc

#include "EventFilter/StorageManager/interface/DataSenderMonitorCollection.h"
#include "EventFilter/StorageManager/interface/DQMEventConsumerRegistrationInfo.h"
#include "EventFilter/StorageManager/interface/DQMEventSelector.h"
#include "EventFilter/StorageManager/interface/ErrorStreamConfigurationInfo.h"
#include "EventFilter/StorageManager/interface/ErrorStreamSelector.h"
#include "EventFilter/StorageManager/interface/EventConsumerRegistrationInfo.h"
#include "EventFilter/StorageManager/interface/EventConsumerSelector.h"
#include "EventFilter/StorageManager/interface/EventDistributor.h"
#include "EventFilter/StorageManager/interface/EventStreamConfigurationInfo.h"
#include "EventFilter/StorageManager/interface/EventStreamSelector.h"
#include "EventFilter/StorageManager/interface/I2OChain.h"
#include "EventFilter/StorageManager/interface/InitMsgCollection.h"
#include "EventFilter/StorageManager/interface/QueueID.h"
#include "EventFilter/StorageManager/interface/RegistrationCollection.h"
#include "EventFilter/StorageManager/interface/RunMonitorCollection.h"
#include "EventFilter/StorageManager/interface/StatisticsReporter.h"
#include "EventFilter/StorageManager/interface/Exception.h"

#include "EventFilter/Utilities/interface/i2oEvfMsgs.h"

using namespace stor;


EventDistributor::EventDistributor(SharedResourcesPtr sr):
  _sharedResources(sr)
{}


EventDistributor::~EventDistributor()
{
  clearStreams();
  clearConsumers();
}

void EventDistributor::addEventToRelevantQueues( I2OChain& ioc )
{
  // special handling for faulty or incomplete events
  if ( ioc.faulty() || !ioc.complete() )
  {
    std::ostringstream msg;
    msg << "Faulty or incomplete I2OChain for event " 
      << ioc.fragmentKey().event_
      << ": 0x" << std::hex << ioc.faultyBits()
      << " received from " << ioc.hltURL()
      << " (rbBufferId " << ioc.rbBufferId() << ").";
    XCEPT_DECLARE( stor::exception::IncompleteEventMessage,
      xcept, msg.str());
    _sharedResources->_statisticsReporter->alarmHandler()->
      notifySentinel(AlarmHandler::ERROR, xcept);

    DataSenderMonitorCollection& dataSenderMonColl =
      _sharedResources->_statisticsReporter->getDataSenderMonitorCollection();
    dataSenderMonColl.addFaultyEventSample(ioc);

    if ( !( _sharedResources->_configuration->getDiskWritingParams()._faultyEventsStream.empty() ) &&
      ( ioc.i2oMessageCode() == I2O_SM_DATA || ioc.i2oMessageCode() == I2O_SM_ERROR) )
      ioc.tagForStream(0); // special stream for faulty events
  }
  else
  {
    tagCompleteEventForQueues( ioc );
  }

  // Check if event belongs here at all:
  bool unexpected = true;

  if( ioc.isTaggedForAnyStream() )
  {
    unexpected = false;
    _sharedResources->_streamQueue->enq_wait( ioc );
  }
  
  if( ioc.isTaggedForAnyEventConsumer() )
  {
    unexpected = false;
    _sharedResources->_eventQueueCollection->addEvent( ioc );
  }

  if( unexpected && ioc.messageCode() == Header::EVENT )
  {
    RunMonitorCollection& runMonColl =
      _sharedResources->_statisticsReporter->getRunMonitorCollection();
    runMonColl.addUnwantedEvent(ioc);
  }
}

void EventDistributor::tagCompleteEventForQueues( I2OChain& ioc )
{
  switch( ioc.messageCode() )
  {
    
    case Header::INIT:
    {
      std::vector<unsigned char> b;
      ioc.copyFragmentsIntoBuffer(b);
      InitMsgView imv( &b[0] );
      if( _sharedResources->_initMsgCollection->addIfUnique( imv ) )
      {
        try
        {
          for_each(_eventStreamSelectors.begin(),_eventStreamSelectors.end(),
            boost::bind(&EventStreamSelector::initialize, _1, imv));

          for_each(_eventConsumerSelectors.begin(), _eventConsumerSelectors.end(),
            boost::bind(&EventConsumerSelector::initialize, _1, imv));
        }
        catch( stor::exception::InvalidEventSelection& e )
        {
          _sharedResources->_statisticsReporter->alarmHandler()->
            notifySentinel(AlarmHandler::ERROR,e);
        }
      }
      
      DataSenderMonitorCollection& dataSenderMonColl = _sharedResources->
        _statisticsReporter->getDataSenderMonitorCollection();
      dataSenderMonColl.addInitSample(ioc);
      
      break;
    }
    
    case Header::EVENT:
    {
      for( EvtSelList::iterator it = _eventStreamSelectors.begin(),
             itEnd = _eventStreamSelectors.end();
           it != itEnd;
           ++it )
      {
        if( (*it)->acceptEvent( ioc ) )
        {
          ioc.tagForStream( (*it)->configInfo().streamId() );
        }
      }
      for( ConsSelList::iterator it = _eventConsumerSelectors.begin(),
             itEnd = _eventConsumerSelectors.end();
           it != itEnd;
           ++it )
      {
        if( (*it)->acceptEvent( ioc ) )
        {
          ioc.tagForEventConsumer( (*it)->queueId() );
        }
      }
      
      RunMonitorCollection& runMonCollection = _sharedResources->
        _statisticsReporter->getRunMonitorCollection();
      runMonCollection.getRunNumbersSeenMQ().addSampleIfLarger(ioc.runNumber());
      runMonCollection.getLumiSectionsSeenMQ().addSampleIfLarger(ioc.lumiSection());
      runMonCollection.getEventIDsReceivedMQ().addSample(ioc.eventNumber());
      
      DataSenderMonitorCollection& dataSenderMonColl = _sharedResources->
        _statisticsReporter->getDataSenderMonitorCollection();
      dataSenderMonColl.addEventSample(ioc);

      break;
    }
    
    case Header::DQM_EVENT:
    {
      utils::time_point_t now = utils::getCurrentTime();

      for( DQMEvtSelList::iterator it = _dqmEventSelectors.begin(),
             itEnd = _dqmEventSelectors.end();
           it != itEnd;
           ++it)
      {
        if( (*it)->acceptEvent( ioc, now ) )
        {
          ioc.tagForDQMEventConsumer( (*it)->queueId() );
        }
      }
      
      DataSenderMonitorCollection& dataSenderMonColl = _sharedResources->
        _statisticsReporter->getDataSenderMonitorCollection();
      dataSenderMonColl.addDQMEventSample(ioc);

      if( ioc.isTaggedForAnyDQMEventConsumer() )
      {
        _sharedResources->_dqmEventQueue->enq_nowait( ioc );
      }
      else
      {
        _sharedResources->_statisticsReporter->getDQMEventMonitorCollection().
          getDroppedDQMEventCountsMQ().addSample(1);
      }
      
      break;
    }
    
    case Header::ERROR_EVENT:
    {
      for( ErrSelList::iterator it = _errorStreamSelectors.begin(),
             itEnd = _errorStreamSelectors.end();
           it != itEnd;
           ++it )
      {
        if( (*it)->acceptEvent( ioc ) )
        {
          ioc.tagForStream( (*it)->configInfo().streamId() );
        }
      }
      
      RunMonitorCollection& runMonCollection = _sharedResources->
        _statisticsReporter->getRunMonitorCollection();
      runMonCollection.getRunNumbersSeenMQ().addSample(ioc.runNumber());
      runMonCollection.getLumiSectionsSeenMQ().addSampleIfLarger(ioc.lumiSection());
      runMonCollection.getErrorEventIDsReceivedMQ().addSample(ioc.eventNumber());
      
      DataSenderMonitorCollection& dataSenderMonColl = _sharedResources->
        _statisticsReporter->getDataSenderMonitorCollection();
      dataSenderMonColl.addErrorEventSample(ioc);

      break;
    }
    
    default:
    {
      std::ostringstream msg;
      msg << "I2OChain with unknown message type " <<
        ioc.messageCode();
      XCEPT_DECLARE( stor::exception::WrongI2OMessageType,
                     xcept, msg.str());
      _sharedResources->_statisticsReporter->
        alarmHandler()->notifySentinel(AlarmHandler::ERROR, xcept);

      // 24-Jun-2009, KAB - this is not really the best way to track this,
      // but it's probably better than nothing in the short term.
      DataSenderMonitorCollection& dataSenderMonColl = _sharedResources->
        _statisticsReporter->getDataSenderMonitorCollection();
      dataSenderMonColl.addFaultyEventSample(ioc);

      break;
    }
  }
}

const bool EventDistributor::full() const
{
  return _sharedResources->_streamQueue->full();
}


void EventDistributor::registerEventConsumer
(
  const EventConsRegPtr regPtr
)
{
  ConsSelPtr evtSel( new EventConsumerSelector(regPtr) );

  InitMsgSharedPtr initMsgPtr =
    _sharedResources->_initMsgCollection->getElementForOutputModule(
      regPtr->outputModuleLabel()
    );
  if ( initMsgPtr.get() != 0 )
  {
    uint8* initPtr = &(*initMsgPtr)[0];
    InitMsgView initView(initPtr);
    try
    {
      evtSel->initialize( initView );
    }
    catch( stor::exception::InvalidEventSelection& e )
    {
      _sharedResources->_statisticsReporter->alarmHandler()->
        notifySentinel(AlarmHandler::ERROR, e);
    }
  }
  
  _eventConsumerSelectors.insert( evtSel );
}

void EventDistributor::registerDQMEventConsumer( const DQMEventConsRegPtr ptr )
{
  DQMEvtSelPtr dqmEvtSel( new DQMEventSelector(ptr) );
  _dqmEventSelectors.insert( dqmEvtSel );
}

void EventDistributor::registerEventStreams( const EvtStrConfigListPtr cl )
{
  for( EvtStrConfigList::const_iterator it = cl->begin(), itEnd = cl->end();
       it != itEnd;
       ++it )
  {
    EvtSelPtr evtSel( new EventStreamSelector(*it) );
    _eventStreamSelectors.insert( evtSel );
  }
}


void EventDistributor::registerErrorStreams( const ErrStrConfigListPtr cl )
{
  for( ErrStrConfigList::const_iterator it = cl->begin(), itEnd = cl->end();
       it != itEnd;
       ++it )
  {
    ErrSelPtr errSel( new ErrorStreamSelector(*it) );
    _errorStreamSelectors.insert( errSel );
  }
}


void EventDistributor::clearStreams()
{
  _eventStreamSelectors.clear();
  _errorStreamSelectors.clear();
}


unsigned int EventDistributor::configuredStreamCount() const
{
  return _eventStreamSelectors.size() +
    _errorStreamSelectors.size();
}


unsigned int EventDistributor::initializedStreamCount() const
{
  unsigned int counter = 0;
  for (EvtSelList::const_iterator it = _eventStreamSelectors.begin(),
         itEnd = _eventStreamSelectors.end();
       it != itEnd;
       ++it)
  {
    if ( (*it)->isInitialized() )
      ++counter;
  }
  return counter;
}


void EventDistributor::clearConsumers()
{
  _eventConsumerSelectors.clear();
  _dqmEventSelectors.clear();
}


unsigned int EventDistributor::configuredConsumerCount() const
{
  return _eventConsumerSelectors.size() + _dqmEventSelectors.size();
}


unsigned int EventDistributor::initializedConsumerCount() const
{
  unsigned int counter = 0;
  for (ConsSelList::const_iterator it = _eventConsumerSelectors.begin(),
         itEnd = _eventConsumerSelectors.end();
       it != itEnd;
       ++it)
  {
    if ( (*it)->isInitialized() )
      ++counter;
  }
  return counter;
}


void EventDistributor::checkForStaleConsumers()
{
  utils::time_point_t now = utils::getCurrentTime();

  EventQueueCollectionPtr eqc =
    _sharedResources->_eventQueueCollection;
  eqc->clearStaleQueues(now);

  DQMEventQueueCollectionPtr dqc =
    _sharedResources->_dqmEventQueueCollection;
  dqc->clearStaleQueues(now);
}


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
