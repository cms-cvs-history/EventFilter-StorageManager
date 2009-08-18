// $Id: ThroughputMonitorCollection.h,v 1.5 2009/08/12 14:58:57 biery Exp $
/// @file: ThroughputMonitorCollection.h 

#ifndef StorageManager_ThroughputMonitorCollection_h
#define StorageManager_ThroughputMonitorCollection_h

#include <boost/thread/mutex.hpp>
#include <boost/shared_ptr.hpp>

#include "EventFilter/StorageManager/interface/DQMEventQueue.h"
#include "EventFilter/StorageManager/interface/FragmentQueue.h"
#include "EventFilter/StorageManager/interface/MonitorCollection.h"
#include "EventFilter/StorageManager/interface/StreamQueue.h"

namespace stor {

  /**
   * A collection of MonitoredQuantities to track the flow of data
   * through the storage manager.
   *
   * $Author: biery $
   * $Revision: 1.5 $
   * $Date: 2009/08/12 14:58:57 $
   */
  
  class ThroughputMonitorCollection : public MonitorCollection
  {
  public:

    explicit ThroughputMonitorCollection(const utils::duration_t& updateInterval);

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

    const MonitoredQuantity& getFragmentStoreEntryCountMQ() const {
      return _entriesInFragmentStore;
    }
    MonitoredQuantity& getFragmentStoreEntryCountMQ() {
      return _entriesInFragmentStore;
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

    void addPoppedEventSample(double dataSize);

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

    void addDiskWriteSample(double dataSize);

    const MonitoredQuantity& getDiskWriteMQ() const {
      return _diskWriteSize;
    }
    MonitoredQuantity& getDiskWriteMQ() {
      return _diskWriteSize;
    }

    void setDQMEventQueue(boost::shared_ptr<DQMEventQueue> dqmEventQueue) {
      _dqmEventQueue = dqmEventQueue;
    }

    const MonitoredQuantity& getDQMEventQueueEntryCountMQ() const {
      return _entriesInDQMEventQueue;
    }
    MonitoredQuantity& getDQMEventQueueEntryCountMQ() {
      return _entriesInDQMEventQueue;
    }

    void addPoppedDQMEventSample(double dataSize);

    const MonitoredQuantity& getPoppedDQMEventSizeMQ() const {
      return _poppedDQMEventSize;
    }
    MonitoredQuantity& getPoppedDQMEventSizeMQ() {
      return _poppedDQMEventSize;
    }

    void addDQMEventProcessorIdleSample(utils::duration_t idleTime);

    const MonitoredQuantity& getDQMEventProcessorIdleMQ() const {
      return _dqmEventProcessorIdleTime;
    }
    MonitoredQuantity& getDQMEventProcessorIdleMQ() {
      return _dqmEventProcessorIdleTime;
    }

    /**
     * Sets the current number of events in the fragment store.
     */
    void setFragmentStoreSize(unsigned int size) {
      // do we really need this lock?
      boost::mutex::scoped_lock sl(_fragmentStoreSizeMutex);
      _currentFragmentStoreSize = size;
    }

    /**
     * Returns the current number of events in the fragment store.
     */
    unsigned int getFragmentStoreSize() {
      // do we really need this lock?
      boost::mutex::scoped_lock sl(_fragmentStoreSizeMutex);
      return _currentFragmentStoreSize;
    }

  private:

    //Prevent copying of the ThroughputMonitorCollection
    ThroughputMonitorCollection(ThroughputMonitorCollection const&);
    ThroughputMonitorCollection& operator=(ThroughputMonitorCollection const&);

    virtual void do_calculateStatistics();
    virtual void do_reset();

    const int _binCount;

    MonitoredQuantity _entriesInFragmentQueue;
    MonitoredQuantity _poppedFragmentSize;
    MonitoredQuantity _fragmentProcessorIdleTime;
    MonitoredQuantity _entriesInFragmentStore;

    MonitoredQuantity _entriesInStreamQueue;
    MonitoredQuantity _poppedEventSize;
    MonitoredQuantity _diskWriterIdleTime;
    MonitoredQuantity _diskWriteSize;

    MonitoredQuantity _entriesInDQMEventQueue;
    MonitoredQuantity _poppedDQMEventSize;
    MonitoredQuantity _dqmEventProcessorIdleTime;

    boost::shared_ptr<FragmentQueue> _fragmentQueue;
    boost::shared_ptr<StreamQueue> _streamQueue;
    boost::shared_ptr<DQMEventQueue> _dqmEventQueue;

    unsigned int _currentFragmentStoreSize;
    mutable boost::mutex _fragmentStoreSizeMutex;

  };
  
} // namespace stor

#endif // StorageManager_ThroughputMonitorCollection_h 


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
