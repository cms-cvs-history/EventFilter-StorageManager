// $Id: FragmentProcessor.cc,v 1.1.2.3 2009/01/30 10:49:57 mommsen Exp $

#include <unistd.h>

#include "EventFilter/StorageManager/interface/FragmentProcessor.h"
#include "EventFilter/StorageManager/interface/I2OChain.h"

using namespace stor;


FragmentProcessor::FragmentProcessor() :
_stateMachine(0, &_eventDistributor, this),
_timeout(100000),
_doProcessMessages(true)
{

}


FragmentProcessor::~FragmentProcessor()
{

}


bool FragmentProcessor::processMessages(toolbox::task::WorkLoop*)
{
  processAllCommands();

  if ( _eventDistributor.full() )
  {
    // The event distributor cannot accept any new events.
    // Wait a bit then start over
    ::usleep(_timeout);
  }
  else
  {
    I2OChain fragment;
    bool foundFragment = false /* = _fragmentQueue.deq_timed_wait(&fragment, _timeout) */;

    const Operations& currentState = _stateMachine.getCurrentState();
  
    if (foundFragment)
    {
      currentState.processI2OFragment(fragment, _eventDistributor, _fragmentStore);
    }
    else
    {
      currentState.noFragmentToProcess(_eventDistributor, _fragmentStore);
    }
  }
  return _doProcessMessages;
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


void FragmentProcessor::processAllCommands()
{

}


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
