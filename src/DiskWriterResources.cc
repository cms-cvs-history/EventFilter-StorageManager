// $Id: DiskWriterResources.cc,v 1.1.2.5 2009/05/05 20:14:12 mommsen Exp $

#include "EventFilter/StorageManager/interface/DiskWriterResources.h"

namespace stor
{
  DiskWriterResources::DiskWriterResources() :
    _configurationIsNeeded(false),
    _streamChangeIsNeeded(false),
    _fileClosingTestIsNeeded(false),
    _diskWriterIsBusy(false),
    _streamChangeInProgress(false)
  {
  }

  void DiskWriterResources::requestStreamConfiguration
  (
    EvtStrConfigListPtr evtStrConfig,
    ErrStrConfigListPtr errStrConfig,
    double timeoutValue
  )
  {
    boost::mutex::scoped_lock sl(_streamChangeMutex);

    _requestedEventStreamConfig = evtStrConfig;
    _requestedErrorStreamConfig = errStrConfig;
    _requestedTimeout = timeoutValue;
    _configurationIsNeeded = true;
    _streamChangeIsNeeded = true;
  }

  void DiskWriterResources::requestStreamDestruction()
  {
    boost::mutex::scoped_lock sl(_streamChangeMutex);
    _configurationIsNeeded = false;
    _streamChangeIsNeeded = true;
  }

  bool DiskWriterResources::streamChangeRequested
  (
    bool& doConfig,
    EvtStrConfigListPtr& evtStrConfig,
    ErrStrConfigListPtr& errStrConfig,
    double& timeoutValue
  )
  {
    std::cout << "stream change requested: " << _streamChangeIsNeeded << std::endl;

    // Avoid locking for each event when there is no
    // change needed.
    if (! _streamChangeIsNeeded) {return false;}

    boost::mutex::scoped_lock sl(_streamChangeMutex);

    doConfig = _configurationIsNeeded;
    if (_configurationIsNeeded)
    {
      _configurationIsNeeded = false;
      evtStrConfig = _requestedEventStreamConfig;
      errStrConfig = _requestedErrorStreamConfig;
      timeoutValue = _requestedTimeout;
    }

    _streamChangeIsNeeded = false;
    _streamChangeInProgress = true;

    return true;
  }

  void DiskWriterResources::waitForStreamChange()
  {
    boost::mutex::scoped_lock sl(_streamChangeMutex);
    std::cout << "wait for stream change" << std::endl;
    if (_streamChangeIsNeeded || _streamChangeInProgress)
      {
        _streamChangeCondition.wait(sl);
      }
  }

  bool DiskWriterResources::streamChangeOngoing()
  {
    boost::mutex::scoped_lock sl(_streamChangeMutex);
    return (_streamChangeIsNeeded || _streamChangeInProgress);
  }

  void DiskWriterResources::streamChangeDone()
  {
    boost::mutex::scoped_lock sl(_streamChangeMutex);
    if (_streamChangeInProgress)
      {
        _streamChangeCondition.notify_one();
      }
    _streamChangeInProgress = false;
  }

  void DiskWriterResources::requestFileClosingTest()
  {
    boost::mutex::scoped_lock sl(_generalMutex);
    _fileClosingTestIsNeeded = true;
  }

  bool DiskWriterResources::fileClosingTestRequested()
  {
    if (! _fileClosingTestIsNeeded) {return false;}

    boost::mutex::scoped_lock sl(_generalMutex);

    _fileClosingTestIsNeeded = false;
    return true;
  }

  void DiskWriterResources::setBusy(bool isBusyFlag)
  {
    //boost::mutex::scoped_lock sl(_generalMutex);
    _diskWriterIsBusy = isBusyFlag;
  }

  bool DiskWriterResources::isBusy()
  {
    //boost::mutex::scoped_lock sl(_generalMutex);
    return _diskWriterIsBusy;
  }

} // namespace stor

/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
