// $Id: StreamsMonitorCollection.h,v 1.1.2.3 2009/04/08 09:34:41 mommsen Exp $

#ifndef StorageManager_StreamsMonitorCollection_h
#define StorageManager_StreamsMonitorCollection_h

#include <sstream>
#include <iomanip>
#include <vector>

#include <boost/shared_ptr.hpp>

#include "xdata/String.h"
#include "xdata/UnsignedInteger32.h"
#include "xdata/Vector.h"

#include "EventFilter/StorageManager/interface/MonitorCollection.h"
#include "EventFilter/StorageManager/interface/Utils.h"


namespace stor {

  /**
   * A collection of MonitoredQuantities of output streams
   *
   * $Author: mommsen $
   * $Revision: 1.1.2.3 $
   * $Date: 2009/04/08 09:34:41 $
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
    xdata::UnsignedInteger32 _storedEvents;                        // number of events stored in all streams
    xdata::Vector<xdata::String> _namesOfStream;                   // vector of stream names
    xdata::Vector<xdata::UnsignedInteger32> _storedEventsInStream; // vector of events stored in stream N

  };
  
} // namespace stor

#endif // StorageManager_StreamsMonitorCollection_h 


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
