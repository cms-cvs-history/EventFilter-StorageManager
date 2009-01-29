// $Id: MonitoredQuantity.h,v 1.1 2008/04/14 15:42:28 biery Exp $

#ifndef StorageManager_MonitoredQuantity_h
#define StorageManager_MonitoredQuantity_h

namespace stor
{

  /**
   * This class keeps track of statistics for a set of sample values 
   * and provides timing information on the samples.
   *
   * \author $Author: $
   * \version $Revision: $
   * \date $Date: $
   *
   */

  class MonitoredQuantity
  {
    
  public:

    enum DataSetType { FULL = 0,      // the full data set (all samples)
                       RECENT = 1 };  // recent data only

    MonitoredQuantity();

    void addSample(double value = 1.0);
    long long getSampleCount(DataSetType dataSet = FULL) const;
    double getSampleRate(DataSetType dataSet = FULL,
                         double currentTime = getCurrentTime()) const;
    double getValueSum(DataSetType dataSet = FULL) const;
    double getValueAverage(DataSetType dataSet = FULL) const;
    double getValueRMS(DataSetType dataSet = FULL) const;
    double getValueMin(DataSetType dataSet = FULL) const;
    double getValueMax(DataSetType dataSet = FULL) const;
    double getValueRate(DataSetType dataSet = FULL,
                        double currentTime = getCurrentTime()) const;
    double getDuration(DataSetType dataSet = FULL,
                       double currentTime = getCurrentTime()) const;

    void latchRecentValuesIfNeeded(double currentTime = getCurrentTime());
    void reset();

    void enable();
    void disable();
    void setNewIntervalForRecentData(double interval);

    bool isEnabled() const {return enabled_;}
    double getIntervalForRecentData() const {return intervalForRecentStats_;}

    static double getCurrentTime();

  private:

    long long sampleCount_;
    double startTime_;
    double overallTotal_;
    double overallSumOfSquares_;
    double overallMin_;
    double overallMax_;

    double lastLatchTime_;
    long long previousSampleCount_;
    double previousTotal_;
    double previousSumOfSquares_;
    double freshMin_;
    double freshMax_;

    long long recentSampleCount_;
    double recentSampleRate_;
    double recentValueSum_;
    double recentValueAverage_;
    double recentValueRMS_;
    double recentValueMin_;
    double recentValueMax_;
    double recentValueRate_;
    double recentDuration_;

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
