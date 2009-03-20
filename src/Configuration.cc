// $Id: Configuration.cc,v 1.1.2.9 2009/03/18 17:33:45 biery Exp $

#include "EventFilter/StorageManager/interface/Configuration.h"
#include "EventFilter/Utilities/interface/ParameterSetRetriever.h"

#include <toolbox/net/Utils.h>

#include <sstream>

namespace stor
{
  Configuration::Configuration(xdata::InfoSpace* infoSpace,
                               unsigned long instanceNumber) :
    _streamConfigurationChanged(false)
  {
    // default values are used to initialize infospace values,
    // so they should be set first
    setDiskWritingDefaults(instanceNumber);
    setDQMProcessingDefaults();
    setEventServingDefaults();

    setupDiskWritingInfoSpaceParams(infoSpace);
    setupDQMProcessingInfoSpaceParams(infoSpace);
    setupEventServingInfoSpaceParams(infoSpace);
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

  void Configuration::updateAllParams()
  {
    boost::mutex::scoped_lock sl(_generalMutex);
    updateLocalDiskWritingData();
    updateLocalDQMProcessingData();
    updateLocalEventServingData();
  }

  void Configuration::updateDiskWritingParams()
  {
    boost::mutex::scoped_lock sl(_generalMutex);
    updateLocalDiskWritingData();
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

  void Configuration::setDiskWritingDefaults(unsigned long instanceNumber)
  {
    _diskWriteParamCopy._streamConfiguration = "";
    _diskWriteParamCopy._fileName = "storageManager";
    _diskWriteParamCopy._filePath = "/scratch2/cheung";
    _diskWriteParamCopy._fileCatalog = "summaryCatalog.txt";
    _diskWriteParamCopy._setupLabel = "mtcc";
    _diskWriteParamCopy._nLogicalDisk = 0;
    _diskWriteParamCopy._maxFileSize = 0;
    _diskWriteParamCopy._highWaterMark = 0.9;
    _diskWriteParamCopy._lumiSectionTimeOut = 45.0;
    _diskWriteParamCopy._fileClosingTestInterval = 5.0;
    _diskWriteParamCopy._exactFileSizeTest = false;

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
    _eventServeParamCopy._esSelectedHLTOutputModule = "out4DQM";
  }

  void Configuration::
  setupDiskWritingInfoSpaceParams(xdata::InfoSpace* infoSpace)
  {
    // copy the initial defaults into the xdata variables
    _streamConfiguration = _diskWriteParamCopy._streamConfiguration;
    _fileName = _diskWriteParamCopy._fileName;
    _filePath = _diskWriteParamCopy._filePath;
    _fileCatalog = _diskWriteParamCopy._fileCatalog;
    _setupLabel = _diskWriteParamCopy._setupLabel;
    _nLogicalDisk = _diskWriteParamCopy._nLogicalDisk;
    _maxFileSize = _diskWriteParamCopy._maxFileSize;
    _highWaterMark = _diskWriteParamCopy._highWaterMark;
    _lumiSectionTimeOut = _diskWriteParamCopy._lumiSectionTimeOut;
    _fileClosingTestInterval =
      static_cast<int>(_diskWriteParamCopy._fileClosingTestInterval);
    _exactFileSizeTest = _diskWriteParamCopy._exactFileSizeTest;

    // bind the local xdata variables to the infospace
    infoSpace->fireItemAvailable("STparameterSet", &_streamConfiguration);
    infoSpace->fireItemAvailable("fileName", &_fileName);
    infoSpace->fireItemAvailable("filePath", &_filePath);
    infoSpace->fireItemAvailable("fileCatalog", &_fileCatalog);
    infoSpace->fireItemAvailable("setupLabel", &_setupLabel);
    infoSpace->fireItemAvailable("nLogicalDisk", &_nLogicalDisk);
    infoSpace->fireItemAvailable("maxFileSize", &_maxFileSize);
    infoSpace->fireItemAvailable("highWaterMark", &_highWaterMark);
    infoSpace->fireItemAvailable("lumiSectionTimeOut", &_lumiSectionTimeOut);
    infoSpace->fireItemAvailable("fileClosingTestInterval",
                                 &_fileClosingTestInterval);
    infoSpace->fireItemAvailable("exactFileSizeTest", &_exactFileSizeTest);

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

  void Configuration::updateLocalDiskWritingData()
  {
    evf::ParameterSetRetriever smpset(_streamConfiguration);
    _diskWriteParamCopy._streamConfiguration = smpset.getAsString();

    _diskWriteParamCopy._fileName = _fileName;
    _diskWriteParamCopy._filePath = _filePath;
    _diskWriteParamCopy._fileCatalog = _fileCatalog;
    _diskWriteParamCopy._setupLabel = _setupLabel;
    _diskWriteParamCopy._nLogicalDisk = _nLogicalDisk;
    _diskWriteParamCopy._maxFileSize = _maxFileSize;
    _diskWriteParamCopy._highWaterMark = _highWaterMark;
    _diskWriteParamCopy._lumiSectionTimeOut = _lumiSectionTimeOut;
    _diskWriteParamCopy._fileClosingTestInterval = _fileClosingTestInterval;
    _diskWriteParamCopy._exactFileSizeTest = _exactFileSizeTest;

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

} // namespace stor

/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
