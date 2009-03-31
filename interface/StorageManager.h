#ifndef _StorageManager_h
#define _StorageManager_h

/*
   Author: Harry Cheung, FNAL

   Description:
     Storage Manager XDAQ application. It can receive and collect
     I2O frames to remake event data. 

     See CMS EventFilter wiki page for further notes.

   $Id: StorageManager.h,v 1.45.6.38 2009/03/31 11:31:54 dshpakov Exp $
*/

#include <string>
#include <map>

#include "FWCore/MessageService/interface/MessageServicePresence.h"

#include "EventFilter/Utilities/interface/Exception.h"
#include "EventFilter/Utilities/interface/Css.h"

#include "EventFilter/StorageManager/interface/SMPerformanceMeter.h"
#include "EventFilter/StorageManager/interface/ForeverAverageCounter.h"
#include "EventFilter/StorageManager/interface/SMFUSenderList.h"
#include "EventFilter/StorageManager/interface/SharedResources.h"
#include "EventFilter/StorageManager/interface/FragmentProcessor.h"
#include "EventFilter/StorageManager/interface/WrapperNotifier.h"

#include "xdaq/Application.h"
#include "xdaq/ApplicationContext.h"

#include "xdata/String.h"
#include "xdata/UnsignedInteger32.h"
#include "xdata/Integer.h"
#include "xdata/Double.h"
#include "xdata/Boolean.h"
#include "xdata/Vector.h"

#include "xgi/exception/Exception.h"

#include "boost/shared_ptr.hpp"
#include "boost/thread/mutex.hpp"

namespace toolbox { 
  namespace mem {
    class Reference;
  }
}

namespace xgi {
  class Input;
  class Output;
}

namespace stor {

  class StorageManager: public xdaq::Application, 
                        public xdata::ActionListener
  {
   public:
    StorageManager(xdaq::ApplicationStub* s) throw (xdaq::exception::Exception);
  
    ~StorageManager();

    // *** Updates the exported parameters
    xoap::MessageReference ParameterGet(xoap::MessageReference message)
    throw (xoap::exception::Exception);

    // *** Anything to do with the flash list
    void setupFlashList();
    void actionPerformed(xdata::Event& e);

    // *** Callbacks to be executed during transitional states
    xoap::MessageReference configuring( xoap::MessageReference )
      throw( xoap::exception::Exception );
    xoap::MessageReference enabling( xoap::MessageReference )
      throw( xoap::exception::Exception );
    xoap::MessageReference stopping( xoap::MessageReference )
      throw( xoap::exception::Exception );
    xoap::MessageReference halting( xoap::MessageReference )
      throw( xoap::exception::Exception );

    // *** FSM soap command callback
    /*
    xoap::MessageReference fsmCallback(xoap::MessageReference msg)
      throw (xoap::exception::Exception);
    */
    // @@EM added monitoring workloop
    void startMonitoringWorkLoop() throw (evf::Exception);
    bool monitoring(toolbox::task::WorkLoop* wl);

    // tests of new Monitor classes
    void startNewMonitorWorkloop() throw (evf::Exception);
    bool newMonitorAction(toolbox::task::WorkLoop* wl);

////////////////////////////////////////////////////////////////////////////////
   private:  
    StorageManager(StorageManager const&); // not implemented
    StorageManager& operator=(StorageManager const&); // not implemented


    void receiveRegistryMessage(toolbox::mem::Reference *ref);
    void receiveDataMessage(toolbox::mem::Reference *ref);
    void receiveErrorDataMessage(toolbox::mem::Reference *ref);
    void receiveDQMMessage(toolbox::mem::Reference *ref);

    void sendDiscardMessage(unsigned int, 
			    unsigned int, 
			    unsigned int, 
			    std::string);

    void configureAction();
    void stopAction();
    void haltAction();

    unsigned int getRunNumber() const;

    void defaultWebPage
      (xgi::Input *in, xgi::Output *out) throw (xgi::exception::Exception);
    void css(xgi::Input *in, xgi::Output *out) throw (xgi::exception::Exception)
      {css_.css(in,out);}
    void storedDataWebPage
      (xgi::Input *in, xgi::Output *out) throw (xgi::exception::Exception);
    void rbsenderWebPage
      (xgi::Input *in, xgi::Output *out) throw (xgi::exception::Exception);
    void streamerOutputWebPage
      (xgi::Input *in, xgi::Output *out) throw (xgi::exception::Exception);
    void eventdataWebPage
      (xgi::Input *in, xgi::Output *out) throw (xgi::exception::Exception);
    void headerdataWebPage
      (xgi::Input *in, xgi::Output *out) throw (xgi::exception::Exception);
    void consumerWebPage
      (xgi::Input *in, xgi::Output *out) throw (xgi::exception::Exception);
    void consumerListWebPage
      (xgi::Input *in, xgi::Output *out) throw (xgi::exception::Exception);
    void eventServerWebPage
      (xgi::Input *in, xgi::Output *out) throw (xgi::exception::Exception);
    void DQMeventdataWebPage
      (xgi::Input *in, xgi::Output *out) throw (xgi::exception::Exception);
    void DQMconsumerWebPage
      (xgi::Input *in, xgi::Output *out) throw (xgi::exception::Exception);

    std::string findStreamName(const std::string &in) const;
	
    // *** state machine related
    std::string       reasonForFailedState_;

    // Get current state name:
    std::string externallyVisibleState() const;

    // State name for infospace updates:
    xdata::String _xdaq_state_name;

    edm::AssertHandler *ah_;
    edm::service::MessageServicePresence theMessageServicePresence;
  
    boost::mutex                           halt_lock_;

    SharedResourcesPtr sharedResourcesPtr_;

    evf::Css css_;
    int pool_is_set_;
    toolbox::mem::Pool *pool_;

    // added for Event Server
    std::vector<unsigned char> mybuffer_; //temporary buffer instead of using stack
    boost::mutex consumerInitMsgLock_;

    SMFUSenderList smrbsenders_;
    xdata::UnsignedInteger32 connectedRBs_;

    xdata::UnsignedInteger32 storedEvents_;
    xdata::UnsignedInteger32 closedFiles_;
    xdata::UnsignedInteger32 openFiles_;
    xdata::Vector<xdata::String> fileList_;
    xdata::Vector<xdata::UnsignedInteger32> eventsInFile_;
    xdata::Vector<xdata::UnsignedInteger32> storedEventsInStream_;
    xdata::Vector<xdata::UnsignedInteger32> receivedEventsFromOutMod_;
    typedef std::map<std::string,uint32> countMap;
    typedef std::map<std::string, boost::shared_ptr<ForeverAverageCounter> > valueMap;
    typedef std::map<uint32,std::string> idMap;
    typedef std::map<uint32,std::string>::iterator idMap_iter;
    countMap receivedEventsMap_;
    valueMap avEventSizeMap_;
    valueMap avCompressRatioMap_;
    idMap modId2ModOutMap_;
    countMap storedEventsMap_;
    xdata::Vector<xdata::UnsignedInteger32> fileSize_;
    xdata::Vector<xdata::String> namesOfStream_;
    xdata::Vector<xdata::String> namesOfOutMod_;

    // *** for stored data performance measurements
    // *** measurements for last set of samples
    xdata::UnsignedInteger32 store_samples_; // number of samples/frames per measurement
    xdata::UnsignedInteger32 store_period4samples_; // time period per measurement
    xdata::Double store_instantBandwidth_; // bandwidth in MB/s
    xdata::Double store_instantRate_;      // number of frames/s
    xdata::Double store_instantLatency_;   // micro-seconds/frame
    xdata::Double store_maxBandwidth_;     // maximum bandwidth in MB/s
    xdata::Double store_minBandwidth_;     // minimum bandwidth in MB/s
    // *** measurements for last time period
    xdata::Double store_instantBandwidth2_;// bandwidth in MB/s
    xdata::Double store_instantRate2_;     // number of frames/s
    xdata::Double store_instantLatency2_;  // micro-seconds/frame
    xdata::Double store_maxBandwidth2_;    // maximum bandwidth in MB/s
    xdata::Double store_minBandwidth2_;    // minimum bandwidth in MB/s

    // *** measurements for all samples
    xdata::Double store_duration_;         // time for run in seconds
    xdata::UnsignedInteger32 store_totalSamples_; //number of samples/frames per measurement
    xdata::Double store_meanBandwidth_;    // bandwidth in MB/s
    xdata::Double store_meanRate_;         // number of frames/s
    xdata::Double store_meanLatency_;      // micro-seconds/frame
    xdata::Double store_receivedVolume_;   // total received data in MB

    xdata::Double store_duration2_;         // time for run in seconds
    xdata::UnsignedInteger32 store_totalSamples2_; //number of samples/frames per measurement
    xdata::Double store_meanBandwidth2_;    // bandwidth in MB/s
    xdata::Double store_meanRate2_;         // number of frames/s
    xdata::Double store_meanLatency2_;      // micro-seconds/frame

    // *** additional flashlist contents (rest was already there)
    xdata::String            class_;
    xdata::UnsignedInteger32 instance_;
    xdata::String            url_;       

    xdata::Double            storedVolume_;
    xdata::UnsignedInteger32 memoryUsed_;
    xdata::String            progressMarker_;

    // @@EM workloop / action signature for monitoring
    toolbox::task::WorkLoop         *wlMonitoring_;      
    toolbox::task::ActionSignature  *asMonitoring_;

    FragmentProcessor *fragmentProcessor_;

    // @@EM parameters monitored by workloop (not in flashlist just yet) 
    struct streammon{
      int		nclosedfiles_;
      int		nevents_;
      int		totSizeInkBytes_;
    };

    typedef std::map<std::string,streammon> smap;
    typedef std::map<std::string,streammon>::iterator ismap;
    smap	 streams_;

    boost::mutex rblist_lock_;  // quick (temporary) fix for registration problem

    // Notifier and wrapper:
    xdaq2rc::RcmsStateNotifier _rcms_notifier;
    WrapperNotifier _wrapper_notifier;

    std::string sm_cvs_version_;

    enum
    {
      DEFAULT_PURGE_TIME = 120,
      DEFAULT_READY_TIME = 30
    };

  }; 
} 

#endif
/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
