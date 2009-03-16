// $Id: Configuration.cc,v 1.1.2.5 2009/03/16 19:05:34 biery Exp $

#include "EventFilter/StorageManager/interface/Configuration.h"

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


  struct DQMProcessingParams Configuration::getDQMParams() const
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

  void Configuration::actionPerformed(xdata::Event& isEvt)
  {
    boost::mutex::scoped_lock sl(_generalMutex);

    if (isEvt.type() == "ItemChangedEvent")
      {
        std::string item =
          dynamic_cast<xdata::ItemChangedEvent&>(isEvt).itemName();
        if (item == "STparameterSet")
          {
            if (_streamConfiguration != _previousStreamCfg)
              {
                _streamConfigurationChanged = true;
                _previousStreamCfg = _streamConfiguration;
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
    _diskWriteParamCopy._maxFileSize = -1;
    _diskWriteParamCopy._highWaterMark = 0.9;
    _diskWriteParamCopy._lumiSectionTimeOut = 45.0;
    _diskWriteParamCopy._fileClosingTestInterval = 5.0;
    _diskWriteParamCopy._exactFileSizeTest = false;

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
  }

  void Configuration::setEventServingDefaults()
  {
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

    _previousStreamCfg = _streamConfiguration;

    // bind the local xdata variables to the infospace
    //infoSpace->fireItemAvailable("STparameterSet", &_streamConfiguration);
    infoSpace->fireItemAvailable("fileName", &_fileName);
    infoSpace->fireItemAvailable("filePath", &_filePath);
    infoSpace->fireItemAvailable("fileCatalog", &_fileCatalog);
    infoSpace->fireItemAvailable("setupLabel", &_setupLabel);
    //infoSpace->fireItemAvailable("nLogicalDisk", &_nLogicalDisk);
    infoSpace->fireItemAvailable("maxFileSize", &_maxFileSize);
    infoSpace->fireItemAvailable("highWaterMark", &_highWaterMark);
    infoSpace->fireItemAvailable("lumiSectionTimeOut", &_lumiSectionTimeOut);
    infoSpace->fireItemAvailable("fileClosingTestInterval",
                                 &_fileClosingTestInterval);
    infoSpace->fireItemAvailable("exactFileSizeTest", &_exactFileSizeTest);

    // special handling for the stream configuration string (we
    // want to note when it changes to see if we need to reconfigure
    // between runs)
    //infoSpace->addItemChangedListener("STparameterSet", this);
  }

  void Configuration::
  setupDQMProcessingInfoSpaceParams(xdata::InfoSpace* infoSpace)
  {
  }

  void Configuration::
  setupEventServingInfoSpaceParams(xdata::InfoSpace* infoSpace)
  {
  }

  void Configuration::updateLocalDiskWritingData()
  {
    //_diskWriteParamCopy._streamConfiguration = _streamConfiguration;
    _diskWriteParamCopy._fileName = _fileName;
    _diskWriteParamCopy._filePath = _filePath;
    _diskWriteParamCopy._fileCatalog = _fileCatalog;
    _diskWriteParamCopy._setupLabel = _setupLabel;
    //_diskWriteParamCopy._nLogicalDisk = _nLogicalDisk;
    _diskWriteParamCopy._maxFileSize = _maxFileSize;
    _diskWriteParamCopy._highWaterMark = _highWaterMark;
    _diskWriteParamCopy._lumiSectionTimeOut = _lumiSectionTimeOut;
    _diskWriteParamCopy._fileClosingTestInterval = _fileClosingTestInterval;
    _diskWriteParamCopy._exactFileSizeTest = _exactFileSizeTest;

    _streamConfigurationChanged = false;
  }

  void Configuration::updateLocalDQMProcessingData()
  {
  }

  void Configuration::updateLocalEventServingData()
  {
  }

} // namespace stor

/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
