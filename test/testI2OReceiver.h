#ifndef _testI2OReceiver_h_
#define _testI2OReceiver_h_

/*
   Author: Harry Cheung, FNAL

   Description:
     Header file used by test XDAQ application that will receive I2O
     frames and write out a data file.

   Modification:
     version 1.1 2005/11/23
       Initial implementation. Needs changes for production version.
     version 1.2 2005/12/15
       Added a default page to give statistics. The statistics for
         the memory pool maximum size is not filled.

*/


#include "xdaq/Application.h"
#include "xdaq/ApplicationContext.h"

#include "toolbox/mem/Reference.h"
#include "xdata/UnsignedLong.h"
#include "xdata/Double.h"
#include "EventFilter/StorageManager/interface/SMPerformanceMeter.h"


#include "xgi/include/xgi/Input.h"
#include "xgi/include/xgi/Output.h"
#include "xgi/include/xgi/exception/Exception.h"
#include "EventFilter/Utilities/interface/Css.h"

#include <vector>
#include <string>
#include <fstream>

struct SMI2ORecFrames;

class testI2OReceiver: public xdaq::Application  
{
  public:
	
  testI2OReceiver(xdaq::ApplicationStub * s) throw (xdaq::exception::Exception);

  virtual ~testI2OReceiver(){ delete pmeter_;}

 void defaultWebPage
    (xgi::Input *in, xgi::Output *out) throw (xgi::exception::Exception);

  private:

  void receiveRegistryMessage(toolbox::mem::Reference *ref);
  void receiveDataMessage(toolbox::mem::Reference *ref);
  void receiveOtherMessage(toolbox::mem::Reference *ref);
  void testCompleteChain(vector<SMI2ORecFrames>::iterator pos);
  void writeCompleteChain(toolbox::mem::Reference *ref);

  void css(xgi::Input *in, xgi::Output *out) throw (xgi::exception::Exception)
    {css_.css(in,out);}

  vector<SMI2ORecFrames> SMframeFragments_;
  string filename_;
  ofstream ost_;
  evf::Css css_;
  unsigned long eventcounter_;
  unsigned long framecounter_;
  int pool_is_set_;
  toolbox::mem::Pool *pool_; 

  // for performance measurements
  void addMeasurement(unsigned long size);
  xdata::UnsignedLong samples_; //number of samples (frames) per measurement
  stor::SMPerformanceMeter *pmeter_;
  // measurements for last set of samples
  xdata::Double databw_;      // bandwidth in MB/s
  xdata::Double datarate_;    // number of frames/s
  xdata::Double datalatency_; // micro-seconds/frame
  xdata::UnsignedLong totalsamples_; //number of samples (frames) per measurement
  xdata::Double duration_;        // time for run in seconds
  xdata::Double meandatabw_;      // bandwidth in MB/s
  xdata::Double meandatarate_;    // number of frames/s
  xdata::Double meandatalatency_; // micro-seconds/frame
  xdata::Double maxdatabw_;       // maximum bandwidth in MB/s
  xdata::Double mindatabw_;       // minimum bandwidth in MB/s

};
#endif
