// $Id: FragmentProcessor.cc,v 1.1.2.8 2009/02/25 10:59:04 mommsen Exp $

#include <unistd.h>

#include "EventFilter/StorageManager/interface/FragmentProcessor.h"
#include "EventFilter/StorageManager/interface/I2OChain.h"

using namespace stor;


FragmentProcessor::FragmentProcessor( boost::shared_ptr<SharedResources> sr,
                                      boost::shared_ptr<StateMachine> sm ) :
  _sharedResources(sr),
  _stateMachine(sm),
  _fragmentStore(),
  _eventDistributor(),
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
  boost::shared_ptr<FragmentQueue> fq = _sharedResources->_fragmentQueue;
  if (fq->deq_timed_wait(fragment, _timeout))
    _stateMachine->getCurrentState().processI2OFragment(fragment);
  else
    _stateMachine->getCurrentState().noFragmentToProcess();  
}


void FragmentProcessor::updateStatistics()
{

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
