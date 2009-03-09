// $Id: EventDistributor.cc,v 1.1.2.15 2009/03/09 15:03:09 biery Exp $

#include "EventFilter/StorageManager/interface/EventDistributor.h"

using namespace stor;


EventDistributor::EventDistributor(boost::shared_ptr<InitMsgCollection> coll):
  _initMsgCollection(coll)
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
        assert( _initMsgCollection.get() != 0 );
        if( _initMsgCollection->addIfUnique( imv ) )
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
               ioc.tagForEventConsumer( it->configInfo().queueId() );
             }
         }
       break;
     }

    case Header::DQM_EVENT:
      {

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
      _streamQueue.addEvent( ioc );
    }

}

const bool EventDistributor::full() const
{
  return false;
}


const QueueID EventDistributor::registerEventConsumer
(
  boost::shared_ptr<EventConsumerRegistrationInfo> registrationInfo
)
{
  QueueID queueId =
    _eventConsumerQueueCollection.registerConsumer( *registrationInfo );

  // store the queue ID in the regInfo here or does the queueCollection do that?

  EventConsumerSelector evtSel( *registrationInfo );
  InitMsgSharedPtr initMsgPtr =
    _initMsgCollection->getElementForOutputModule( registrationInfo->selHLTOut() );
  if ( initMsgPtr.get() != 0 )
    {
      uint8* regPtr = &(*initMsgPtr)[0];
      InitMsgView initView(regPtr);
      evtSel.initialize( initView );
    }
  _eventConsumerSelectors.push_back( evtSel );

  return queueId;
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
  return _eventStreamSelectors.size() + _errorStreamSelectors.size();
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
