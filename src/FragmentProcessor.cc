// $Id: FragmentProcessor.cc,v 1.1.2.13 2009/03/19 12:08:01 dshpakov Exp $

#include <unistd.h>

#include "EventFilter/StorageManager/interface/FragmentProcessor.h"
#include "EventFilter/StorageManager/interface/I2OChain.h"

using namespace stor;


FragmentProcessor::FragmentProcessor( SharedResources sr ) :
  _sharedResources(sr),
  _fragmentStore(),
  _eventDistributor(sr),
  _diskWriter(),
  _timeout(1),
  _actionIsActive(true),
  _fileCheckIntervalStart(time(0)),
  _fileCheckEventCounter(0)
{
  _stateMachine.reset(new StateMachine(&_diskWriter, &_eventDistributor,
                                       &_fragmentStore, &_sharedResources));
  _stateMachine->initiate();
}

FragmentProcessor::~FragmentProcessor()
{
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
  boost::shared_ptr<FragmentQueue> fq = _sharedResources._fragmentQueue2;
  ::sleep(_timeout);
  //if (fq->deq_timed_wait(fragment, _timeout))
  if (fq->deq_nowait(fragment))
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

  boost::shared_ptr<CommandQueue> cq = _sharedResources._commandQueue;
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
    _sharedResources._registrationQueue;
  while ( regQueue->deq_nowait( regInfo ) )
    {
      regInfo->registerMe( &_eventDistributor );
    }
}


// temporary!
void FragmentProcessor::closeDiskFilesIfNeeded()
{
  DiskWritingParams dwParams =
    _sharedResources._configuration->getDiskWritingParams();
  time_t now = time(0);
  if ((now - _fileCheckIntervalStart) >= dwParams._fileClosingTestInterval)
    {
      _fileCheckIntervalStart = now;
      boost::shared_ptr<edm::ServiceManager> serviceManager =
        _sharedResources._serviceManager;
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
