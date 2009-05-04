// $Id: EventDistributor.cc,v 1.1.2.42 2009/04/27 11:05:17 dshpakov Exp $

#include "EventFilter/StorageManager/interface/EventDistributor.h"

using namespace stor;


EventDistributor::EventDistributor(SharedResourcesPtr sr):
  _sharedResources(sr)
{

}


EventDistributor::~EventDistributor()
{

}


void EventDistributor::addEventToRelevantQueues( I2OChain& ioc )
{
  // special handling for faulty or incomplete events
  if ( ioc.faulty() || !ioc.complete() )
    {
      // mark these events for the special SM error stream

      // log a warning???
    }

  else
    {
      tagCompleteEventForQueues( ioc );
    }

  if( ioc.isTaggedForAnyStream() )
    {
      _sharedResources->_streamQueue->enq_wait( ioc );
    }

  if( ioc.isTaggedForAnyEventConsumer() )
    {
      _sharedResources->_eventConsumerQueueCollection->addEvent( ioc );
      checkForStaleConsumers();
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
            for( EvtSelList::iterator it = _eventStreamSelectors.begin();
                 it != _eventStreamSelectors.end();
                 ++it )
              {
                it->initialize( imv );
              }
            for( ConsSelList::iterator it = _eventConsumerSelectors.begin();
                 it != _eventConsumerSelectors.end();
                 ++it )
              {
                it->initialize( imv );
              }
          }

        DataSenderMonitorCollection& dataSenderMonColl = _sharedResources->
          _statisticsReporter->getDataSenderMonitorCollection();
        dataSenderMonColl.addInitSample(ioc);

        break;
      }

    case Header::EVENT:
      {
        for( EvtSelList::iterator it = _eventStreamSelectors.begin();
             it != _eventStreamSelectors.end();
             ++it )
          {
            if( it->acceptEvent( ioc ) )
              {
                ioc.tagForStream( it->configInfo().streamId() );
              }
          }
        for( ConsSelList::iterator it = _eventConsumerSelectors.begin();
             it != _eventConsumerSelectors.end();
             ++it )
          {
            if( it->acceptEvent( ioc ) )
              {
                ioc.tagForEventConsumer( it->queueId() );
              }
          }

        DataSenderMonitorCollection& dataSenderMonColl = _sharedResources->
          _statisticsReporter->getDataSenderMonitorCollection();
        dataSenderMonColl.addEventSample(ioc);

        // temporary handling (until the new event server is ready)
        if ( _sharedResources->_oldEventServer.get() != NULL )
          {
             ioc.copyFragmentsIntoBuffer(_tempEventArea);
             EventMsgView emsg(&_tempEventArea[0]);
            _sharedResources->_oldEventServer->processEvent(emsg);
          }

        break;
      }

    case Header::DQM_EVENT:
      {
        for( DQMSelList::iterator it = _DQMSelectors.begin();
             it != _DQMSelectors.end(); ++it )
          {
            if( it->acceptEvent( ioc ) )
              {
                ioc.tagForDQMEventConsumer( it->queueId() );
              }
          }

#ifdef SM_NEW_DQM
        // Put this here or in EventDistributor::addEventToRelevantQueues?

        // pass any DQM event to the DQM event processor, as it might write 
        // DQM histograms to disk which are not requested by any consumer
        _sharedResources->_dqmEventQueue->enq_nowait( ioc );
#else
        // temporary handling (until the DQMProcessor comes online)
        if ( _sharedResources->_dqmServiceManager.get() != 0 )
          {
            ioc.copyFragmentsIntoBuffer(_tempEventArea);
            DQMEventMsgView dqmEventView(&_tempEventArea[0]);
            _sharedResources->_dqmServiceManager->manageDQMEventMsg(dqmEventView);
          }
#endif

        break;
      }

    case Header::ERROR_EVENT:
      {
        for( ErrSelList::iterator it = _errorStreamSelectors.begin();
             it != _errorStreamSelectors.end();
             ++it )
          {
            if( it->acceptEvent( ioc ) )
              {
                ioc.tagForStream( it->configInfo().streamId() );
              }
          }

        break;
      }

    default:
      {
        // Log error and/or go to failed state
        break;
      }

    }

}

const bool EventDistributor::full() const
{
  return false;
}


void EventDistributor::registerEventConsumer
(
  EventConsumerRegistrationInfo* registrationInfo
)
{
  EventConsumerSelector evtSel( *registrationInfo );

  InitMsgSharedPtr initMsgPtr =
    _sharedResources->_initMsgCollection->getElementForOutputModule( registrationInfo->selHLTOut() );
  if ( initMsgPtr.get() != 0 )
    {
      uint8* regPtr = &(*initMsgPtr)[0];
      InitMsgView initView(regPtr);
      evtSel.initialize( initView );
    }

  _eventConsumerSelectors.push_back( evtSel );
}

////////////////////////////////
//// Register DQM consumer: ////
////////////////////////////////
void EventDistributor::registerDQMEventConsumer( DQMRegPtr ptr )
{
  _DQMSelectors.push_back( DQMEventSelector( *ptr ) );
}

void EventDistributor::registerEventStreams( const EvtStrConfigList& cl )
{
  for( EvtStrConfigList::const_iterator it = cl.begin(); it != cl.end(); ++it )
    {
      _eventStreamSelectors.push_back( EventStreamSelector( *it ) );
    }
}


void EventDistributor::registerErrorStreams( const ErrStrConfigList& cl )
{
  for( ErrStrConfigList::const_iterator it = cl.begin(); it != cl.end(); ++it )
    {
      _errorStreamSelectors.push_back( ErrorStreamSelector( *it ) );
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
  unsigned int listSize = _eventStreamSelectors.size();
  for (unsigned int idx = 0; idx < listSize; ++idx)
    {
      if (_eventStreamSelectors[idx].isInitialized())
        {
          ++counter;
        }
    }
  return counter;
}


void EventDistributor::clearConsumers()
{
  _eventConsumerSelectors.clear();
}


unsigned int EventDistributor::configuredConsumerCount() const
{
  return _eventConsumerSelectors.size();
}


unsigned int EventDistributor::initializedConsumerCount() const
{
  unsigned int counter = 0;
  unsigned int listSize = _eventConsumerSelectors.size();
  for (unsigned int idx = 0; idx < listSize; ++idx)
    {
      if (_eventConsumerSelectors[idx].isInitialized())
        {
          ++counter;
        }
    }
  return counter;
}


void EventDistributor::checkForStaleConsumers()
{

  std::vector<QueueID> stale_qs;
  _sharedResources->_eventConsumerQueueCollection->clearStaleQueues( stale_qs );

  // Double linear search, should try to optimize...
  for( ConsSelList::iterator i = _eventConsumerSelectors.begin();
           i != _eventConsumerSelectors.end(); ++i )
    {

      // First, assume the consumer is active. If we find its queue in
      // the stale list, we'll mark it stale in the inner loop.
      i->markAsActive();

      for( std::vector<QueueID>::const_iterator j = stale_qs.begin();
           j != stale_qs.end(); ++j )
        {
          if( i->queueId() == *j )
            {
              i->markAsStale();
            }
        }

    }

}

/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
