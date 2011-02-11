// $Id: MonitoredQuantity.h,v 1.9.2.1 2011/01/24 12:18:39 mommsen Exp $
/// @file: MonitoredQuantity.h 

#ifndef EventFilter_StorageManager_MonitoredQuantity_h
#define EventFilter_StorageManager_MonitoredQuantity_h

#include "boost/shared_ptr.hpp"
#include "boost/thread/mutex.hpp"

#include <math.h>
#include <stdint.h>
#include <vector>

#include "EventFilter/StorageManager/interface/Utils.h"


namespace stor
{

  /**
   * This class keeps track of statistics for a set of sample values 
   * and provides timing information on the samples.
   *
   * $Author: mommsen $
   * $Revision: 1.9.2.1 $
   * $Date: 2011/01/24 12:18:39 $
   */

  class MonitoredQuantity
  {
    
  public:
    class Stats;

    enum DataSetType { FULL = 0,      // the full data set (all samples)
                       RECENT = 1 };  // recent data only

    explicit MonitoredQuantity
    (
      utils::duration_t expectedCalculationInterval,
      utils::duration_t timeWindowForRecentResults
    );

    /**
     * Adds the specified doubled valued sample value to the monitor instance.
     */
    void addSample(const double& value = 1);

    /**
     * Adds the specified integer valued sample value to the monitor instance.
     */
    void addSample(const int& value = 1);

    /**
     * Adds the specified unsigned integer valued sample value to the monitor instance.
     */
    void addSample(const unsigned int& value = 1);

    /**
     * Adds the specified long valued sample value to the monitor instance.
     */
    void addSample(const long& value = 1);

    /**
     * Adds the specified unsigned long valued sample value to the monitor instance.
     */
    void addSample(const unsigned long& value = 1);

    /**
     * Adds the specified long long valued sample value to the monitor instance.
     */
    void addSample(const long long& value = 1);

    /**
     * Adds the specified unsigned long long valued sample value to the monitor instance.
     */
    void addSample(const unsigned long long& value = 1);

    /**
     * Adds the specified double valued sample value to the monitor instance
     * if it is larger than the previously added value.
     */
    void addSampleIfLarger(const double& value);

    /**
     * Forces a calculation of the statistics for the monitored quantity.
     * The frequency of the updates to the statistics is driven by how
     * often this method is called.  It is expected that this method
     * will be called once per interval specified by
     * expectedCalculationInterval
     */
    void calculateStatistics(const utils::time_point_t& currentTime = 
                             utils::getCurrentTime());

    /**
     * Resets the monitor (zeroes out all counters and restarts the
     * time interval).
     */
    void reset();

    /**
     * Enables the monitor (and resets the statistics to provide a
     * fresh start).
     */
    void enable();

    /**
     * Disables the monitor.
     */
    void disable();

    /**
     * Tests whether the monitor is currently enabled.
     */
    bool isEnabled() const {return _enabled;}

    /**
     * Specifies a new time interval to be used when calculating
     * "recent" statistics.
     */
    void setNewTimeWindowForRecentResults(const utils::duration_t& interval);

    /**
     * Returns the length of the time window that has been specified
     * for recent results.  (This may be different than the actual
     * length of the recent time window which is affected by the
     * interval of calls to the calculateStatistics() method.  Use
     * a getDuration(RECENT) call to determine the actual recent
     * time window.)
     */
    utils::duration_t getTimeWindowForRecentResults() const
    {
      return _intervalForRecentStats;
    }

    utils::duration_t ExpectedCalculationInterval() const
    {
      return _expectedCalculationInterval;
    }

    /**
       Write all our collected statistics into the given Stats struct.
     */
    void getStats(Stats& stats) const;

  private:

    // Prevent copying of the MonitoredQuantity
    MonitoredQuantity(MonitoredQuantity const&);
    MonitoredQuantity& operator=(MonitoredQuantity const&);

    // Helper functions.
    void _reset_accumulators();
    void _reset_results();

    utils::time_point_t _lastCalculationTime;
    uint64_t _workingSampleCount;
    double _workingValueSum;
    double _workingValueSumOfSquares;
    double _workingValueMin;
    double _workingValueMax;
    double _workingLastSampleValue;

    mutable boost::mutex _accumulationMutex;

    uint32_t _binCount;
    uint32_t _workingBinId;
    std::vector<uint64_t> _binSampleCount;
    std::vector<double> _binValueSum;
    std::vector<double> _binValueSumOfSquares;
    std::vector<double> _binValueMin;
    std::vector<double> _binValueMax;
    std::vector<utils::duration_t> _binDuration;
    std::vector<utils::time_point_t> _binSnapshotTime;

    uint64_t _fullSampleCount;
    double _fullSampleRate;
    double _fullValueSum;
    double _fullValueSumOfSquares;
    double _fullValueAverage;
    double _fullValueRMS;
    double _fullValueMin;
    double _fullValueMax;
    double _fullValueRate;
    utils::duration_t _fullDuration;

    uint64_t _recentSampleCount;
    double _recentSampleRate;
    double _recentValueSum;
    double _recentValueSumOfSquares;
    double _recentValueAverage;
    double _recentValueRMS;
    double _recentValueMin;
    double _recentValueMax;
    double _recentValueRate;
    utils::duration_t _recentDuration;
    double _lastLatchedSampleValue;
    double _lastLatchedValueRate;

    mutable boost::mutex _resultsMutex;

    bool _enabled;
    utils::duration_t _intervalForRecentStats;  // seconds
    const utils::duration_t _expectedCalculationInterval;  // seconds
  };

  struct MonitoredQuantity::Stats
  {
    uint64_t fullSampleCount;
    double fullSampleRate;
    double fullValueSum;
    double fullValueSumOfSquares;
    double fullValueAverage;
    double fullValueRMS;
    double fullValueMin;
    double fullValueMax;
    double fullValueRate;
    double fullSampleLatency;
    utils::duration_t fullDuration;

    uint64_t recentSampleCount;
    double recentSampleRate;
    double recentValueSum;
    double recentValueSumOfSquares;
    double recentValueAverage;
    double recentValueRMS;
    double recentValueMin;
    double recentValueMax;
    double recentValueRate;
    double recentSampleLatency;
    utils::duration_t recentDuration;
    std::vector<uint64_t> recentBinnedSampleCounts;
    std::vector<double> recentBinnedValueSums;
    std::vector<utils::duration_t> recentBinnedDurations;
    std::vector<utils::time_point_t> recentBinnedSnapshotTimes;

    double lastSampleValue;
    double lastValueRate;
    bool   enabled;

    uint64_t getSampleCount(DataSetType t = FULL) const { return t == RECENT ? recentSampleCount : fullSampleCount; }
    double getValueSum(DataSetType t = FULL) const { return t == RECENT ? recentValueSum : fullValueSum; }
    double getValueAverage(DataSetType t = FULL) const { return t == RECENT ? recentValueAverage : fullValueAverage; }
    double getValueRate(DataSetType t = FULL) const { return t== RECENT ? recentValueRate : fullValueRate; }
    double getValueRMS(DataSetType t = FULL) const { return t == RECENT ? recentValueRMS : fullValueRMS; }
    double getValueMin(DataSetType t = FULL) const { return t == RECENT ? recentValueMin : fullValueMin; }
    double getValueMax(DataSetType t = FULL) const { return t == RECENT ? recentValueMax : fullValueMax; }
    utils::duration_t getDuration(DataSetType t = FULL) const { return t == RECENT ? recentDuration : fullDuration; }
    double getSampleRate(DataSetType t = FULL) const { return t == RECENT ? recentSampleRate : fullSampleRate; }
    double getSampleLatency(DataSetType t = FULL) const { double v=getSampleRate(t); return v  ? 1e6/v : INFINITY;}
    double getLastSampleValue() const { return lastSampleValue; }
    double getLastValueRate() const { return lastValueRate; }
    bool   isEnabled() const { return enabled; }
  };

  typedef boost::shared_ptr<MonitoredQuantity> MonitoredQuantityPtr;

} // namespace stor

#endif // EventFilter_StorageManager_MonitoredQuantity_h



/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
