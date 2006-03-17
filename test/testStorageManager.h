#ifndef _testStorageManager_h
#define _testStorageManager_h
/*
   Author: Harry Cheung, FNAL

   Description:
     Header file used by test Storage Manager XDAQ application that
     will receive I2O frames and write out a root file.

   Modification:
     version 1.1 2006/01/24
       Initial implementation. Needs changes for production version.

*/

#include <exception>
#include <iostream>
#include <string>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/unistd.h>

#include "FWCore/Framework/interface/EventProcessor.h"
#include "DataFormats/Common/interface/ProductRegistry.h"
#include "FWCore/Utilities/interface/ProblemTracker.h"
#include "FWCore/Utilities/interface/Exception.h"
#include "FWCore/MessageService/interface/MessageServicePresence.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "IOPool/Streamer/interface/HLTInfo.h"
#include "IOPool/Streamer/interface/Utilities.h"
#include "IOPool/Streamer/interface/TestFileReader.h"
#include "EventFilter/StorageManager/interface/JobController.h"
#include "PluginManager/PluginManager.h"

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

#include "EventFilter/Utilities/interface/EPStateMachine.h"

#include "boost/shared_ptr.hpp"

using namespace std;

namespace stor {

  struct SMFUSenderList;

  class testStorageManager: public xdaq::Application
  {
   public:
    //XDAQ_INSTANTIATOR();
    testStorageManager(xdaq::ApplicationStub* s) throw (xdaq::exception::Exception);
  
    ~testStorageManager();
  
   private:
    void configureAction(toolbox::Event::Reference e) throw (toolbox::fsm::exception::Exception);
    void enableAction(toolbox::Event::Reference e) throw (toolbox::fsm::exception::Exception);
    virtual void haltAction(toolbox::Event::Reference e) 
      throw (toolbox::fsm::exception::Exception);
    virtual void suspendAction(toolbox::Event::Reference e) 
      throw (toolbox::fsm::exception::Exception);
    virtual void resumeAction(toolbox::Event::Reference e) 
      throw (toolbox::fsm::exception::Exception);
    virtual void nullAction(toolbox::Event::Reference e) 
      throw (toolbox::fsm::exception::Exception);
    xoap::MessageReference fireEvent(xoap::MessageReference msg)
      throw (xoap::exception::Exception);
  
    void receiveRegistryMessage(toolbox::mem::Reference *ref);
    void receiveDataMessage(toolbox::mem::Reference *ref);
    void receiveOtherMessage(toolbox::mem::Reference *ref);

    void registerFUSender(const char* hltURL, const char* hltClassName,
                 const unsigned long hltLocalId, const unsigned long hltInstance,
                 const unsigned long hltTid,
                 const unsigned long registrySize, const char* registryData);
    void updateFUSender4data(const char* hltURL,
      const char* hltClassName, const unsigned long hltLocalId,
      const unsigned long hltInstance, const unsigned long hltTid,
      const unsigned long runNumber, const unsigned long eventNumber,
      const unsigned long frameNum, const unsigned long totalFrames,
      const unsigned long origdatasize, const bool isLocal);
  
    void defaultWebPage
      (xgi::Input *in, xgi::Output *out) throw (xgi::exception::Exception);
    void css(xgi::Input *in, xgi::Output *out) throw (xgi::exception::Exception)
      {css_.css(in,out);}
    void fusenderWebPage
      (xgi::Input *in, xgi::Output *out) throw (xgi::exception::Exception);
    void eventdataWebPage
      (xgi::Input *in, xgi::Output *out) throw (xgi::exception::Exception);
    void headerdataWebPage
      (xgi::Input *in, xgi::Output *out) throw (xgi::exception::Exception);
  
    evf::EPStateMachine *fsm_;
    //edm::AssertHandler *ah_;
    edm::service::MessageServicePresence theMessageServicePresence;
    xdata::String offConfig_;
    xdata::String fuConfig_;
    friend class evf::EPStateMachine;
  
    boost::shared_ptr<stor::JobController> jc_;
  
    evf::Css css_;
    unsigned long eventcounter_;
    unsigned long framecounter_;
    int pool_is_set_;
    toolbox::mem::Pool *pool_;

    vector<SMFUSenderList> smfusenders_;
  
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
  }; // end of class
} // end of namespace stor
#endif
