// $Id: FragmentProcessor.cc,v 1.1.2.10 2009/03/10 21:18:54 biery Exp $

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
  _actionIsActive(true),
  _fileCheckIntervalStart(time(0)),
  _fileCheckEventCounter(0)
{
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
    ::usleep(_timeout);
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
