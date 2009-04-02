// $Id: DiskWriterResources.cc,v 1.1.2.16 2009/04/01 20:02:19 biery Exp $

#include "EventFilter/StorageManager/interface/DiskWriterResources.h"

namespace stor
{
  DiskWriterResources::DiskWriterResources() :
    _configurationIsNeeded(false),
    _streamDestructionIsNeeded(false),
    _fileClosingTestIsNeeded(false),
    _diskWriterIsBusy(false)
  {
  }

  void DiskWriterResources::
  requestStreamConfiguration(EvtStrConfigList evtStrConfig,
                             ErrStrConfigList errStrConfig)
  {
    boost::mutex::scoped_lock sl(_generalMutex);

    _requestedEventStreamConfig = evtStrConfig;
    _requestedErrorStreamConfig = errStrConfig;
    _configurationIsNeeded = true;
  }

  bool DiskWriterResources::
  streamConfigurationRequested(EvtStrConfigList& evtStrConfig,
                               ErrStrConfigList& errStrConfig)
  {
    boost::mutex::scoped_lock sl(_generalMutex);
    if (! _configurationIsNeeded) {return false;}

    _configurationIsNeeded = false;
    evtStrConfig = _requestedEventStreamConfig;
    errStrConfig = _requestedErrorStreamConfig;
    return true;
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
    return true;
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

/// emacs DiskWriterResources
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
