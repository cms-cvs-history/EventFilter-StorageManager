// $Id: EventDistributor.cc,v 1.1.2.2 2009/01/20 10:54:37 mommsen Exp $

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
