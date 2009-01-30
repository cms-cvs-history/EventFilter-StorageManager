// $Id: MonitoredQuantity.h,v 1.1.2.2 2009/01/29 22:13:51 biery Exp $

#ifndef StorageManager_MonitoredQuantity_h
#define StorageManager_MonitoredQuantity_h

#include "boost/shared_ptr.hpp"
#include "boost/thread/mutex.hpp"
#include <vector>

namespace stor
{

  /**
   * This class keeps track of statistics for a set of sample values 
   * and provides timing information on the samples.
   *
   * $Author: mommsen $
   * $Revision: 1.1.2.1 $
   * $Date: 2009/01/29 17:17:14 $
   */

  class MonitoredQuantity
  {
    
  public:

    static const int EXPECTED_CALCULATION_INTERVAL = 1;  // seconds

    enum DataSetType { FULL = 0,      // the full data set (all samples)
                       RECENT = 1 };  // recent data only

    MonitoredQuantity(double timeWindowForRecentResults = 10.0);

    /**
     * Adds the specified sample value to the monitor instance.
     */
    void addSample(double value = 1.0);

    /**
     * Returns the number of samples stored in the monitor.
     */
    long long getSampleCount(DataSetType dataSet = FULL);

    /**
     * Returns the rate of samples stored in the monitor
     * (number of samples divided by duration).  The units of the
     * return value are samples per second.
     */
    double getSampleRate(DataSetType dataSet = FULL);

    /**
     * Returns the sum of all sample values stored in the monitor.
     */
    double getValueSum(DataSetType dataSet = FULL);

    /**
     * Returns the average value of the samples that have been stored in
     * the monitor or 0.0 if no samples have been added.
     */
    double getValueAverage(DataSetType dataSet = FULL);

    /**
     * Returns the RMS of the sample values that have been stored in
     * the monitor or 0.0 if no samples have been added.
     */
    double getValueRMS(DataSetType dataSet = FULL);

    /**
     * Returns the minimum sample value that has been stored in
     * the monitor.
     */
    double getValueMin(DataSetType dataSet = FULL);

    /**
     * Returns the maximum sample value that has been stored in
     * the monitor.
     */
    double getValueMax(DataSetType dataSet = FULL);

    /**
     * Returns the sample value rate (the sum of all sample values stored
     * in the monitor divided by the duration) or 0.0 if no samples have been
     * stored.  The units of the return
     * value are [the units of the sample values] per second (e.g. MByte/sec).
     */
    double getValueRate(DataSetType dataSet = FULL);

    /**
     * Returns the amount of time (seconds) that this monitor instance has
     * been processing values starting with the time of the first sample
     * or 0.0 if no samples have been stored in the monitor.
     */
    double getDuration(DataSetType dataSet = FULL);

    /**
     * Forces a calculation of the statistics for the monitored quantity.
     * The frequency of the updates to the statistics is driven by how
     * often this method is called.  It is expected that this method
     * will be called once per interval specified by
     * EXPECTED_CALCULATION_INTERVAL.
     */
    void calculateStatistics(double currentTime = getCurrentTime());

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
    void setNewTimeWindowForRecentResults(double interval);

    /**
     * Tests whether the monitor is currently enabled.
     */
    bool isEnabled() const {return enabled_;}

    /**
     * Returns the length of the time window that has been specified
     * for recent results.  (This may be different than the actual
     * length of the recent time window which is affected by the
     * interval of calls to the calculateStatistics() method.  Use
     * a getDuration(RECENT) call to determine the actual recent
     * time window.)
     */
    double getTimeWindowForRecentResults() const {
      return intervalForRecentStats_;
    }

    /**
     * Returns the current time as a double.  The value corresponds to the
     * number of seconds since the epoch (including a fractional part good to
     * the microsecond level).  A negative value indicates that an error
     * occurred when fetching the time from the operating system.
     */
    static double getCurrentTime();

  private:

    double lastCalculationTime_;
    long long workingSampleCount_;
    double workingValueSum_;
    double workingValueSumOfSquares_;
    double workingValueMin_;
    double workingValueMax_;
    boost::mutex accumulationMutex_;

    int binCount_;
    int workingBinId_;
    boost::shared_ptr< std::vector<long long> > binSampleCount_;
    boost::shared_ptr< std::vector<double> > binValueSum_;
    boost::shared_ptr< std::vector<double> > binValueSumOfSquares_;
    boost::shared_ptr< std::vector<double> > binValueMin_;
    boost::shared_ptr< std::vector<double> > binValueMax_;
    boost::shared_ptr< std::vector<double> > binDuration_;

    long long fullSampleCount_;
    double fullSampleRate_;
    double fullValueSum_;
    double fullValueSumOfSquares_;
    double fullValueAverage_;
    double fullValueRMS_;
    double fullValueMin_;
    double fullValueMax_;
    double fullValueRate_;
    double fullDuration_;

    long long recentSampleCount_;
    double recentSampleRate_;
    double recentValueSum_;
    double recentValueSumOfSquares_;
    double recentValueAverage_;
    double recentValueRMS_;
    double recentValueMin_;
    double recentValueMax_;
    double recentValueRate_;
    double recentDuration_;

    boost::mutex resultsMutex_;

    bool enabled_;
    double intervalForRecentStats_;  // seconds

  };

} // namespace stor

#endif // StorageManager_MonitoredQuantity_h



/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
