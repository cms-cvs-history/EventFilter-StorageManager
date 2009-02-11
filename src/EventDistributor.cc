// $Id: EventDistributor.cc,v 1.1.2.3 2009/01/30 10:49:57 mommsen Exp $

#include "EventFilter/StorageManager/interface/EventDistributor.h"

using namespace stor;


EventDistributor::EventDistributor()
{

}


EventDistributor::~EventDistributor()
{

}


void EventDistributor::addEventToRelevantQueues(I2OChain&)
{

}


bool EventDistributor::full()
{
  return false;
}


const QueueID EventDistributor::registerEventConsumer
(
  boost::shared_ptr<EventConsumerRegistrationInfo> registrationInfo
)
{
  return _eventConsumerQueueCollection.registerEventConsumer(registrationInfo);
}


void EventDistributor::registerEventStreams()
{

}


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
