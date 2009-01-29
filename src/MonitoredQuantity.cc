// $Id: MonitoredQuantity.cc,v 1.1 2008/04/14 15:42:28 biery Exp $

#include "EventFilter/StorageManager/interface/MonitoredQuantity.h"
#include <sys/time.h>
#include <math.h>

using namespace stor;

/**
 * Default constructor.
 */
MonitoredQuantity::MonitoredQuantity():
  enabled_(true),
  intervalForRecentStats_(30.0)
{
  reset();
}

/**
 * Adds the specified sample value to the monitor instance.
 */
void MonitoredQuantity::addSample(double value)
{
  if (! enabled_) {return;}

  if (sampleCount_ == 0) {
    startTime_ = getCurrentTime();
    lastLatchTime_ = startTime_;
  }

  ++sampleCount_;
  overallTotal_ += value;
  overallSumOfSquares_ += (value * value);

  if (value < overallMin_) overallMin_ = value;
  if (value > overallMax_) overallMax_ = value;

  if (value < freshMin_) freshMin_ = value;
  if (value > freshMax_) freshMax_ = value;
}

/**
 * Returns the number of samples stored in the monitor.
 */
long long MonitoredQuantity::getSampleCount(DataSetType dataSet) const
{
  if (dataSet == RECENT) {
    return recentSampleCount_;
  }
  else {
    return sampleCount_;
  }
}

/**
 * Returns the rate of samples stored in the monitor
 * (number of samples divided by duration).  The units of the
 * return value are samples per second.
 */
double MonitoredQuantity::getSampleRate(DataSetType dataSet,
                                   double currentTime) const
{
  if (dataSet == RECENT) {
    return recentSampleRate_;
  }
  else {
    double duration = getDuration(FULL, currentTime);
    if (duration > 0.0) {
      return ((double) sampleCount_) / duration;
    }
    else {
      return 0.0;
    }
  }
}

/**
 * Returns the sum of all sample values stored in the monitor.
 */
double MonitoredQuantity::getValueSum(DataSetType dataSet) const
{
  if (dataSet == RECENT) {
    return recentValueSum_;
  }
  else {
    return overallTotal_;
  }
}

/**
 * Returns the average value of the samples that have been stored in
 * the monitor or 0.0 if no samples have been added.
 */
double MonitoredQuantity::getValueAverage(DataSetType dataSet) const
{
  if (dataSet == RECENT) {
    return recentValueAverage_;
  }
  else {
    if (sampleCount_ > 0) {
      return ((double) overallTotal_) / ((double) sampleCount_);
    }
    else {
      return 0.0;
    }
  }
}

/**
 * Returns the RMS of the sample values that have been stored in
 * the monitor or 0.0 if no samples have been added.
 */
double MonitoredQuantity::getValueRMS(DataSetType dataSet) const
{
  if (dataSet == RECENT) {
    return recentValueRMS_;
  }
  else {
    if (sampleCount_ > 0) {
      double squareAvg = (overallSumOfSquares_ / (double) sampleCount_);
      double avg = (overallTotal_ / (double) sampleCount_);
      double sigSquared = squareAvg - avg*avg;
      if(sigSquared > 0.0) {
        return sqrt(sigSquared);
      }
      else {
        return 0.0;
      }
    }
    else {
      return 0.0;
    }
  }
}

/**
 * Returns the minimum sample value that has been stored in
 * the monitor.
 */
double MonitoredQuantity::getValueMin(DataSetType dataSet) const
{
  if (dataSet == RECENT) {
    return recentValueMin_;
  }
  else {
    return overallMin_;
  }
}

/**
 * Returns the maximum sample value that has been stored in
 * the monitor.
 */
double MonitoredQuantity::getValueMax(DataSetType dataSet) const
{
  if (dataSet == RECENT) {
    return recentValueMax_;
  }
  else {
    return overallMax_;
  }
}

/**
 * Returns the sample value rate (the sum of all sample values stored
 * in the monitor divided by the duration) or 0.0 if no samples have been
 * stored.  The units of the return
 * value are [the units of the sample values] per second (e.g. MByte/sec).
 */
double MonitoredQuantity::getValueRate(DataSetType dataSet,
                                  double currentTime) const
{
  if (dataSet == RECENT) {
    return recentValueAverage_;
  }
  else {
    double duration = getDuration(FULL, currentTime);
    if (duration > 0.0) {
      return ((double) overallTotal_) / duration;
    }
    else {
      return 0.0;
    }
  }
}

/**
 * Returns the amount of time (seconds) that this monitor instance has
 * been processing values starting with the time of the first sample
 * or 0.0 if no samples have been stored in the monitor.
 */
double MonitoredQuantity::getDuration(DataSetType dataSet,
                                 double currentTime) const
{
  if (dataSet == RECENT) {
    return recentDuration_;
  }
  else {
    if (sampleCount_ == 0) {
      return 0.0;
    }
    else {
      return (currentTime - startTime_);
    }
  }
}

/**
 * Updates the statistics for the "recent" data set.  This method
 * currently ends one "recent" interval and starts another.
 */
void MonitoredQuantity::latchRecentValuesIfNeeded(double currentTime)
{
  if (! enabled_) {return;}
  if (sampleCount_ == 0) {return;}
  if ((currentTime - lastLatchTime_) < intervalForRecentStats_) {return;}

  recentSampleCount_ = sampleCount_ - previousSampleCount_;
  previousSampleCount_ = sampleCount_;

  recentDuration_ = currentTime - lastLatchTime_;
  lastLatchTime_ = currentTime;

  if (recentDuration_ > 0.0) {
    recentSampleRate_ = ((double) recentSampleCount_) / recentDuration_;
  }
  else {
    recentSampleRate_ = 0.0;
  }

  recentValueSum_ = overallTotal_ - previousTotal_;
  previousTotal_ = overallTotal_;

  if (recentSampleCount_ > 0) {
    recentValueAverage_ = recentValueSum_ / ((double) recentSampleCount_);
  }
  else {
    recentValueAverage_ = 0.0;
  }

  if (recentSampleCount_ > 0) {
    double squareAvg = ((overallSumOfSquares_ - previousSumOfSquares_)
                         / (double) recentSampleCount_);
    previousSumOfSquares_ = overallSumOfSquares_;
    double avg = (recentValueSum_ / (double) recentSampleCount_);
    double sigSquared = squareAvg - avg*avg;
    if(sigSquared > 0.0) {
      recentValueRMS_ = sqrt(sigSquared);
    }
    else {
      recentValueRMS_ = 0.0;
    }
  }
  else {
    recentValueRMS_ = 0.0;
  }

  if (recentSampleCount_ > 0) {
    recentValueMin_ = freshMin_;
    recentValueMax_ = freshMax_;
  }
  else {
    recentValueMin_ = 0.0;
    recentValueMax_ = 0.0;
  }
  freshMin_ =  999999999.0;
  freshMax_ = -999999999.0;

  if (recentDuration_ > 0.0) {
    recentValueRate_ = ((double) recentValueSum_) / recentDuration_;
  }
  else {
    recentValueRate_ = 0.0;
  }
}

/**
 * Resets the monitor (zeroes out all counters and restarts the
 * time interval).
 */
void MonitoredQuantity::reset()
{
    sampleCount_ = 0;
    startTime_ = 0.0;
    overallTotal_ = 0.0;
    overallSumOfSquares_ = 0.0;
    overallMin_ =  999999999.0;
    overallMax_ = -999999999.0;

    lastLatchTime_ = 0.0;
    previousSampleCount_ = 0;;
    previousTotal_ = 0.0;
    previousSumOfSquares_ = 0.0;
    freshMin_ =  999999999.0;
    freshMax_ = -999999999.0;

    recentSampleCount_ = 0;
    recentSampleRate_ = 0.0;
    recentValueSum_ = 0.0;
    recentValueAverage_ = 0.0;
    recentValueRMS_ = 0.0;
    recentValueMin_ = 0.0;
    recentValueMax_ = 0.0;
    recentValueRate_ = 0.0;
    recentDuration_ = 0.0;
}

/**
 * Enables the monitor (and resets the statistics to provide a
 * fresh start).
 */
void MonitoredQuantity::enable()
{
  if (! enabled_) {
    reset();  // do this first to provide some internal consistency
    enabled_ = true;
  }
}

/**
 * Disables the monitor.
 */
void MonitoredQuantity::disable()
{
  if (enabled_) {
    enabled_ = false;
  }
}

/**
 * Specifies a new time interval to be used when calculating
 * "recent" statistics.
 */
void MonitoredQuantity::setNewIntervalForRecentData(double interval)
{
  intervalForRecentStats_ = interval;
}

/**
 * Returns the current time as a double.  The value corresponds to the
 * number of seconds since the epoch (including a fractional part good to
 * the microsecond level).  A negative value indicates that an error
 * occurred when fetching the time from the operating system.
 */
double MonitoredQuantity::getCurrentTime()
{
  double now = -1.0;
  struct timeval timeStruct;
  int status = gettimeofday(&timeStruct, 0);
  if (status == 0) {
    now = ((double) timeStruct.tv_sec) +
      (((double) timeStruct.tv_usec) / 1000000.0);
  }
  return now;
}



/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
