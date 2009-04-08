// $Id: DiskWriterResources.cc,v 1.1.2.3 2009/04/03 14:41:22 biery Exp $

#include "EventFilter/StorageManager/interface/DiskWriterResources.h"

namespace stor
{
  DiskWriterResources::DiskWriterResources() :
    _configurationIsNeeded(false),
    _streamDestructionIsNeeded(false),
    _fileClosingTestIsNeeded(false),
    _diskWriterIsBusy(false),
    _configurationInProgress(false),
    _streamDestructionInProgress(false)
  {
  }

  void DiskWriterResources::
  requestStreamConfiguration(EvtStrConfigList* evtStrConfig,
                             ErrStrConfigList* errStrConfig,
                             double timeoutValue)
  {
    boost::mutex::scoped_lock sl(_generalMutex);

    _requestedEventStreamConfig = evtStrConfig;
    _requestedErrorStreamConfig = errStrConfig;
    _requestedTimeout = timeoutValue;
    _configurationIsNeeded = true;
  }

  bool DiskWriterResources::
  streamConfigurationRequested(EvtStrConfigList*& evtStrConfig,
                               ErrStrConfigList*& errStrConfig,
                               double& timeoutValue)
  {
    boost::mutex::scoped_lock sl(_generalMutex);
    if (! _configurationIsNeeded) {return false;}

    _configurationIsNeeded = false;
    evtStrConfig = _requestedEventStreamConfig;
    errStrConfig = _requestedErrorStreamConfig;
    timeoutValue = _requestedTimeout;
    _configurationInProgress = true;
    return true;
  }

  void DiskWriterResources::waitForStreamConfiguration()
  {
    boost::mutex::scoped_lock sl(_generalMutex);
    if (_configurationIsNeeded || _configurationInProgress)
      {
        _configurationCondition.wait(sl);
      }
  }

  void DiskWriterResources::streamConfigurationDone()
  {
    boost::mutex::scoped_lock sl(_generalMutex);
    if (_configurationInProgress)
      {
        _configurationCondition.notify_one();
      }
    _configurationInProgress = false;
  }

  void DiskWriterResources::requestStreamDestruction()
  {
    boost::mutex::scoped_lock sl(_generalMutex);
    _streamDestructionIsNeeded = true;
  }

  bool DiskWriterResources::streamDestructionRequested()
  {
    boost::mutex::scoped_lock sl(_generalMutex);
    if (! _streamDestructionIsNeeded) {return false;}

    _streamDestructionIsNeeded = false;
    _streamDestructionInProgress = true;
    return true;
  }

  void DiskWriterResources::waitForStreamDestruction()
  {
    boost::mutex::scoped_lock sl(_generalMutex);
    if (_streamDestructionIsNeeded || _streamDestructionInProgress)
      {
        _destructionCondition.wait(sl);
      }
  }

  void DiskWriterResources::streamDestructionDone()
  {
    boost::mutex::scoped_lock sl(_generalMutex);
    if (_streamDestructionInProgress)
      {
        _destructionCondition.notify_one();
      }
    _streamDestructionInProgress = false;
  }

  void DiskWriterResources::requestFileClosingTest()
  {
    boost::mutex::scoped_lock sl(_generalMutex);
    _fileClosingTestIsNeeded = true;
  }

  bool DiskWriterResources::fileClosingTestRequested()
  {
    boost::mutex::scoped_lock sl(_generalMutex);
    if (! _fileClosingTestIsNeeded) {return false;}

    _fileClosingTestIsNeeded = false;
    return true;
  }

  void DiskWriterResources::setBusy(bool isBusyFlag)
  {
    boost::mutex::scoped_lock sl(_generalMutex);
    _diskWriterIsBusy = isBusyFlag;
  }

  bool DiskWriterResources::isBusy()
  {
    boost::mutex::scoped_lock sl(_generalMutex);
    return _diskWriterIsBusy;
  }

} // namespace stor

/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
