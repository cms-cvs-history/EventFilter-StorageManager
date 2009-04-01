// $Id: FragmentProcessor.cc,v 1.1.2.20 2009/03/31 19:37:46 mommsen Exp $

#include <unistd.h>

#include "toolbox/task/WorkLoopFactory.h"

#include "EventFilter/StorageManager/interface/Exception.h"
#include "EventFilter/StorageManager/interface/FragmentProcessor.h"
#include "EventFilter/StorageManager/interface/I2OChain.h"

using namespace stor;


FragmentProcessor::FragmentProcessor( SharedResourcesPtr sr,
                                      WrapperNotifier& wn ) :
  _sharedResources(sr),
  _fragmentStore(),
  _eventDistributor(sr),
  _wrapperNotifier( wn ),
  _timeout(1),
  _actionIsActive(true)
{
  _stateMachine.reset( new StateMachine( &_eventDistributor,
                                         &_fragmentStore, &_wrapperNotifier,
                                         _sharedResources ) );
  _stateMachine->initiate();
}

FragmentProcessor::~FragmentProcessor()
{
}

void FragmentProcessor::startWorkLoop(std::string applicationIdentifier)
{
  try
    {
      _processWL = toolbox::task::getWorkLoopFactory()->
        getWorkLoop( applicationIdentifier + "FragmentProcessor",
                     "waiting" );

      if ( ! _processWL->isActive() )
        {
          toolbox::task::ActionSignature* processAction = 
            toolbox::task::bind(this, &FragmentProcessor::processMessages, 
                                applicationIdentifier + "ProcessMessages");
          _processWL->submit(processAction);

          _processWL->activate();
        }
    }
  catch (xcept::Exception& e)
    {
      std::string msg = "Failed to start workloop 'FragmentProcessor' with 'processMessages'.";
    XCEPT_RETHROW(stor::exception::FragmentProcessing, msg, e);
  }
}

bool FragmentProcessor::processMessages(toolbox::task::WorkLoop*)
{
  processAllCommands();
  processAllRegistrations();
  processOneFragmentIfPossible();

  return _actionIsActive;
}

void FragmentProcessor::processOneFragmentIfPossible()
{
  if (_eventDistributor.full()) 
    ::sleep(_timeout);
  else 
    processOneFragment();
}

void FragmentProcessor::processOneFragment()
{
  I2OChain fragment;
  boost::shared_ptr<FragmentQueue> fq = _sharedResources->_fragmentQueue;
  if (fq->deq_timed_wait(fragment, _timeout))
    {
      _stateMachine->getCurrentState().processI2OFragment(fragment);
    }
  else
    {
      _stateMachine->getCurrentState().noFragmentToProcess();  
    }
}


void FragmentProcessor::updateStatistics()
{

}


void FragmentProcessor::processAllCommands()
{

  boost::shared_ptr<CommandQueue> cq = _sharedResources->_commandQueue;
  stor::event_ptr evt;

  while( cq->deq_nowait( evt ) )
    {
      _stateMachine->process_event( *evt );
    }

}


void FragmentProcessor::processAllRegistrations()
{
  RegInfoBasePtr regInfo;
  boost::shared_ptr<RegistrationQueue> regQueue =
    _sharedResources->_registrationQueue;
  while ( regQueue->deq_nowait( regInfo ) )
    {
      regInfo->registerMe( &_eventDistributor );
    }
}


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
