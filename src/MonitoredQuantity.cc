// $Id: MonitoredQuantity.cc,v 1.1.2.5 2009/02/05 10:14:50 mommsen Exp $

#include "EventFilter/StorageManager/interface/MonitoredQuantity.h"
#include <sys/time.h>
#include <math.h>

using namespace stor;

MonitoredQuantity::MonitoredQuantity(double timeWindowForRecentResults):
  enabled_(true)
{
  setNewTimeWindowForRecentResults(timeWindowForRecentResults);
}

void MonitoredQuantity::addSample(double value)
{
  if (! enabled_) {return;}

  boost::mutex::scoped_lock sl(accumulationMutex_);

  if (lastCalculationTime_ <= 0.0) {
    lastCalculationTime_ = getCurrentTime();
  }

  ++workingSampleCount_;
  workingValueSum_ += value;
  workingValueSumOfSquares_ += (value * value);

  if (value < workingValueMin_) workingValueMin_ = value;
  if (value > workingValueMax_) workingValueMax_ = value;
}

void  MonitoredQuantity::addSample(int value)
{
  addSample(static_cast<double>(value));
}

long long MonitoredQuantity::getSampleCount(DataSetType dataSet) const
{
  boost::mutex::scoped_lock sl(resultsMutex_);

  if (dataSet == RECENT) {
    return recentSampleCount_;
  }
  else {
    return fullSampleCount_;
  }
}

double MonitoredQuantity::getSampleRate(DataSetType dataSet) const
{
  boost::mutex::scoped_lock sl(resultsMutex_);

  if (dataSet == RECENT) {
    return recentSampleRate_;
  }
  else {
    return fullSampleRate_;
  }
}

double MonitoredQuantity::getSampleLatency(DataSetType dataSet) const
{
  double value = getSampleRate(dataSet);
  return (value) ? 1e6/value : 0;
}

double MonitoredQuantity::getValueSum(DataSetType dataSet) const
{
  boost::mutex::scoped_lock sl(resultsMutex_);

  if (dataSet == RECENT) {
    return recentValueSum_;
  }
  else {
    return fullValueSum_;
  }
}

double MonitoredQuantity::getValueAverage(DataSetType dataSet) const
{
  boost::mutex::scoped_lock sl(resultsMutex_);

  if (dataSet == RECENT) {
    return recentValueAverage_;
  }
  else {
    return fullValueAverage_;
  }
}

double MonitoredQuantity::getValueRMS(DataSetType dataSet) const
{
  boost::mutex::scoped_lock sl(resultsMutex_);

  if (dataSet == RECENT) {
    return recentValueRMS_;
  }
  else {
    return fullValueRMS_;
  }
}

double MonitoredQuantity::getValueMin(DataSetType dataSet) const
{
  boost::mutex::scoped_lock sl(resultsMutex_);

  if (dataSet == RECENT) {
    return recentValueMin_;
  }
  else {
    return fullValueMin_;
  }
}

double MonitoredQuantity::getValueMax(DataSetType dataSet) const
{
  boost::mutex::scoped_lock sl(resultsMutex_);

  if (dataSet == RECENT) {
    return recentValueMax_;
  }
  else {
    return fullValueMax_;
  }
}

double MonitoredQuantity::getValueRate(DataSetType dataSet) const
{
  boost::mutex::scoped_lock sl(resultsMutex_);

  if (dataSet == RECENT) {
    return recentValueRate_;
  }
  else {
    return fullValueRate_;
  }
}

double MonitoredQuantity::getDuration(DataSetType dataSet) const
{
  boost::mutex::scoped_lock sl(resultsMutex_);

  if (dataSet == RECENT) {
    return recentDuration_;
  }
  else {
    return fullDuration_;
  }
}

void MonitoredQuantity::calculateStatistics(double currentTime)
{
  if (! enabled_) {return;}
  if (lastCalculationTime_ <= 0.0) {return;}

  // create local copies of the working values to minimize the
  // time that we could block a thread trying to add a sample.
  // Also, reset the working values.
  long long latestSampleCount;
  double latestValueSum;
  double latestValueSumOfSquares;
  double latestValueMin;
  double latestValueMax;
  double latestDuration;
  {
    boost::mutex::scoped_lock sl(accumulationMutex_);

    latestSampleCount = workingSampleCount_;
    latestValueSum = workingValueSum_;
    latestValueSumOfSquares = workingValueSumOfSquares_;
    latestValueMin = workingValueMin_;
    latestValueMax = workingValueMax_;
    latestDuration = currentTime - lastCalculationTime_;

    lastCalculationTime_ = currentTime;
    workingSampleCount_ = 0;
    workingValueSum_ = 0.0;
    workingValueSumOfSquares_ = 0.0;
    workingValueMin_ =  999999999.0;
    workingValueMax_ = -999999999.0;
  }

  // lock out any interaction with the results while we update them
  {
    boost::mutex::scoped_lock sl(resultsMutex_);

    // we simply add the latest results to the full set
    fullSampleCount_ += latestSampleCount;
    fullValueSum_ += latestValueSum;
    fullValueSumOfSquares_ += latestValueSumOfSquares;
    if (latestValueMin < fullValueMin_) {fullValueMin_ = latestValueMin;}
    if (latestValueMax > fullValueMax_) {fullValueMax_ = latestValueMax;}
    fullDuration_ += latestDuration;

    // for the recent results, we need to replace the contents of
    // the working bin and re-calculate the recent values
    binSampleCount_[workingBinId_] = latestSampleCount;
    binValueSum_[workingBinId_] = latestValueSum;
    binValueSumOfSquares_[workingBinId_] = latestValueSumOfSquares;
    binValueMin_[workingBinId_] = latestValueMin;
    binValueMax_[workingBinId_] = latestValueMax;
    binDuration_[workingBinId_] = latestDuration;

    recentSampleCount_ = 0;
    recentValueSum_ = 0.0;
    recentValueSumOfSquares_ = 0.0;
    recentValueMin_ =  999999999.0;
    recentValueMax_ = -999999999.0;
    recentDuration_ = 0.0;

    for (int idx = 0; idx < binCount_; ++idx) {
      recentSampleCount_ += binSampleCount_[idx];
      recentValueSum_ += binValueSum_[idx];
      recentValueSumOfSquares_ += binValueSumOfSquares_[idx];
      if (binValueMin_[idx] < recentValueMin_) {
        recentValueMin_ = binValueMin_[idx];
      }
      if (binValueMax_[idx] > recentValueMax_) {
        recentValueMax_ = binValueMax_[idx];
      }
      recentDuration_ += binDuration_[idx];
    }

    // update the working bin ID here so that we are ready for
    // the next calculation request
    ++workingBinId_;
    if (workingBinId_ >= binCount_) {workingBinId_ = 0;}

    // calculate the derived full values
    if (fullDuration_ > 0.0) {
      fullSampleRate_ = static_cast<double>(fullSampleCount_) / fullDuration_;
      fullValueRate_ = static_cast<double>(fullValueSum_) / fullDuration_;
    }
    else {
      fullSampleRate_ = 0.0;
      fullValueRate_ = 0.0;
    }

    if (fullSampleCount_ > 0) {
      fullValueAverage_ = fullValueSum_ / static_cast<double>(fullSampleCount_);

      double squareAvg = fullValueSumOfSquares_ / static_cast<double>(fullSampleCount_);
      double avg = fullValueSum_ / static_cast<double>(fullSampleCount_);
      double sigSquared = squareAvg - avg*avg;
      if(sigSquared > 0.0) {
        fullValueRMS_ = sqrt(sigSquared);
      }
      else {
        fullValueRMS_ = 0.0;
      }
    }
    else {
      fullValueAverage_ = 0.0;
      fullValueRMS_ = 0.0;
    }

    // calculate the derived recent values
    if (recentDuration_ > 0.0) {
      recentSampleRate_ = static_cast<double>(recentSampleCount_) / recentDuration_;
      recentValueRate_ = static_cast<double>(recentValueSum_) / recentDuration_;
    }
    else {
      recentSampleRate_ = 0.0;
      recentValueRate_ = 0.0;
    }

    if (recentSampleCount_ > 0) {
      recentValueAverage_ = recentValueSum_ / static_cast<double>(recentSampleCount_);

      double squareAvg = recentValueSumOfSquares_ /
        static_cast<double>(recentSampleCount_);
      double avg = recentValueSum_ / static_cast<double>(recentSampleCount_);
      double sigSquared = squareAvg - avg*avg;
      if(sigSquared > 0.0) {
        recentValueRMS_ = sqrt(sigSquared);
      }
      else {
        recentValueRMS_ = 0.0;
      }
    }
    else {
      recentValueAverage_ = 0.0;
      recentValueRMS_ = 0.0;
    }
  }
}

void MonitoredQuantity::reset()
{
  {
    boost::mutex::scoped_lock sl(accumulationMutex_);

    lastCalculationTime_ = -1.0;
    workingSampleCount_ = 0;
    workingValueSum_ = 0.0;
    workingValueSumOfSquares_ = 0.0;
    workingValueMin_ =  999999999.0;
    workingValueMax_ = -999999999.0;
  }

  {
    boost::mutex::scoped_lock sl(resultsMutex_);

    workingBinId_ = 0;
    for (int idx = 0; idx < binCount_; ++idx) {
      binSampleCount_[idx] = 0;
      binValueSum_[idx] = 0.0;
      binValueSumOfSquares_[idx] = 0.0;
      binValueMin_[idx] =  999999999.0;
      binValueMax_[idx] = -999999999.0;
      binDuration_[idx] = 0.0;
    }

    fullSampleCount_ = 0;
    fullSampleRate_ = 0.0;
    fullValueSum_ = 0.0;
    fullValueSumOfSquares_ = 0.0;
    fullValueAverage_ = 0.0;
    fullValueRMS_ = 0.0;
    fullValueMin_ =  999999999.0;
    fullValueMax_ = -999999999.0;
    fullValueRate_ = 0.0;
    fullDuration_ = 0.0;

    recentSampleCount_ = 0;
    recentSampleRate_ = 0.0;
    recentValueSum_ = 0.0;
    recentValueSumOfSquares_ = 0.0;
    recentValueAverage_ = 0.0;
    recentValueRMS_ = 0.0;
    recentValueMin_ =  999999999.0;
    recentValueMax_ = -999999999.0;
    recentValueRate_ = 0.0;
    recentDuration_ = 0.0;
  }
}

void MonitoredQuantity::enable()
{
  if (! enabled_) {
    reset();
    enabled_ = true;
  }
}

void MonitoredQuantity::disable()
{
  if (enabled_) {
    enabled_ = false;
  }
}

void MonitoredQuantity::setNewTimeWindowForRecentResults(double interval)
{
  // lock the results objects since we're dramatically changing the
  // bins used for the recent results
  {
    boost::mutex::scoped_lock sl(resultsMutex_);

    intervalForRecentStats_ = interval;

    // determine how many bins we should use in our sliding window
    // by dividing the input time window by the expected calculation
    // interval and rounding to the nearest integer.
    binCount_ =
      static_cast<int>((intervalForRecentStats_ / 
                        static_cast<double>(EXPECTED_CALCULATION_INTERVAL)) +
                       0.5);

    // create the vectors for the binned quantities
    binSampleCount_.reserve(binCount_);
    binValueSum_.reserve(binCount_);
    binValueSumOfSquares_.reserve(binCount_);
    binValueMin_.reserve(binCount_);
    binValueMax_.reserve(binCount_);
    binDuration_.reserve(binCount_);
  }

  // call the reset method to populate the correct initial values
  // for the internal sample data
  reset();
}

double MonitoredQuantity::getCurrentTime()
{
  double now = -1.0;
  struct timeval timeStruct;
  int status = gettimeofday(&timeStruct, 0);
  if (status == 0) {
    now = static_cast<double>(timeStruct.tv_sec) +
      (static_cast<double>(timeStruct.tv_usec) / 1000000.0);
  }
  return now;
}



/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
