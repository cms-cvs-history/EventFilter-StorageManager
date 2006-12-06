/*
   Author: Harry Cheung, FNAL

   Description:
     Header file for performance statistics for
     Storage Manager and I2O output module.
       This is a modified version of code taken from
     the XDAQ toolbox::PerformanceMeter written by
     J. Gutleber and L. Orsini

   Modification:
     version 1.1 2005/12/15
       Initial implementation.  Modified from toolbox
       version to keep a running mean, and max and min values.

*/

#include "EventFilter/StorageManager/interface/SMPerformanceMeter.h"

void stor::SMPerformanceMeter::init(unsigned long samples) 
{
  // samples is the number of samples that will be collected
  // before statistics for those samples are calculated.
  // A running mean statistics is also kept and not changed
  // by changing the number samples_.
  samples_ = samples;
  loopCounter_ = 0;
  latency_ = 0.0;
  throughput_= 0.0;
  rate_ = 0.0;
}
	
stor::SMPerformanceMeter::SMPerformanceMeter()
{
  latency_ = 0.0;
  throughput_= 0.0;
  rate_ = 0.0;

  totalMB4mean_= 0.0;
  meanThroughput_= 0.0;
  meanRate_= 0.0;
  meanLatency_= 0.0;
  sampleCounter_= 0;
  allTime_= 0.0;
}
			
bool stor::SMPerformanceMeter::addSample(unsigned long size) 
{	
  if ( loopCounter_ == 0 )
  {	
    chrono_.start(0);
    totalMB_ = 0;
  }
  else if ( loopCounter_ == (samples_ - 1  ) )
  {
    chrono_.stop(0);
    double usecs = (double) chrono_.dusecs();
    loopCounter_ = 0;

    throughput_ = totalMB_ / (usecs/ (double) 1000000.0);
    rate_ = ((double) samples_) / (usecs/ (double) 1000000.0);
    latency_ = ((double) 1000000.0)/rate_;
    // for mean measurements
    //double secs = (double) chrono_.dsecs();
    allTime_ += usecs/ (double) 1000000.0;
    meanThroughput_ = totalMB4mean_ / allTime_;
    meanRate_ = ((double) sampleCounter_) / allTime_;
    meanLatency_ = ((double) 1000000.0)/meanRate_;
    return true;			
  }	

  totalMB_ += ( (double) size / (double) 0x100000 );
  loopCounter_++;
  // for mean measurements
  totalMB4mean_ += ( (double) size / (double) 0x100000 );
  sampleCounter_++;

  return false;
}	

double stor::SMPerformanceMeter::bandwidth() 
{
  return throughput_;
}

double stor::SMPerformanceMeter::rate() 
{
  return rate_;
}

double stor::SMPerformanceMeter::latency() 
{
  return latency_;
}

double stor::SMPerformanceMeter::meanbandwidth() 
{
  return meanThroughput_;
}

double stor::SMPerformanceMeter::meanrate() 
{
  return meanRate_;
}

double stor::SMPerformanceMeter::meanlatency() 
{
  return meanLatency_;
}

unsigned long stor::SMPerformanceMeter::totalsamples() 
{
  return sampleCounter_;
}

double stor::SMPerformanceMeter::totalvolumemb() 
{
  return totalMB4mean_;
}

double stor::SMPerformanceMeter::duration() 
{
  return allTime_;
}

