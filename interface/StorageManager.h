#ifndef _StorageManager_h
#define _StorageManager_h

/*
   Author: Harry Cheung, FNAL

   Description:
     Storage Manager XDAQ application. It can receive and collect
     I2O frames to remake event data. 

     See CMS EventFilter wiki page for further notes.

   $Id: StorageManager.h,v 1.45.6.58 2009/05/05 10:39:50 mommsen Exp $
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
#include "EventFilter/StorageManager/interface/DiskWriter.h"
#include "EventFilter/StorageManager/interface/DQMEventProcessor.h"
#include "EventFilter/StorageManager/interface/WebPageHelper.h"
#include "EventFilter/StorageManager/interface/WrapperNotifier.h"

#include "xdaq/Application.h"
#include "xdaq/ApplicationContext.h"

#include "xdata/UnsignedInteger32.h"

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

  class StorageManager: public xdaq::Application
  {
   public:
    StorageManager(xdaq::ApplicationStub* s) throw (xdaq::exception::Exception);
  
    ~StorageManager();

    // *** Updates the exported parameters
    xoap::MessageReference ParameterGet(xoap::MessageReference message)
    throw (xoap::exception::Exception);

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

////////////////////////////////////////////////////////////////////////////////
   private:  
    StorageManager(StorageManager const&); // not implemented
    StorageManager& operator=(StorageManager const&); // not implemented


    void receiveRegistryMessage(toolbox::mem::Reference *ref);
    void receiveDataMessage(toolbox::mem::Reference *ref);
    void receiveErrorDataMessage(toolbox::mem::Reference *ref);
    void receiveDQMMessage(toolbox::mem::Reference *ref);

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
    void rbsenderDetailWebPage
      (xgi::Input *in, xgi::Output *out) throw (xgi::exception::Exception);
    void fileStatisticsWebPage
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
    void dqmEventStatisticsWebPage
      (xgi::Input *in, xgi::Output *out) throw (xgi::exception::Exception);

    // New consumer handling methods. Will replace consumerWebPage,
    // headerdataWebPage, eventdataWebPage, DQMconsumerWebPage, and DQMeventdataWebPage
    void processConsumerRegistrationRequest( xgi::Input* in, xgi::Output* out )
      throw( xgi::exception::Exception );
    void processConsumerHeaderRequest( xgi::Input* in, xgi::Output* out )
      throw( xgi::exception::Exception );
    void processConsumerEventRequest( xgi::Input* in, xgi::Output* out )
      throw( xgi::exception::Exception );
    void processDQMConsumerRegistrationRequest( xgi::Input* in, xgi::Output* out )
      throw( xgi::exception::Exception );
    void processDQMConsumerEventRequest( xgi::Input* in, xgi::Output* out )
      throw( xgi::exception::Exception );

    // New consumer statistics web page. Will replace
    // eventServerWebPage.
    void consumerStatisticsPage( xgi::Input* in, xgi::Output* out )
      throw( xgi::exception::Exception );

    // *** state machine related
    std::string       reasonForFailedState_;

    // Get current state name:
    std::string externallyVisibleState() const;

    // instantiate the plugin manager, not referenced here after!
    edm::AssertHandler _ah;

    SharedResourcesPtr _sharedResources;

    evf::Css css_;

    // added for Event Server
    std::vector<unsigned char> mybuffer_; //temporary buffer instead of using stack
    boost::mutex consumerInitMsgLock_;

    SMFUSenderList smrbsenders_;

    FragmentProcessor *_fragmentProcessor;
    DiskWriter *_diskWriter;
    DQMEventProcessor *_dqmEventProcessor;

    // Notifier wrapper:
    WrapperNotifier _wrapper_notifier;

    WebPageHelper _webPageHelper;

  }; 
} 

#endif
/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
