// $Id: StreamsMonitorCollection.h,v 1.1.2.2 2009/04/06 18:29:51 mommsen Exp $

#ifndef StorageManager_StreamsMonitorCollection_h
#define StorageManager_StreamsMonitorCollection_h

#include <sstream>
#include <iomanip>
#include <vector>

#include <boost/shared_ptr.hpp>

#include "xdata/UnsignedInteger32.h"

#include "EventFilter/StorageManager/interface/MonitorCollection.h"
#include "EventFilter/StorageManager/interface/Utils.h"


namespace stor {

  /**
   * A collection of MonitoredQuantities of output streams
   *
   * $Author: mommsen $
   * $Revision: 1.1.2.2 $
   * $Date: 2009/04/06 18:29:51 $
   */
  
  class StreamsMonitorCollection : public MonitorCollection
  {
  private:

    static MonitoredQuantity _allStreamsFileCount;
    static MonitoredQuantity _allStreamsVolume;
    static MonitoredQuantity _allStreamsBandwidth;

  public:

    struct StreamRecord
    {
      std::string streamName;       // name of the stream
      MonitoredQuantity fileCount;  // number of files written for this stream
      MonitoredQuantity volume;     // data in MBytes stored in this stream
      MonitoredQuantity bandwidth;  // bandwidth in MBytes for this stream
      void incrementFileCount();
      void addSizeInBytes(double);
    };

    // We do not know how many streams there will be.
    // Thus, we need a vector of them.
    typedef boost::shared_ptr<StreamRecord> StreamRecordPtr;
    typedef std::vector<StreamRecordPtr> StreamRecordList;


    explicit StreamsMonitorCollection(xdaq::Application*);

    const StreamRecordPtr getNewStreamRecord();

    const StreamRecordList& getStreamRecordsMQ() const {
      return _streamRecords;
    }
    StreamRecordList& getStreamRecordsMQ() {
      return _streamRecords;
    }

    const MonitoredQuantity& getAllStreamsFileCountMQ() const {
      return _allStreamsFileCount;
    }
    MonitoredQuantity& getAllStreamsFileCountMQ() {
      return _allStreamsFileCount;
    }

    const MonitoredQuantity& getAllStreamsVolumeMQ() const {
      return _allStreamsVolume;
    }
    MonitoredQuantity& getAllStreamsVolumeMQ() {
      return _allStreamsVolume;
    }

    const MonitoredQuantity& getAllStreamsBandwidthMQ() const {
      return _allStreamsBandwidth;
    }
    MonitoredQuantity& getAllStreamsBandwidthMQ() {
      return _allStreamsBandwidth;
    }



  private:

    //Prevent copying of the StreamsMonitorCollection
    StreamsMonitorCollection(StreamsMonitorCollection const&);
    StreamsMonitorCollection& operator=(StreamsMonitorCollection const&);

    virtual void do_calculateStatistics();
    
    virtual void do_updateInfoSpace();

    virtual void do_reset();

    StreamRecordList _streamRecords;
    utils::duration_t _timeWindowForRecentResults;

    // InfoSpace items which were defined in the old SM


  };
  
} // namespace stor

#endif // StorageManager_StreamsMonitorCollection_h 


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
