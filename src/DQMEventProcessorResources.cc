// $Id: DQMEventProcessorResources.cc,v 1.1.2.4 2009/04/08 16:25:59 biery Exp $

#include "EventFilter/StorageManager/interface/DQMEventProcessorResources.h"

namespace stor
{
  DQMEventProcessorResources::DQMEventProcessorResources() :
    _configurationIsNeeded(false),
    _endOfRunIsNeeded(false),
    _storeDestructionIsNeeded(false),
    _configurationInProgress(false),
    _endOfRunInProgress(false),
    _storeDestructionInProgress(false)
  {
  }

  void DQMEventProcessorResources::
  requestConfiguration(DQMProcessingParams params, double timeoutValue)
  {
    boost::mutex::scoped_lock sl(_generalMutex);

    _requestedDQMProcessingParams = params;
    _requestedTimeout = timeoutValue;
    _configurationIsNeeded = true;
  }

  bool DQMEventProcessorResources::
  configurationRequested(DQMProcessingParams& params, double& timeoutValue)
  {
    boost::mutex::scoped_lock sl(_generalMutex);
    if (! _configurationIsNeeded) {return false;}

    _configurationIsNeeded = false;
    params = _requestedDQMProcessingParams;
    timeoutValue = _requestedTimeout;
    _configurationInProgress = true;
    return true;
  }

  void DQMEventProcessorResources::waitForConfiguration()
  {
    boost::mutex::scoped_lock sl(_generalMutex);
    if (_configurationIsNeeded || _configurationInProgress)
      {
        _configurationCondition.wait(sl);
      }
  }

  void DQMEventProcessorResources::configurationDone()
  {
    boost::mutex::scoped_lock sl(_generalMutex);
    if (_configurationInProgress)
      {
        _configurationCondition.notify_one();
      }
    _configurationInProgress = false;
  }

  void DQMEventProcessorResources::requestStoreDestruction()
  {
    boost::mutex::scoped_lock sl(_generalMutex);
    _storeDestructionIsNeeded = true;
  }

  bool DQMEventProcessorResources::storeDestructionRequested()
  {
    boost::mutex::scoped_lock sl(_generalMutex);
    if (! _storeDestructionIsNeeded) {return false;}

    _storeDestructionIsNeeded = false;
    _storeDestructionInProgress = true;
    return true;
  }

  void DQMEventProcessorResources::waitForStoreDestruction()
  {
    boost::mutex::scoped_lock sl(_generalMutex);
    if (_storeDestructionIsNeeded || _storeDestructionInProgress)
      {
        _storeDestructionCondition.wait(sl);
      }
  }

  void DQMEventProcessorResources::storeDestructionDone()
  {
    boost::mutex::scoped_lock sl(_generalMutex);
    if (_storeDestructionInProgress)
      {
        _storeDestructionCondition.notify_one();
      }
    _storeDestructionInProgress = false;
  }

  void DQMEventProcessorResources::requestEndOfRun()
  {
    boost::mutex::scoped_lock sl(_generalMutex);
    _endOfRunIsNeeded = true;
  }

  bool DQMEventProcessorResources::endOfRunRequested()
  {
    boost::mutex::scoped_lock sl(_generalMutex);
    if (! _endOfRunIsNeeded) {return false;}

    _endOfRunIsNeeded = false;
    _endOfRunInProgress = true;
    return true;
  }

  void DQMEventProcessorResources::waitForEndOfRun()
  {
    boost::mutex::scoped_lock sl(_generalMutex);
    if (_endOfRunIsNeeded || _endOfRunInProgress)
      {
        _endOfRunCondition.wait(sl);
      }
  }

  bool DQMEventProcessorResources::isEndOfRunDone()
  {
    boost::mutex::scoped_lock sl(_generalMutex);
    return (!_endOfRunIsNeeded && !_endOfRunInProgress);
  }

  void DQMEventProcessorResources::endOfRunDone()
  {
    boost::mutex::scoped_lock sl(_generalMutex);
    if (_endOfRunInProgress)
      {
        _endOfRunCondition.notify_one();
      }
    _endOfRunInProgress = false;
  }

} // namespace stor

/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
