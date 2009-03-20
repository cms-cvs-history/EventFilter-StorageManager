// $Id: FragmentCollector.cc,v 1.43.4.22 2009/03/20 17:27:32 biery Exp $

#include "EventFilter/StorageManager/interface/FragmentCollector.h"
#include "EventFilter/StorageManager/interface/I2OChain.h"

#include "IOPool/Streamer/interface/MsgHeader.h"
#include "IOPool/Streamer/interface/InitMessage.h"
#include "IOPool/Streamer/interface/DQMEventMessage.h"
#include "IOPool/Streamer/interface/FRDEventMessage.h"

#include "boost/bind.hpp"

#include "log4cplus/loggingmacros.h"

#include <algorithm>
#include <utility>
#include <cstdlib>
#include <fstream>

using namespace edm;
using namespace std;

static const bool debugme = getenv("FRAG_DEBUG")!=0;  
#define FR_DEBUG if(debugme) std::cerr


namespace stor
{

// TODO fixme: this quick fix to give thread status should be reviewed!
  struct SMFC_thread_data
  {
    SMFC_thread_data() {
      exception_in_thread = false;
      reason_for_exception = "";
    }

    volatile bool exception_in_thread;
    std::string reason_for_exception;
  };

  static SMFC_thread_data SMFragCollThread;

  bool getSMFC_exceptionStatus() { return SMFragCollThread.exception_in_thread; }
  std::string getSMFC_reason4Exception() { return SMFragCollThread.reason_for_exception; }


  FragmentCollector::FragmentCollector(HLTInfo& h,
				       log4cplus::Logger& applicationLogger,
                                       SharedResourcesPtr sharedResources):
    cmd_q_(&(h.getCommandQueue())),
    frag_q_(&(h.getFragmentQueue())),
    prods_(0),
    info_(&h), 
    lastStaleCheckTime_(time(0)),
    staleFragmentTimeout_(30),
    applicationLogger_(applicationLogger),
    writer_(sharedResources->_serviceManager),
    eventServer_(sharedResources->_oldEventServer),
    DQMeventServer_(sharedResources->_oldDQMEventServer),
    initMsgCollection_(sharedResources->_initMsgCollection)
  {
    // supposed to have given parameterSet smConfigString to writer_
    // at ctor
    event_area_.reserve(7000000);

    if (eventServer_.get() != NULL) {
      eventServer_->setStreamSelectionTable(writer_->getStreamSelectionTable());
    }
  }
  FragmentCollector::FragmentCollector(std::auto_ptr<HLTInfo> info,
				       log4cplus::Logger& applicationLogger,
                                       SharedResourcesPtr sharedResources):
    cmd_q_(&(info.get()->getCommandQueue())),
    frag_q_(&(info.get()->getFragmentQueue())),
    prods_(0),
    info_(info.get()), 
    lastStaleCheckTime_(time(0)),
    staleFragmentTimeout_(30),
    applicationLogger_(applicationLogger),
    writer_(sharedResources->_serviceManager),
    eventServer_(sharedResources->_oldEventServer),
    DQMeventServer_(sharedResources->_oldDQMEventServer),
    initMsgCollection_(sharedResources->_initMsgCollection)
  {
    // supposed to have given parameterSet smConfigString to writer_
    // at ctor
    event_area_.reserve(7000000);

    if (eventServer_.get() != NULL) {
      eventServer_->setStreamSelectionTable(writer_->getStreamSelectionTable());
    }
  }

  FragmentCollector::~FragmentCollector()
  {
  }

  void FragmentCollector::run(FragmentCollector* t)
  {
    try {
      t->processFragments();
    }
    catch(cms::Exception& e)
    {
       edm::LogError("StorageManager") << "Fatal error in FragmentCollector thread: " << "\n"
                 << e.explainSelf() << std::endl;
       SMFragCollThread.exception_in_thread = true;
       SMFragCollThread.reason_for_exception = "Fatal error in FragmentCollector thread: \n" +
          e.explainSelf();
    }
    // just let the thread end here there is no cleanup, no recovery possible
  }

  void FragmentCollector::start()
  {
    // 14-Oct-2008, KAB - avoid race condition by starting writers first
    // (otherwise INIT message could be received and processed before
    // the writers are started (and whatever initialization is done in the
    // writers when INIT messages are processed could be wiped out by 
    // the start command)
    writer_->start();
    me_.reset(new boost::thread(boost::bind(FragmentCollector::run,this)));
  }

  void FragmentCollector::join()
  {
    me_->join();
  }

  void FragmentCollector::processFragments()
  {
    // everything comes in on the fragment queue, even
    // command-like messages.  we need to dispatch things
    // we recognize - either execute the command, forward it
    // to the command queue, or process it for output to the 
    // event queue.
    bool done=false;
    I2OChain i2oChain;

    while(!done)
      {
        bool nothingHappening = true;

        if(!(*frag_q_).empty())
          {
            nothingHappening = false;

            EventBuffer::ConsumerBuffer cb(*frag_q_);
            if(cb.size()==0) break;
            FragEntry* entry = (FragEntry*)cb.buffer();
            FR_DEBUG << "FragColl: " << (void*)this << " Got a buffer size="
                     << entry->buffer_size_ << endl;
            switch(entry->code_)
              {
              case Header::DONE:
                {
                  // make sure that this is actually sent by the controller! (JBK)
                  // this does nothing currently
                  FR_DEBUG << "FragColl: Got a Done" << endl;
                  done=true;
                  break;
                }
              default:
                {
                  char codeString[32];
                  sprintf(codeString, "%d", entry->code_);
                  std::string logMsg = "Invalid message code (";
                  logMsg.append(codeString);
                  logMsg.append(") received on old fragment queue!");
                  LOG4CPLUS_ERROR(applicationLogger_, logMsg);

                  FR_DEBUG << "FragColl: Got junk" << endl;
                  break; // lets ignore other things for now
                }
              }
          }

        if (nothingHappening) { ::usleep(100); }
      }
    
    FR_DEBUG << "FragColl: DONE!" << endl;
    writer_->stop();
  }

  void FragmentCollector::stop()
  {
    // called from a different thread - trigger completion to the
    // fragment collector, which will cause a completion of the 
    // event processor

    edm::EventBuffer::ProducerBuffer cb(*frag_q_);
    cb.commit();
  }
}
