// $Id: FragmentProcessor.cc,v 1.1.2.3 2009/01/30 10:49:57 mommsen Exp $

#include "EventFilter/StorageManager/interface/FragmentProcessor.h"

using namespace stor;


FragmentProcessor::FragmentProcessor()
{

}


FragmentProcessor::~FragmentProcessor()
{

}


void FragmentProcessor::processMessages()
{

}


void FragmentProcessor:: haveStateProcessFragment(I2OChain &chain)
{

}


void FragmentProcessor::updateStatistics()
{

}


const QueueID FragmentProcessor::registerEventConsumer
(
  boost::shared_ptr<EventConsumerRegistrationInfo> registrationInfo
)
{
  return _eventDistributor.registerEventConsumer(registrationInfo);
}


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
