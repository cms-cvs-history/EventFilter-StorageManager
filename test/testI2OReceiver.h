#ifndef _testI2OReceiver_h_
#define _testI2OReceiver_h_

/*
   Author: Harry Cheung, FNAL

   Description:
     Header file used by simple XDAQ application that will receive I2O
     frames from FU StreamerI2OConsumer output module and write out a data file.
     This is the bare bones Storage Manager application that uses the
     new message classes.

   Modification:
     version 1.1 2006/07/29
       Initial implementation.

*/

#include <exception>
#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/unistd.h>
#include <fstream>

#include "xdaq/Application.h"
#include "xdaq/ApplicationContext.h"

#include "toolbox/mem/Reference.h"
#include "xdata/UnsignedLong.h"
#include "xdata/Integer.h"
#include "xdata/Double.h"
#include "EventFilter/StorageManager/interface/SMPerformanceMeter.h"
#include "EventFilter/StorageManager/interface/EvtMsgRingBuffer.h"

#include "xgi/include/xgi/Input.h"
#include "xgi/include/xgi/Output.h"
#include "xgi/include/xgi/exception/Exception.h"
#include "EventFilter/Utilities/interface/Css.h"

#include "EventFilter/Utilities/interface/RunBase.h"
#include "EventFilter/StorageManager/interface/SMStateMachine.h"

#include "boost/shared_ptr.hpp"
#include "boost/thread/thread.hpp"

namespace stor {

  struct SMI2ORecFrames;
  struct SMFUSenderList;

  class testI2OReceiver: public xdaq::Application, public evf::RunBase
  {
   public:
	
      testI2OReceiver(xdaq::ApplicationStub * s) throw (xdaq::exception::Exception);

      ~testI2OReceiver();

   /**
     * Updates the exported parameters
     */
    xoap::MessageReference ParameterGet(xoap::MessageReference message)
    throw (xoap::exception::Exception);

   private:
    void configureAction(toolbox::Event::Reference e) throw (toolbox::fsm::exception::Exception);
    void enableAction(toolbox::Event::Reference e) throw (toolbox::fsm::exception::Exception);
    virtual void haltAction(toolbox::Event::Reference e)
      throw (toolbox::fsm::exception::Exception);
    virtual void nullAction(toolbox::Event::Reference e)
      throw (toolbox::fsm::exception::Exception);
    xoap::MessageReference fireEvent(xoap::MessageReference msg)
      throw (xoap::exception::Exception);

    void receiveRegistryMessage(toolbox::mem::Reference *ref);
    void receiveDataMessage(toolbox::mem::Reference *ref);
    void receiveOtherMessage(toolbox::mem::Reference *ref);

    void testCompleteChain(vector<SMI2ORecFrames>::iterator pos);
    void writeCompleteChain(toolbox::mem::Reference *ref);

    void registerFUSender(const char* hltURL, const char* hltClassName,
                 const unsigned long hltLocalId, const unsigned long hltInstance,
                 const unsigned long hltTid,
                 const unsigned long frameCount, const unsigned long numFrames,
                 const unsigned long registrySize, const char* registryData,
                 toolbox::mem::Reference *ref);
    void testCompleteFUReg(list<SMFUSenderList>::iterator pos);
    void copyAndTestRegistry(list<SMFUSenderList>::iterator pos,
                 toolbox::mem::Reference *head);
    void updateFUSender4data(const char* hltURL,
      const char* hltClassName, const unsigned long hltLocalId,
      const unsigned long hltInstance, const unsigned long hltTid,
      const unsigned long runNumber, const unsigned long eventNumber,
      const unsigned long frameNum, const unsigned long totalFrames,
      const unsigned long origdatasize, const bool isLocal);
    void removeFUSender(const char* hltURL,
      const char* hltClassName, const unsigned long hltLocalId,
      const unsigned long hltInstance, const unsigned long hltTid);

    void defaultWebPage
      (xgi::Input *in, xgi::Output *out) throw (xgi::exception::Exception);
    void css(xgi::Input *in, xgi::Output *out) throw (xgi::exception::Exception)
      {css_.css(in,out);}
    //void fusenderWebPage
    //  (xgi::Input *in, xgi::Output *out) throw (xgi::exception::Exception);
    void eventdataWebPage
      (xgi::Input *in, xgi::Output *out) throw (xgi::exception::Exception);
    void headerdataWebPage
      (xgi::Input *in, xgi::Output *out) throw (xgi::exception::Exception);

    stor::SMStateMachine *fsm_;
    xdata::String offConfig_;
    xdata::String stfileName_;
    friend class stor::SMStateMachine;

    vector<SMI2ORecFrames> SMframeFragments_;
    string filename_;
    ofstream ost_;

    evf::Css css_;
    unsigned long eventcounter_;
    unsigned long framecounter_;
    int pool_is_set_;
    toolbox::mem::Pool *pool_; 

    // added for Event Server
    char serialized_prods_[1000*1000*2];
    int  ser_prods_size_;
    xdata::Integer oneinN_; //place one in eveny oneinN_ into buffer
    char mybuffer_[7000000]; //temporary buffer instead of using stack
    // 
    stor::EvtMsgRingBuffer evtsrv_area_;
    int count_4_oneinN_;
    int hltBitCount;
    int l1BitCount;

    list<SMFUSenderList> smfusenders_;
    xdata::UnsignedLong connectedFUs_;
    xdata::UnsignedLong storedEvents_;

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
    boost::mutex halt_lock_;
  }; // end of class
}; // end of namespace stor
#endif
