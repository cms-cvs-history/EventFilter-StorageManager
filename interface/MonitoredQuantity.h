// $Id: MonitoredQuantity.h,v 1.1.2.13 2009/03/30 14:40:49 paterno Exp $

#ifndef StorageManager_MonitoredQuantity_h
#define StorageManager_MonitoredQuantity_h

#include "boost/shared_ptr.hpp"
#include "boost/thread/mutex.hpp"
#include <vector>

#include "EventFilter/StorageManager/interface/Utils.h"


namespace stor
{

  /**
   * This class keeps track of statistics for a set of sample values 
   * and provides timing information on the samples.
   *
   * $Author: paterno $
   * $Revision: 1.1.2.13 $
   * $Date: 2009/03/30 14:40:49 $
   */

  class MonitoredQuantity
  {
    
  public:
    enum DataSetType { FULL = 0,      // the full data set (all samples)
                       RECENT = 1 };  // recent data only

    static utils::duration_t ExpectedCalculationInterval();

    explicit MonitoredQuantity(double timeWindowForRecentResults = 10.0);

    /**
     * Adds the specified doubled valued sample value to the monitor instance.
     */
    void addSample(const double value = 1.0);

    /**
     * Adds the specified integer valued sample value to the monitor instance.
     */
    void addSample(const int value = 1);

    /**
     * Adds the specified uint32_t valued sample value to the monitor instance.
     */
    void addSample(const uint32_t value = 1);

    /**
     * Returns the number of samples stored in the monitor.
     */
    long long getSampleCount(DataSetType dataSet = FULL) const;

    /**
     * Returns the rate of samples stored in the monitor
     * (number of samples divided by duration).  The units of the
     * return value are samples per second.
     */
    double getSampleRate(DataSetType dataSet = FULL) const;

    /**
     * Returns the latency of samples stored in the monitor
     * (duration divided by number of samples).  The units of the
     * return value are microsecond per sample.
     */
    double getSampleLatency(DataSetType dataSet = FULL) const;

    /**
     * Returns the sum of all sample values stored in the monitor.
     */
    double getValueSum(DataSetType dataSet = FULL) const;

    /**
     * Returns the average value of the samples that have been stored in
     * the monitor or 0.0 if no samples have been added.
     */
    double getValueAverage(DataSetType dataSet = FULL) const;

    /**
     * Returns the RMS of the sample values that have been stored in
     * the monitor or 0.0 if no samples have been added.
     */
    double getValueRMS(DataSetType dataSet = FULL) const;

    /**
     * Returns the minimum sample value that has been stored in
     * the monitor.
     */
    double getValueMin(DataSetType dataSet = FULL) const;

    /**
     * Returns the maximum sample value that has been stored in
     * the monitor.
     */
    double getValueMax(DataSetType dataSet = FULL) const;

    /**
     * Returns the sample value rate (the sum of all sample values stored
     * in the monitor divided by the duration) or 0.0 if no samples have been
     * stored.  The units of the return
     * value are [the units of the sample values] per second (e.g. MByte/sec).
     */
    double getValueRate(DataSetType dataSet = FULL) const;

    /**
     * Returns the amount of time (seconds) that this monitor instance has
     * been processing values starting with the time of the first sample
     * or 0.0 if no samples have been stored in the monitor.
     */
    double getDuration(DataSetType dataSet = FULL) const;

    /**
     * Returns the value of the last added sample
     */
    double getLastSampleValue() const {return _lastSampleValue;}

    /**
     * Forces a calculation of the statistics for the monitored quantity.
     * The frequency of the updates to the statistics is driven by how
     * often this method is called.  It is expected that this method
     * will be called once per interval specified by
     * EXPECTED_CALCULATION_INTERVAL.
     */
    void calculateStatistics(utils::time_point_t currentTime = 
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
     * Specifies a new time interval to be used when calculating
     * "recent" statistics.
     */
    void setNewTimeWindowForRecentResults(utils::duration_t interval);

    /**
     * Tests whether the monitor is currently enabled.
     */
    bool isEnabled() const {return _enabled;}

    /**
     * Returns the length of the time window that has been specified
     * for recent results.  (This may be different than the actual
     * length of the recent time window which is affected by the
     * interval of calls to the calculateStatistics() method.  Use
     * a getDuration(RECENT) call to determine the actual recent
     * time window.)
     */
    utils::duration_t getTimeWindowForRecentResults() const {
      return _intervalForRecentStats;
    }

  private:

    // Prevent copying of the MonitoredQuantity
    MonitoredQuantity(MonitoredQuantity const&);
    MonitoredQuantity& operator=(MonitoredQuantity const&);

    // Helper functions.
    void _reset_accumulators();
    void _reset_results();

    utils::time_point_t _lastCalculationTime;
    double _lastSampleValue;
    long long _workingSampleCount;
    double _workingValueSum;
    double _workingValueSumOfSquares;
    double _workingValueMin;
    double _workingValueMax;
    boost::mutex _accumulationMutex;

    int _binCount;
    int _workingBinId;
    std::vector<long long> _binSampleCount;
    std::vector<double> _binValueSum;
    std::vector<double> _binValueSumOfSquares;
    std::vector<double> _binValueMin;
    std::vector<double> _binValueMax;
    std::vector<utils::duration_t> _binDuration;

    long long _fullSampleCount;
    double _fullSampleRate;
    double _fullValueSum;
    double _fullValueSumOfSquares;
    double _fullValueAverage;
    double _fullValueRMS;
    double _fullValueMin;
    double _fullValueMax;
    double _fullValueRate;
    utils::duration_t _fullDuration;

    long long _recentSampleCount;
    double _recentSampleRate;
    double _recentValueSum;
    double _recentValueSumOfSquares;
    double _recentValueAverage;
    double _recentValueRMS;
    double _recentValueMin;
    double _recentValueMax;
    double _recentValueRate;
    utils::duration_t _recentDuration;

    mutable boost::mutex _resultsMutex;

    bool _enabled;
    utils::duration_t _intervalForRecentStats;  // seconds

  };

} // namespace stor

#endif // StorageManager_MonitoredQuantity_h



/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
