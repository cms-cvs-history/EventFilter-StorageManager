// $Id: EventDistributor.cc,v 1.1.2.6 2009/03/02 17:44:46 paterno Exp $

#include "EventFilter/StorageManager/interface/EventDistributor.h"

using namespace stor;


EventDistributor::EventDistributor()
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
        if( _initMsgCollection.addIfUnique( imv ) )
          {
            for( ESList::iterator it = _eventSelectors.begin();
                 it != _eventSelectors.end();
                 ++it )
              {
                it->initialize( imv );
              }
          }
      }

   case Header::EVENT:
     {
       for( ESList::iterator it = _eventSelectors.begin();
            it != _eventSelectors.end();
            ++it )
         {
           if( it->acceptEvent( ioc ) )
             {
               // get the stream ID from the selector
               //            unsigned int sid = it->streamId();
               // add the stream ID to the list of stream IDs in the i2oChain
               // ????
             }
         }
     }

    default:
      {
        // Log error and/or go to failed state
      }

    }

  // if the list of stream IDs in the i2oChain has more than zero entries,
  // push the chain onto the StreamQueue
  //  ????

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


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
