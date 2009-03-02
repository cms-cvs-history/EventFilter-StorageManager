// $Id: EventDistributor.cc,v 1.1.2.5 2009/03/01 22:57:43 biery Exp $

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


void EventDistributor::
registerEventStreams(std::vector<EventStreamConfigurationInfo>& cfgList)
{

}


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
