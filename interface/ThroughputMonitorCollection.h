// $Id: ThroughputMonitorCollection.h,v 1.1.2.7 2009/05/04 17:54:09 biery Exp $

#ifndef StorageManager_ThroughputMonitorCollection_h
#define StorageManager_ThroughputMonitorCollection_h

#include <boost/thread/mutex.hpp>
#include <boost/shared_ptr.hpp>

#include "EventFilter/StorageManager/interface/FragmentQueue.h"
#include "EventFilter/StorageManager/interface/MonitorCollection.h"
#include "EventFilter/StorageManager/interface/StreamQueue.h"

namespace stor {

  /**
   * A collection of MonitoredQuantities to track the flow of data
   * through the storage manager.
   *
   * $Author: biery $
   * $Revision: 1.1.2.7 $
   * $Date: 2009/05/04 17:54:09 $
   */
  
  class ThroughputMonitorCollection : public MonitorCollection
  {
  public:

    explicit ThroughputMonitorCollection(xdaq::Application*);

    int getBinCount() const {return _binCount;}

    void setFragmentQueue(boost::shared_ptr<FragmentQueue> fragmentQueue) {
      _fragmentQueue = fragmentQueue;
    }

    const MonitoredQuantity& getFragmentQueueEntryCountMQ() const {
      return _entriesInFragmentQueue;
    }
    MonitoredQuantity& getFragmentQueueEntryCountMQ() {
      return _entriesInFragmentQueue;
    }

    void addPoppedFragmentSample(double dataSize);

    const MonitoredQuantity& getPoppedFragmentSizeMQ() const {
      return _poppedFragmentSize;
    }
    MonitoredQuantity& getPoppedFragmentSizeMQ() {
      return _poppedFragmentSize;
    }

    void addFragmentProcessorIdleSample(utils::duration_t idleTime);

    const MonitoredQuantity& getFragmentProcessorIdleMQ() const {
      return _fragmentProcessorIdleTime;
    }
    MonitoredQuantity& getFragmentProcessorIdleMQ() {
      return _fragmentProcessorIdleTime;
    }

    void setStreamQueue(boost::shared_ptr<StreamQueue> streamQueue) {
      _streamQueue = streamQueue;
    }

    const MonitoredQuantity& getStreamQueueEntryCountMQ() const {
      return _entriesInStreamQueue;
    }
    MonitoredQuantity& getStreamQueueEntryCountMQ() {
      return _entriesInStreamQueue;
    }

    void addEventSizeSample(double size);

    const MonitoredQuantity& getPoppedEventSizeMQ() const {
      return _poppedEventSize;
    }
    MonitoredQuantity& getPoppedEventSizeMQ() {
      return _poppedEventSize;
    }

    void addDiskWriterIdleSample(utils::duration_t idleTime);

    const MonitoredQuantity& getDiskWriterIdleMQ() const {
      return _diskWriterIdleTime;
    }
    MonitoredQuantity& getDiskWriterIdleMQ() {
      return _diskWriterIdleTime;
    }

    void addDiskWriteSample(double size);

    const MonitoredQuantity& getDiskWriteMQ() const {
      return _diskWriteSize;
    }
    MonitoredQuantity& getDiskWriteMQ() {
      return _diskWriteSize;
    }

  private:

    //Prevent copying of the ThroughputMonitorCollection
    ThroughputMonitorCollection(ThroughputMonitorCollection const&);
    ThroughputMonitorCollection& operator=(ThroughputMonitorCollection const&);

    virtual void do_calculateStatistics();

    virtual void do_updateInfoSpace();

    virtual void do_reset();

    int _binCount;

    MonitoredQuantity _entriesInFragmentQueue;
    MonitoredQuantity _poppedFragmentSize;
    MonitoredQuantity _fragmentProcessorIdleTime;

    MonitoredQuantity _entriesInStreamQueue;
    MonitoredQuantity _poppedEventSize;
    MonitoredQuantity _diskWriterIdleTime;
    MonitoredQuantity _diskWriteSize;

    boost::shared_ptr<FragmentQueue> _fragmentQueue;
    boost::shared_ptr<StreamQueue> _streamQueue;

  };
  
} // namespace stor

#endif // StorageManager_ThroughputMonitorCollection_h 


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
