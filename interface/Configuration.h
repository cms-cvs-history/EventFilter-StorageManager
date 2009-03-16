// $Id: Configuration.h,v 1.1.2.1 2009/03/13 13:41:28 biery Exp $


#ifndef EventFilter_StorageManager_Configuration_h
#define EventFilter_StorageManager_Configuration_h

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
   * Class for managing configuration information from the infospace
   * and providing local copies of that information that are updated
   * only at requested times.
   *
   * $Author: biery $
   * $Revision: 1.1.2.1 $
   * $Date: 2009/03/13 13:41:28 $
   */

  struct DiskWritingParams
  {
    std::string _streamConfiguration;
    std::string _fileName;
    std::string _filePath;
    std::string _fileCatalog;
    std::string _setupLabel;
    int _nLogicalDisk;
    int _maxFileSize;
    double _highWaterMark;  // not actually used for anything, KAB 16-Mar-09
    utils::duration_t _lumiSectionTimeOut;
    utils::duration_t _fileClosingTestInterval;
    bool _exactFileSizeTest;

    // not mapped to infospace params
    std::string _smInstanceString;
    std::string _hostName;
    int _initialSafetyLevel;  // what is this used for?
  };

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

  // some of these may go away once we get rid of the event server
  struct EventServingParams
  {
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
    struct DQMProcessingParams getDQMParams() const;

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

    xdata::String _previousStreamCfg;
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

    xdata::Boolean pushmode2proxy_;
    xdata::Double maxESEventRate_;  // hertz
    xdata::Double maxESDataRate_;  // MB/sec
    xdata::Integer activeConsumerTimeout_;  // seconds
    xdata::Integer idleConsumerTimeout_;  // seconds
    xdata::Integer consumerQueueSize_;
    xdata::Boolean fairShareES_;
    xdata::Double DQMmaxESEventRate_;  // hertz
    xdata::Integer DQMactiveConsumerTimeout_;  // seconds
    xdata::Integer DQMidleConsumerTimeout_;  // seconds
    xdata::Integer DQMconsumerQueueSize_;
    boost::mutex consumerInitMsgLock_;
    xdata::String esSelectedHLTOutputModule_;


    xdata::Boolean collateDQM_;
    xdata::Boolean archiveDQM_;
    xdata::Integer archiveIntervalDQM_;
    xdata::String  filePrefixDQM_;
    xdata::Integer purgeTimeDQM_;
    xdata::Integer readyTimeDQM_;
    xdata::Boolean useCompressionDQM_;
    xdata::Integer compressionLevelDQM_;
  };

}

#endif

/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -

