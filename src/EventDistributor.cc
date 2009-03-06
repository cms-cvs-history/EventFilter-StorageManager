// $Id: EventDistributor.cc,v 1.1.2.10 2009/03/06 14:58:12 biery Exp $

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
            for( ESList::iterator it = _eventSelectors.begin();
                 it != _eventSelectors.end();
                 ++it )
              {
                it->initialize( imv );
              }
          }
        break;
      }

   case Header::EVENT:
     {
       for( ESList::iterator it = _eventSelectors.begin();
            it != _eventSelectors.end();
            ++it )
         {
           if( it->acceptEvent( ioc ) )
             {
               ioc.getEventStreamTags().push_back( it->configInfo().streamId() );
             }
         }
       break;
     }

    case Header::DQM_EVENT:
      {

        break;
      }

    default:
      {
        // Log error and/or go to failed state
        break;
      }

    }

  if( ioc.isTaggedForAnyEventStream() )
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
  return _eventConsumerQueueCollection.registerConsumer(*registrationInfo);
}


void EventDistributor::registerEventStreams( const StreamConfList& cl )
{
  for( StreamConfList::const_iterator it = cl.begin(); it != cl.end(); ++it )
    {
      _eventSelectors.push_back( EventSelector( *it ) );
    }
}


void EventDistributor::clearEventStreams()
{
  _eventSelectors.clear();
}


unsigned int EventDistributor::configuredStreamCount() const
{
  return _eventSelectors.size();
}


unsigned int EventDistributor::initializedStreamCount() const
{
  unsigned int counter = 0;
  unsigned int listSize = _eventSelectors.size();
  for (unsigned int idx = 0; idx < listSize; ++idx)
    {
      if (_eventSelectors[idx].isInitialized())
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
