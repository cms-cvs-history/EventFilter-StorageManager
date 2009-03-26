// $Id: Configuration.h,v 1.1.2.6 2009/03/20 17:53:01 mommsen Exp $


#ifndef EventFilter_StorageManager_Configuration_h
#define EventFilter_StorageManager_Configuration_h

#include "EventFilter/StorageManager/interface/ErrorStreamConfigurationInfo.h"
#include "EventFilter/StorageManager/interface/EventStreamConfigurationInfo.h"
#include "EventFilter/StorageManager/interface/Utils.h"

#include "xdata/InfoSpace.h"
#include "xdata/String.h"
#include "xdata/Integer.h"
#include "xdata/Double.h"
#include "xdata/Boolean.h"

#include "boost/thread/mutex.hpp"

namespace stor
{
  /**
   * Data structure to hold configuration parameters
   * that are relevant for writing data to disk.
   */
  struct DiskWritingParams
  {
    std::string _streamConfiguration;
    std::string _fileName;
    std::string _filePath;
    std::string _fileCatalog;
    std::string _setupLabel;
    int _nLogicalDisk;
    int _maxFileSizeMB;
    double _highWaterMark;  // not actually used for anything, KAB 16-Mar-09
    utils::duration_t _lumiSectionTimeOut;
    utils::duration_t _fileClosingTestInterval;
    bool _exactFileSizeTest;

    // not mapped to infospace params
    std::string _smInstanceString;
    std::string _hostName;
    int _initialSafetyLevel;  // what is this used for?
  };

  /**
   * Data structure to hold configuration parameters
   * that are relevant for the processing of DQM histograms.
   */
  struct DQMProcessingParams
  {
    bool _collateDQM;
    bool _archiveDQM;
    std::string _filePrefixDQM;
    utils::duration_t _archiveIntervalDQM;
    utils::duration_t _purgeTimeDQM;
    utils::duration_t _readyTimeDQM;
    bool _useCompressionDQM;
    int _compressionLevelDQM;
  };

  /**
   * Data structure to hold configuration parameters
   * that are relevant for serving events to consumers.
   */
  struct EventServingParams
  {
    // some of these may go away once we get rid of the event server
    bool _pushmode2proxy;
    double _maxESEventRate;  // hertz
    double _maxESDataRate;  // MB/sec
    utils::duration_t _activeConsumerTimeout;  // seconds
    utils::duration_t _idleConsumerTimeout;  // seconds
    int _consumerQueueSize;
    bool _fairShareES;
    double _DQMmaxESEventRate;  // hertz
    utils::duration_t _DQMactiveConsumerTimeout;  // seconds
    utils::duration_t _DQMidleConsumerTimeout;  // seconds
    int _DQMconsumerQueueSize;
    std::string _esSelectedHLTOutputModule;
  };

  /**
   * Free function to parse a storage manager configuration string
   * into the appropriate "configuration info" objects.
   */
  typedef std::vector<EventStreamConfigurationInfo> EvtStrConfList;
  typedef std::vector<ErrorStreamConfigurationInfo> ErrStrConfList;
  void parseStreamConfiguration(std::string cfgString,
                                EvtStrConfList& evtCfgList,
                                ErrStrConfList& errCfgList);

  /**
   * Class for managing configuration information from the infospace
   * and providing local copies of that information that are updated
   * only at requested times.
   *
   * $Author: mommsen $
   * $Revision: 1.1.2.6 $
   * $Date: 2009/03/20 17:53:01 $
   */

  class Configuration : public xdata::ActionListener
  {
  public:

    /**
     * Constructs a Configuration instance for the specified infospace
     * and application instance number.
     */
    Configuration(xdata::InfoSpace* infoSpace, unsigned long instanceNumber);

    /**
     * Destructor.
     */
    virtual ~Configuration()
    {
      // should we detach from the infospace???
    }

    /**
     * Returns a copy of the disk writing parameters.  These values
     * will be current as of the most recent global update of the local
     * cache from the infospace (see the updateAllParams() method) or
     * the most recent update of only the disk writing parameters
     * (see the updateDiskWritingParams() method).
     */
    struct DiskWritingParams getDiskWritingParams() const;

    /**
     * Returns a copy of the DQM processing parameters.  These values
     * will be current as of the most recent global update of the local
     * cache from the infospace (see the globalUpdate() method).
     */
    struct DQMProcessingParams getDQMProcessingParams() const;

    /**
     * Returns a copy of the event serving parameters.  These values
     * will be current as of the most recent global update of the local
     * cache from the infospace (see the globalUpdate() method).
     */
    struct EventServingParams getEventServingParams() const;

    /**
     * Updates the local copy of all configuration parameters from
     * the infospace.
     */
    void updateAllParams();

    /**
     * Updates the local copy of the disk writing configuration parameters
     * from the infospace.
     */
    void updateDiskWritingParams();

    /**
     * Tests whether the stream configuration string has changed in
     * the infospace.  Returns true if it has changed, false if not.
     */
    bool streamConfigurationHasChanged() const;

    /**
     * Gets invoked when a operation is performed on the infospace
     * that we are interested in knowing about.
     */
    void actionPerformed(xdata::Event& isEvt);

    /**
     * Sets the current list of event stream configuration info
     * objects.
     */
    void setCurrentEventStreamConfig(EvtStrConfList cfgList);

    /**
     * Sets the current list of error stream configuration info
     * objects.
     */
    void setCurrentErrorStreamConfig(ErrStrConfList cfgList);

    /**
     * Retrieves the current list of event stream configuration info
     * objects.
     */
    EvtStrConfList getCurrentEventStreamConfig() const;

    /**
     * Retrieves the current list of error stream configuration info
     * objects.
     */
    ErrStrConfList getCurrentErrorStreamConfig() const;

  private:

    void setDiskWritingDefaults(unsigned long instanceNumber);
    void setDQMProcessingDefaults();
    void setEventServingDefaults();

    void setupDiskWritingInfoSpaceParams(xdata::InfoSpace* infoSpace);
    void setupDQMProcessingInfoSpaceParams(xdata::InfoSpace* infoSpace);
    void setupEventServingInfoSpaceParams(xdata::InfoSpace* infoSpace);

    void updateLocalDiskWritingData();
    void updateLocalDQMProcessingData();
    void updateLocalEventServingData();

    struct DiskWritingParams _diskWriteParamCopy;
    struct DQMProcessingParams _dqmParamCopy;
    struct EventServingParams _eventServeParamCopy;

    mutable boost::mutex _generalMutex;

    std::string _previousStreamCfg;
    bool _streamConfigurationChanged;

    xdata::String _streamConfiguration;
    xdata::String _fileName;
    xdata::String _filePath;
    xdata::String _fileCatalog;
    xdata::String _setupLabel;
    xdata::Integer _nLogicalDisk;
    xdata::Integer _maxFileSize;
    xdata::Double _highWaterMark;
    xdata::Double _lumiSectionTimeOut;
    xdata::Integer _fileClosingTestInterval;
    xdata::Boolean _exactFileSizeTest;

    xdata::Boolean _pushmode2proxy;
    xdata::Double _maxESEventRate;  // hertz
    xdata::Double _maxESDataRate;  // MB/sec
    xdata::Integer _activeConsumerTimeout;  // seconds
    xdata::Integer _idleConsumerTimeout;  // seconds
    xdata::Integer _consumerQueueSize;
    xdata::Boolean _fairShareES;
    xdata::Double _DQMmaxESEventRate;  // hertz
    xdata::Integer _DQMactiveConsumerTimeout;  // seconds
    xdata::Integer _DQMidleConsumerTimeout;  // seconds
    xdata::Integer _DQMconsumerQueueSize;
    xdata::String _esSelectedHLTOutputModule;

    xdata::Boolean _collateDQM;
    xdata::Boolean _archiveDQM;
    xdata::Integer _archiveIntervalDQM;
    xdata::String  _filePrefixDQM;
    xdata::Integer _purgeTimeDQM;
    xdata::Integer _readyTimeDQM;
    xdata::Boolean _useCompressionDQM;
    xdata::Integer _compressionLevelDQM;


    mutable boost::mutex _evtStrCfgMutex;
    mutable boost::mutex _errStrCfgMutex;

    EvtStrConfList _currentEventStreamConfig;
    ErrStrConfList _currentErrorStreamConfig;
  };

}

#endif

/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -

