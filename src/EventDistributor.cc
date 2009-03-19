// $Id: EventDistributor.cc,v 1.1.2.25 2009/03/19 18:59:38 biery Exp $

#include "EventFilter/StorageManager/interface/EventDistributor.h"

using namespace stor;


EventDistributor::EventDistributor( boost::shared_ptr<SharedResources> sr ):
  _sharedResources(sr)
{

}


EventDistributor::~EventDistributor()
{

}


void EventDistributor::addEventToRelevantQueues( I2OChain& ioc )
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

  if( ioc.isTaggedForAnyStream() )
    {
      _sharedResources->_streamQueue->enq_wait( ioc );
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

void EventDistributor::registerEventStreams( const EvtStrConfList& cl )
{
  for( EvtStrConfList::const_iterator it = cl.begin(); it != cl.end(); ++it )
    {
      _eventStreamSelectors.push_back( EventStreamSelector( *it ) );
    }
}


void EventDistributor::registerErrorStreams( const ErrStrConfList& cl )
{
  for( ErrStrConfList::const_iterator it = cl.begin(); it != cl.end(); ++it )
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


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
