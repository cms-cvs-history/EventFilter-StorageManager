// $Id: Configuration.cc,v 1.9 2009/08/21 07:18:44 mommsen Exp $
/// @file: Configuration.cc

#include "EventFilter/StorageManager/interface/Configuration.h"
#include "EventFilter/Utilities/interface/ParameterSetRetriever.h"
#include "FWCore/Framework/interface/EventSelector.h"
#include "FWCore/ParameterSet/interface/PythonProcessDesc.h"

#include <toolbox/net/Utils.h>

#include <sstream>

namespace stor
{
  Configuration::Configuration(xdata::InfoSpace* infoSpace,
                               unsigned long instanceNumber) :
    _streamConfigurationChanged(false),
    _infospaceRunNumber(0),
    _localRunNumber(0)
  {
    // default values are used to initialize infospace values,
    // so they should be set first
    setDiskWritingDefaults(instanceNumber);
    setDQMProcessingDefaults();
    setEventServingDefaults();
    setQueueConfigurationDefaults();
    setWorkerThreadDefaults();

    setupDiskWritingInfoSpaceParams(infoSpace);
    setupDQMProcessingInfoSpaceParams(infoSpace);
    setupEventServingInfoSpaceParams(infoSpace);
    setupQueueConfigurationInfoSpaceParams(infoSpace);
    setupWorkerThreadInfoSpaceParams(infoSpace);
  }

  struct DiskWritingParams Configuration::getDiskWritingParams() const
  {
    boost::mutex::scoped_lock sl(_generalMutex);
    return _diskWriteParamCopy;
  }


  struct DQMProcessingParams Configuration::getDQMProcessingParams() const
  {
    boost::mutex::scoped_lock sl(_generalMutex);
    return _dqmParamCopy;
  }

  struct EventServingParams Configuration::getEventServingParams() const
  {
    boost::mutex::scoped_lock sl(_generalMutex);
    return _eventServeParamCopy;
  }

  struct QueueConfigurationParams Configuration::getQueueConfigurationParams() const
  {
    boost::mutex::scoped_lock sl(_generalMutex);
    return _queueConfigParamCopy;
  }

  struct WorkerThreadParams Configuration::getWorkerThreadParams() const
  {
    boost::mutex::scoped_lock sl(_generalMutex);
    return _workerThreadParamCopy;
  }

  void Configuration::updateAllParams()
  {
    boost::mutex::scoped_lock sl(_generalMutex);
    updateLocalDiskWritingData();
    updateLocalDQMProcessingData();
    updateLocalEventServingData();
    updateLocalQueueConfigurationData();
    updateLocalWorkerThreadData();
    updateLocalRunNumber();
  }

  unsigned int Configuration::getRunNumber() const
  {
    return _localRunNumber;
  }

  void Configuration::updateDiskWritingParams()
  {
    boost::mutex::scoped_lock sl(_generalMutex);
    updateLocalDiskWritingData();
  }

  void Configuration::updateRunParams()
  {
    boost::mutex::scoped_lock sl(_generalMutex);
    updateLocalRunNumber();
  }

  bool Configuration::streamConfigurationHasChanged() const
  {
    boost::mutex::scoped_lock sl(_generalMutex);
    return _streamConfigurationChanged;
  }

  void Configuration::actionPerformed(xdata::Event& ispaceEvent)
  {
    boost::mutex::scoped_lock sl(_generalMutex);

    if (ispaceEvent.type() == "ItemChangedEvent")
      {
        std::string item =
          dynamic_cast<xdata::ItemChangedEvent&>(ispaceEvent).itemName();
        if (item == "STparameterSet")
          {
            evf::ParameterSetRetriever smpset(_streamConfiguration);
            std::string tmpStreamConfiguration = smpset.getAsString();

            if (tmpStreamConfiguration != _previousStreamCfg)
              {
                _streamConfigurationChanged = true;
                _previousStreamCfg = tmpStreamConfiguration;
              }
          }
      }
  }

  void Configuration::setCurrentEventStreamConfig(EvtStrConfigListPtr cfgList)
  {
    boost::mutex::scoped_lock sl(_evtStrCfgMutex);
    _currentEventStreamConfig = cfgList;
  }

  void Configuration::setCurrentErrorStreamConfig(ErrStrConfigListPtr cfgList)
  {
    boost::mutex::scoped_lock sl(_errStrCfgMutex);
    _currentErrorStreamConfig = cfgList;
  }

  EvtStrConfigListPtr Configuration::getCurrentEventStreamConfig() const
  {
    boost::mutex::scoped_lock sl(_evtStrCfgMutex);
    return _currentEventStreamConfig;
  }

  ErrStrConfigListPtr Configuration::getCurrentErrorStreamConfig() const
  {
    boost::mutex::scoped_lock sl(_errStrCfgMutex);
    return _currentErrorStreamConfig;
  }

  void Configuration::setDiskWritingDefaults(unsigned long instanceNumber)
  {
    _diskWriteParamCopy._streamConfiguration = "";
    _diskWriteParamCopy._fileName = "storageManager";
    _diskWriteParamCopy._filePath = "/tmp";
    _diskWriteParamCopy._lookAreaPath = "/store/lookarea";
    _diskWriteParamCopy._ecalCalibPath = "/store/calibarea";
    _diskWriteParamCopy._fileCatalog = "summaryCatalog.txt";
    _diskWriteParamCopy._setupLabel = "mtcc";
    _diskWriteParamCopy._nLogicalDisk = 0;
    _diskWriteParamCopy._maxFileSizeMB = 0;
    _diskWriteParamCopy._highWaterMark = 0.9;
    _diskWriteParamCopy._lumiSectionTimeOut = 45.0;
    _diskWriteParamCopy._errorEventsTimeOut = 300.0;
    _diskWriteParamCopy._fileClosingTestInterval = 5.0;
    _diskWriteParamCopy._exactFileSizeTest = false;
    _diskWriteParamCopy._useIndexFiles = true;
    _diskWriteParamCopy._sataUser = "USER:mickey2mouse";

    _previousStreamCfg = _diskWriteParamCopy._streamConfiguration;

    std::ostringstream oss;
    oss << instanceNumber;
    _diskWriteParamCopy._smInstanceString = oss.str();

    std::string tmpString(toolbox::net::getHostName());
    // strip domainame
    std::string::size_type pos = tmpString.find('.');  
    if (pos != std::string::npos) {  
      std::string basename = tmpString.substr(0,pos);  
      tmpString = basename;
    }
    _diskWriteParamCopy._hostName = tmpString;

    _diskWriteParamCopy._initialSafetyLevel = 0;
  }

  void Configuration::setDQMProcessingDefaults()
  {
    _dqmParamCopy._collateDQM = false;
    _dqmParamCopy._archiveDQM = false;
    _dqmParamCopy._filePrefixDQM = "/tmp/DQM";
    _dqmParamCopy._archiveIntervalDQM = 0;
    _dqmParamCopy._purgeTimeDQM = 120;
    _dqmParamCopy._readyTimeDQM = 30;
    _dqmParamCopy._useCompressionDQM = true;
    _dqmParamCopy._compressionLevelDQM = 1;
  }

  void Configuration::setEventServingDefaults()
  {
    _eventServeParamCopy._pushmode2proxy = false;
    _eventServeParamCopy._maxESEventRate = 100.0;  // hertz
    _eventServeParamCopy._maxESDataRate = 1024.0;  // MB/sec
    _eventServeParamCopy._activeConsumerTimeout = 60.0;  // seconds
    _eventServeParamCopy._idleConsumerTimeout = 120.0;  // seconds
    _eventServeParamCopy._consumerQueueSize = 5;
    _eventServeParamCopy._fairShareES = false;
    _eventServeParamCopy._DQMmaxESEventRate = 1.0;  // hertz
    _eventServeParamCopy._DQMactiveConsumerTimeout = 60.0;  // seconds
    _eventServeParamCopy._DQMidleConsumerTimeout = 120.0;  // seconds
    _eventServeParamCopy._DQMconsumerQueueSize = 15;
    _eventServeParamCopy._esSelectedHLTOutputModule = "hltOutputDQM";
  }

  void Configuration::setQueueConfigurationDefaults()
  {
    _queueConfigParamCopy._commandQueueSize = 128;
    _queueConfigParamCopy._dqmEventQueueSize = 3072;
    _queueConfigParamCopy._fragmentQueueSize = 1024;
    _queueConfigParamCopy._registrationQueueSize = 128;
    _queueConfigParamCopy._streamQueueSize = 2048;
  }

  void Configuration::setWorkerThreadDefaults()
  {
    // set defaults
    _workerThreadParamCopy._FPdeqWaitTime = 0.25;
    _workerThreadParamCopy._DWdeqWaitTime = 0.50;
    _workerThreadParamCopy._DQMEPdeqWaitTime = 0.50;

    // validate the defaults
    // Currently, this consists of rounding up the values to next
    // whole number since ConcurrentQueue::deq_timed_wait only takes
    // integer values.  This can be removed once we switch to Boost 1.38
    // and we get sub-second intervals.
    _workerThreadParamCopy._FPdeqWaitTime =
      ceil(_workerThreadParamCopy._FPdeqWaitTime);
    _workerThreadParamCopy._DWdeqWaitTime =
      ceil(_workerThreadParamCopy._DWdeqWaitTime);
    _workerThreadParamCopy._DQMEPdeqWaitTime =
      ceil(_workerThreadParamCopy._DQMEPdeqWaitTime);
  
    _workerThreadParamCopy._staleFragmentTimeOut = 60;
    _workerThreadParamCopy._monitoringSleepSec = 1;
}

  void Configuration::
  setupDiskWritingInfoSpaceParams(xdata::InfoSpace* infoSpace)
  {
    // copy the initial defaults into the xdata variables
    _streamConfiguration = _diskWriteParamCopy._streamConfiguration;
    _fileName = _diskWriteParamCopy._fileName;
    _filePath = _diskWriteParamCopy._filePath;
    _lookAreaPath = _diskWriteParamCopy._lookAreaPath;
    _ecalCalibPath = _diskWriteParamCopy._ecalCalibPath;
    _fileCatalog = _diskWriteParamCopy._fileCatalog;
    _setupLabel = _diskWriteParamCopy._setupLabel;
    _nLogicalDisk = _diskWriteParamCopy._nLogicalDisk;
    _maxFileSize = _diskWriteParamCopy._maxFileSizeMB;
    _highWaterMark = _diskWriteParamCopy._highWaterMark;
    _lumiSectionTimeOut = _diskWriteParamCopy._lumiSectionTimeOut;
    _errorEventsTimeOut = _diskWriteParamCopy._errorEventsTimeOut;
    _fileClosingTestInterval =
      static_cast<int>(_diskWriteParamCopy._fileClosingTestInterval);
    _exactFileSizeTest = _diskWriteParamCopy._exactFileSizeTest;
    _useIndexFiles = _diskWriteParamCopy._useIndexFiles;
    _sataUser = _diskWriteParamCopy._sataUser;

    // bind the local xdata variables to the infospace
    infoSpace->fireItemAvailable("STparameterSet", &_streamConfiguration);
    infoSpace->fireItemAvailable("fileName", &_fileName);
    infoSpace->fireItemAvailable("filePath", &_filePath);
    infoSpace->fireItemAvailable("lookAreaPath", &_lookAreaPath);
    infoSpace->fireItemAvailable("ecalCalibPath", &_ecalCalibPath);
    infoSpace->fireItemAvailable("fileCatalog", &_fileCatalog);
    infoSpace->fireItemAvailable("setupLabel", &_setupLabel);
    infoSpace->fireItemAvailable("nLogicalDisk", &_nLogicalDisk);
    infoSpace->fireItemAvailable("maxFileSize", &_maxFileSize);
    infoSpace->fireItemAvailable("highWaterMark", &_highWaterMark);
    infoSpace->fireItemAvailable("lumiSectionTimeOut", &_lumiSectionTimeOut);
    infoSpace->fireItemAvailable("errorEventsTimeOut", &_errorEventsTimeOut);
    infoSpace->fireItemAvailable("fileClosingTestInterval",
                                 &_fileClosingTestInterval);
    infoSpace->fireItemAvailable("exactFileSizeTest", &_exactFileSizeTest);
    infoSpace->fireItemAvailable("useIndexFiles", &_useIndexFiles);
    infoSpace->fireItemAvailable("sataUser", &_sataUser);

    // special handling for the stream configuration string (we
    // want to note when it changes to see if we need to reconfigure
    // between runs)
    infoSpace->addItemChangedListener("STparameterSet", this);
  }

  void Configuration::
  setupDQMProcessingInfoSpaceParams(xdata::InfoSpace* infoSpace)
  {
    // copy the initial defaults to the xdata variables
    _collateDQM = _dqmParamCopy._collateDQM;
    _archiveDQM = _dqmParamCopy._archiveDQM;
    _archiveIntervalDQM = static_cast<int>(_dqmParamCopy._archiveIntervalDQM);
    _filePrefixDQM = _dqmParamCopy._filePrefixDQM;
    _purgeTimeDQM = static_cast<int>(_dqmParamCopy._purgeTimeDQM);
    _readyTimeDQM = static_cast<int>(_dqmParamCopy._readyTimeDQM);
    _useCompressionDQM = _dqmParamCopy._useCompressionDQM;
    _compressionLevelDQM = _dqmParamCopy._compressionLevelDQM;

    // bind the local xdata variables to the infospace
    infoSpace->fireItemAvailable("collateDQM", &_collateDQM);
    infoSpace->fireItemAvailable("archiveDQM", &_archiveDQM);
    infoSpace->fireItemAvailable("archiveIntervalDQM", &_archiveIntervalDQM);
    infoSpace->fireItemAvailable("purgeTimeDQM", &_purgeTimeDQM);
    infoSpace->fireItemAvailable("readyTimeDQM", &_readyTimeDQM);
    infoSpace->fireItemAvailable("filePrefixDQM", &_filePrefixDQM);
    infoSpace->fireItemAvailable("useCompressionDQM", &_useCompressionDQM);
    infoSpace->fireItemAvailable("compressionLevelDQM", &_compressionLevelDQM);
  }

  void Configuration::
  setupEventServingInfoSpaceParams(xdata::InfoSpace* infoSpace)
  {
    // copy the initial defaults to the xdata variables
    _pushmode2proxy = _eventServeParamCopy._pushmode2proxy;
    _maxESEventRate = _eventServeParamCopy._maxESEventRate;
    _maxESDataRate = _eventServeParamCopy._maxESDataRate;
    _activeConsumerTimeout =
      static_cast<int>(_eventServeParamCopy._activeConsumerTimeout);
    _idleConsumerTimeout =
      static_cast<int>(_eventServeParamCopy._idleConsumerTimeout);
    _consumerQueueSize = _eventServeParamCopy._consumerQueueSize;
    _fairShareES = _eventServeParamCopy._fairShareES;
    _DQMmaxESEventRate = _eventServeParamCopy._DQMmaxESEventRate;
    _DQMactiveConsumerTimeout =
      static_cast<int>(_eventServeParamCopy._DQMactiveConsumerTimeout);
    _DQMidleConsumerTimeout =
      static_cast<int>(_eventServeParamCopy._DQMidleConsumerTimeout);
    _DQMconsumerQueueSize = _eventServeParamCopy._DQMconsumerQueueSize;
    _esSelectedHLTOutputModule =
      _eventServeParamCopy._esSelectedHLTOutputModule;

    // bind the local xdata variables to the infospace
    infoSpace->fireItemAvailable( "runNumber", &_infospaceRunNumber );
    infoSpace->fireItemAvailable("pushMode2Proxy", &_pushmode2proxy);
    infoSpace->fireItemAvailable("maxESEventRate",&_maxESEventRate);
    infoSpace->fireItemAvailable("maxESDataRate",&_maxESDataRate);
    infoSpace->fireItemAvailable("activeConsumerTimeout",
                                 &_activeConsumerTimeout);
    infoSpace->fireItemAvailable("idleConsumerTimeout",&_idleConsumerTimeout);
    infoSpace->fireItemAvailable("consumerQueueSize",&_consumerQueueSize);
    //infoSpace->fireItemAvailable("fairShareES",&_fairShareES);
    infoSpace->fireItemAvailable("DQMmaxESEventRate",&_DQMmaxESEventRate);
    infoSpace->fireItemAvailable("DQMactiveConsumerTimeout",
                              &_DQMactiveConsumerTimeout);
    infoSpace->fireItemAvailable("DQMidleConsumerTimeout",
                              &_DQMidleConsumerTimeout);
    infoSpace->fireItemAvailable("DQMconsumerQueueSize",
                                 &_DQMconsumerQueueSize);
    infoSpace->fireItemAvailable("esSelectedHLTOutputModule",
                              &_esSelectedHLTOutputModule);
  }

  void Configuration::
  setupQueueConfigurationInfoSpaceParams(xdata::InfoSpace* infoSpace)
  {
    // copy the initial defaults to the xdata variables
    _commandQueueSize = _queueConfigParamCopy._commandQueueSize;
    _dqmEventQueueSize = _queueConfigParamCopy._dqmEventQueueSize;
    _fragmentQueueSize = _queueConfigParamCopy._fragmentQueueSize;
    _registrationQueueSize = _queueConfigParamCopy._registrationQueueSize;
    _streamQueueSize = _queueConfigParamCopy._streamQueueSize;

    // bind the local xdata variables to the infospace
    infoSpace->fireItemAvailable("commandQueueSize", &_commandQueueSize);
    infoSpace->fireItemAvailable("dqmEventQueueSize", &_dqmEventQueueSize);
    infoSpace->fireItemAvailable("fragmentQueueSize", &_fragmentQueueSize);
    infoSpace->fireItemAvailable("registrationQueueSize",
                                 &_registrationQueueSize);
    infoSpace->fireItemAvailable("streamQueueSize", &_streamQueueSize);
  }

  void Configuration::
  setupWorkerThreadInfoSpaceParams(xdata::InfoSpace* infoSpace)
  {
    // copy the initial defaults to the xdata variables
    _FPdeqWaitTime = _workerThreadParamCopy._FPdeqWaitTime;
    _DWdeqWaitTime = _workerThreadParamCopy._DWdeqWaitTime;
    _DQMEPdeqWaitTime = _workerThreadParamCopy._DQMEPdeqWaitTime;
    _staleFragmentTimeOut = _workerThreadParamCopy._staleFragmentTimeOut;
    _monitoringSleepSec = _workerThreadParamCopy._monitoringSleepSec;

    // bind the local xdata variables to the infospace
    infoSpace->fireItemAvailable("FPdeqWaitTime", &_FPdeqWaitTime);
    infoSpace->fireItemAvailable("DWdeqWaitTime", &_DWdeqWaitTime);
    infoSpace->fireItemAvailable("DQMEPdeqWaitTime", &_DQMEPdeqWaitTime);
    infoSpace->fireItemAvailable("staleFragmentTimeOut", &_staleFragmentTimeOut);
    infoSpace->fireItemAvailable("monitoringSleepSec", &_monitoringSleepSec);
  }

  void Configuration::updateLocalDiskWritingData()
  {
    evf::ParameterSetRetriever smpset(_streamConfiguration);
    _diskWriteParamCopy._streamConfiguration = smpset.getAsString();

    _diskWriteParamCopy._fileName = _fileName;
    _diskWriteParamCopy._filePath = _filePath;
    _diskWriteParamCopy._lookAreaPath = _lookAreaPath;
    _diskWriteParamCopy._ecalCalibPath = _ecalCalibPath;
    _diskWriteParamCopy._fileCatalog = _fileCatalog;
    _diskWriteParamCopy._setupLabel = _setupLabel;
    _diskWriteParamCopy._nLogicalDisk = _nLogicalDisk;
    _diskWriteParamCopy._maxFileSizeMB = _maxFileSize;
    _diskWriteParamCopy._highWaterMark = _highWaterMark;
    _diskWriteParamCopy._lumiSectionTimeOut = _lumiSectionTimeOut;
    _diskWriteParamCopy._errorEventsTimeOut = _errorEventsTimeOut;
    _diskWriteParamCopy._fileClosingTestInterval = _fileClosingTestInterval;
    _diskWriteParamCopy._exactFileSizeTest = _exactFileSizeTest;
    _diskWriteParamCopy._useIndexFiles = _useIndexFiles;
    _diskWriteParamCopy._sataUser = _sataUser;

    _streamConfigurationChanged = false;
  }

  void Configuration::updateLocalDQMProcessingData()
  {
    _dqmParamCopy._collateDQM = _collateDQM;
    _dqmParamCopy._archiveDQM = _archiveDQM;
    _dqmParamCopy._archiveIntervalDQM = _archiveIntervalDQM;
    _dqmParamCopy._filePrefixDQM = _filePrefixDQM;
    _dqmParamCopy._purgeTimeDQM = _purgeTimeDQM;
    _dqmParamCopy._readyTimeDQM = _readyTimeDQM;
    _dqmParamCopy._useCompressionDQM = _useCompressionDQM;
    _dqmParamCopy._compressionLevelDQM = _compressionLevelDQM;
  }

  void Configuration::updateLocalEventServingData()
  {
    _eventServeParamCopy._pushmode2proxy = _pushmode2proxy;
    _eventServeParamCopy._maxESEventRate = _maxESEventRate;
    _eventServeParamCopy._maxESDataRate = _maxESDataRate;
    _eventServeParamCopy._activeConsumerTimeout = _activeConsumerTimeout;
    _eventServeParamCopy._idleConsumerTimeout = _idleConsumerTimeout;
    _eventServeParamCopy._consumerQueueSize = _consumerQueueSize;
    _eventServeParamCopy._fairShareES = _fairShareES;
    _eventServeParamCopy._DQMmaxESEventRate = _DQMmaxESEventRate;
    _eventServeParamCopy._DQMactiveConsumerTimeout = _DQMactiveConsumerTimeout;
    _eventServeParamCopy._DQMidleConsumerTimeout = _DQMidleConsumerTimeout;
    _eventServeParamCopy._DQMconsumerQueueSize = _DQMconsumerQueueSize;
    _eventServeParamCopy._esSelectedHLTOutputModule =
      _esSelectedHLTOutputModule;

    // validation
    if (_eventServeParamCopy._consumerQueueSize < 1)
      {
        _eventServeParamCopy._consumerQueueSize = 1;
      }
    if (_eventServeParamCopy._DQMconsumerQueueSize < 1)
      {
        _eventServeParamCopy._DQMconsumerQueueSize = 1;
      }
    if (_eventServeParamCopy._maxESEventRate < 0.0)
      {
        _eventServeParamCopy._maxESEventRate = 0.0;
      }
    if (_eventServeParamCopy._maxESDataRate < 0.0)
      {
        _eventServeParamCopy._maxESDataRate = 0.0;
      }
    if (_eventServeParamCopy._DQMmaxESEventRate < 0.0)
      {
        _eventServeParamCopy._DQMmaxESEventRate = 0.0;
      }
  }

  void Configuration::updateLocalQueueConfigurationData()
  {
    _queueConfigParamCopy._commandQueueSize = _commandQueueSize;
    _queueConfigParamCopy._dqmEventQueueSize = _dqmEventQueueSize;
    _queueConfigParamCopy._fragmentQueueSize = _fragmentQueueSize;
    _queueConfigParamCopy._registrationQueueSize = _registrationQueueSize;
    _queueConfigParamCopy._streamQueueSize = _streamQueueSize;
  }

  void Configuration::updateLocalWorkerThreadData()
  {
    _workerThreadParamCopy._FPdeqWaitTime = _FPdeqWaitTime;
    _workerThreadParamCopy._DWdeqWaitTime = _DWdeqWaitTime;
    _workerThreadParamCopy._DQMEPdeqWaitTime = _DQMEPdeqWaitTime;

    // validate the values
    // Currently, this consists of rounding up the values to next
    // whole number since ConcurrentQueue::deq_timed_wait only takes
    // integer values.  This can be removed once we switch to Boost 1.38
    // and we get sub-second intervals.
    _workerThreadParamCopy._FPdeqWaitTime =
      ceil(_workerThreadParamCopy._FPdeqWaitTime);
    _workerThreadParamCopy._DWdeqWaitTime =
      ceil(_workerThreadParamCopy._DWdeqWaitTime);
    _workerThreadParamCopy._DQMEPdeqWaitTime =
      ceil(_workerThreadParamCopy._DQMEPdeqWaitTime);

    _workerThreadParamCopy._staleFragmentTimeOut = _staleFragmentTimeOut;
    _workerThreadParamCopy._monitoringSleepSec = _monitoringSleepSec;
  }

  void Configuration::updateLocalRunNumber()
  {
    _localRunNumber = _infospaceRunNumber;
  }

  void parseStreamConfiguration(std::string cfgString,
                                EvtStrConfigListPtr evtCfgList,
                                ErrStrConfigListPtr errCfgList)
  {
    if (cfgString == "") return;

    PythonProcessDesc py_pdesc(cfgString.c_str());
    boost::shared_ptr<edm::ProcessDesc> pdesc = py_pdesc.processDesc();
    boost::shared_ptr<edm::ParameterSet> smPSet = pdesc->getProcessPSet();

    // loop over each end path
    size_t streamId = 0;
    std::vector<std::string> allEndPaths = 
      smPSet->getParameter<std::vector<std::string> >("@end_paths");
    for(std::vector<std::string>::iterator endPathIter = allEndPaths.begin();
        endPathIter != allEndPaths.end(); ++endPathIter) {

      // loop over each element in the end path list (not sure why...)
      std::vector<std::string> anEndPath =
        smPSet->getParameter<std::vector<std::string> >((*endPathIter));
      for(std::vector<std::string>::iterator ep2Iter = anEndPath.begin();
          ep2Iter != anEndPath.end(); ++ep2Iter) {

        // fetch the end path parameter set
        edm::ParameterSet endPathPSet =
          smPSet->getParameter<edm::ParameterSet>((*ep2Iter));
        if (! endPathPSet.empty()) {
          std::string mod_type =
            endPathPSet.getParameter<std::string> ("@module_type");
          if (mod_type == "EventStreamFileWriter") {

            std::string streamLabel =
              endPathPSet.getParameter<std::string> ("streamLabel");
            int maxFileSizeMB = endPathPSet.getParameter<int> ("maxSize");
            EventStreamConfigurationInfo::FilterList requestedEvents =
              edm::EventSelector::getEventSelectionVString(endPathPSet);
            std::string requestedOMLabel =
              endPathPSet.getUntrackedParameter<std::string>("SelectHLTOutput",
                                                             std::string());
            bool useCompression =
              endPathPSet.getUntrackedParameter<bool>("use_compression", true);
            unsigned int compressionLevel =
              endPathPSet.getUntrackedParameter<int>("compression_level", 1);
            unsigned int maxEventSize =
              endPathPSet.getUntrackedParameter<int>("max_event_size", 7000000);

            EventStreamConfigurationInfo cfgInfo(streamLabel,
                                                 maxFileSizeMB,
                                                 requestedEvents,
                                                 requestedOMLabel,
                                                 useCompression,
                                                 compressionLevel,
                                                 maxEventSize);
            cfgInfo.setStreamId(++streamId);
            evtCfgList->push_back(cfgInfo);
          }
          else if (mod_type == "ErrorStreamFileWriter" ||
                   mod_type == "FRDStreamFileWriter") {

            std::string streamLabel =
              endPathPSet.getParameter<std::string> ("streamLabel");
            int maxFileSizeMB = endPathPSet.getParameter<int> ("maxSize");

            ErrorStreamConfigurationInfo cfgInfo(streamLabel,
                                                 maxFileSizeMB);
            cfgInfo.setStreamId(++streamId);
            errCfgList->push_back(cfgInfo);
          }
        }
      }
    }
  }

} // namespace stor

/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
