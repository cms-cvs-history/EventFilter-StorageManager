// $Id: FragmentProcessor.cc,v 1.1.2.18 2009/03/20 21:30:07 biery Exp $

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
  _diskWriter(sr),
  _wrapperNotifier( wn ),
  _timeout(1),
  _actionIsActive(true),
  _fileCheckIntervalStart(time(0)),
  _fileCheckEventCounter(0)
{
  _stateMachine.reset( new StateMachine( &_diskWriter, &_eventDistributor,
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

      // temporary!
      ++_fileCheckEventCounter;
      if (_fileCheckEventCounter >= 100)
        {
          _fileCheckEventCounter = 0;
          closeDiskFilesIfNeeded();
        }
    }
  else
    {
      _stateMachine->getCurrentState().noFragmentToProcess();  

      // temporary!
      _fileCheckEventCounter = 0;
      closeDiskFilesIfNeeded();
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


// temporary!
void FragmentProcessor::closeDiskFilesIfNeeded()
{
  DiskWritingParams dwParams =
    _sharedResources->_configuration->getDiskWritingParams();
  time_t now = time(0);
  if ((now - _fileCheckIntervalStart) >= dwParams._fileClosingTestInterval)
    {
      _fileCheckIntervalStart = now;
      boost::shared_ptr<edm::ServiceManager> serviceManager =
        _sharedResources->_serviceManager;
      if (serviceManager.get() != NULL)
        {
          serviceManager->closeFilesIfNeeded();
        }
    }
}


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
