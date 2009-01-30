// $Id: FragmentProcessor.cc,v 1.1.2.2 2009/01/20 10:54:37 mommsen Exp $

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
