// $Id: FragmentProcessor.cc,v 1.1.2.5 2009/02/19 19:52:51 mommsen Exp $

#include <unistd.h>

#include "EventFilter/StorageManager/interface/FragmentProcessor.h"
#include "EventFilter/StorageManager/interface/I2OChain.h"

using namespace stor;


FragmentProcessor::FragmentProcessor() :
  _eventDistributor(),
  _stateMachine(0, &_eventDistributor, this, &_fragmentStore),
  _fragmentQueue(new FragmentQueue(100)), // this needs fixing!
  _fragmentStore(),
  _timeout(100*1000),
  _actionIsActive(true)
{
}

FragmentProcessor::~FragmentProcessor()
{
}

bool FragmentProcessor::processMessages(toolbox::task::WorkLoop*)
{
  processAllCommands();
  processOneFragmentIfPossible();
  return _actionIsActive;
}

void FragmentProcessor::processOneFragmentIfPossible()
{
  if (_eventDistributor.full()) 
    ::usleep(_timeout);
  else 
    processOneFragment();
}

void FragmentProcessor::processOneFragment()
{
  I2OChain fragment;
  if (_fragmentQueue->deq_timed_wait(fragment, _timeout))
    _stateMachine.getCurrentState().processI2OFragment(fragment);
  else
    _stateMachine.getCurrentState().noFragmentToProcess();  
}

// bool FragmentProcessor::processMessages(toolbox::task::WorkLoop*)
// {
//   processAllCommands();

//   if ( _eventDistributor.full() )
//   {
//     // The event distributor cannot accept any new events.
//     // Wait a bit then start over
//     ::usleep(_timeout);
//   }
//   else
//   {
//     I2OChain fragment;
//     bool foundFragment =  _fragmentQueue.deq_timed_wait(fragment, _timeout);

//     const Operations& currentState = _stateMachine.getCurrentState();
  
//     if (foundFragment)
//     {
//       currentState.processI2OFragment( fragment );
//     }
//     else
//     {
//       currentState.noFragmentToProcess();
//     }
//   }
//   return _actionIsActive;
// }


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
