// $Id: FragmentCollector.cc,v 1.43.4.9 2009/03/04 15:21:08 biery Exp $

#include "EventFilter/StorageManager/interface/FragmentCollector.h"
#include "EventFilter/StorageManager/interface/ProgressMarker.h"
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


  FragmentCollector::FragmentCollector(HLTInfo& h,Deleter d,
				       log4cplus::Logger& applicationLogger,
                                       SharedResources sharedResources,
                                       boost::shared_ptr<DiscardManager> discardMgr,
                                       const string& config_str):
    cmd_q_(&(h.getCommandQueue())),
    frag_q_(&(h.getFragmentQueue())),
    buffer_deleter_(d),
    prods_(0),
    info_(&h), 
    lastStaleCheckTime_(time(0)),
    staleFragmentTimeout_(30),
    disks_(0),
    applicationLogger_(applicationLogger),
    newFragmentQueue_(sharedResources._fragmentQueue),
    discardManager_(discardMgr),
    writer_(new edm::ServiceManager(config_str)),
    dqmServiceManager_(new stor::DQMServiceManager())
  {
    // supposed to have given parameterSet smConfigString to writer_
    // at ctor
    event_area_.reserve(7000000);
  }
  FragmentCollector::FragmentCollector(std::auto_ptr<HLTInfo> info,Deleter d,
				       log4cplus::Logger& applicationLogger,
                                       SharedResources sharedResources,
                                       boost::shared_ptr<DiscardManager> discardMgr,
                                       const string& config_str):
    cmd_q_(&(info.get()->getCommandQueue())),
    frag_q_(&(info.get()->getFragmentQueue())),
    buffer_deleter_(d),
    prods_(0),
    info_(info.get()), 
    lastStaleCheckTime_(time(0)),
    staleFragmentTimeout_(30),
    disks_(0),
    applicationLogger_(applicationLogger),
    newFragmentQueue_(sharedResources._fragmentQueue),
    discardManager_(discardMgr),
    writer_(new edm::ServiceManager(config_str)),
    dqmServiceManager_(new stor::DQMServiceManager())
  {
    // supposed to have given parameterSet smConfigString to writer_
    // at ctor
    event_area_.reserve(7000000);
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
              case Header::FILE_CLOSE_REQUEST:
                {
                  FR_DEBUG << "FragColl: Got a File Close Request message" << endl;
                  writer_->closeFilesIfNeeded();
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

        if (newFragmentQueue_->deq_nowait(i2oChain))
          {
            nothingHappening = false;

            switch(i2oChain.messageCode())
              {
              case Header::INIT:
                {
                  FR_DEBUG << "FragColl: Got an Init" << endl;
                  processHeader(i2oChain);
                  break;
                }
              case Header::EVENT:
                {
                  FR_DEBUG << "FragColl: Got an Event" << endl;
                  processEvent(i2oChain);
                  break;
                }
              case Header::DQM_EVENT:
                {
                  FR_DEBUG << "FragColl: Got a DQM_Event" << endl;
                  processDQMEvent(i2oChain);
                  break;
                }
              case Header::ERROR_EVENT:
                {
                  FR_DEBUG << "FragColl: Got an Error_Event" << endl;
                  processErrorEvent(i2oChain);
                  break;
                }
              default:
                {
                  char codeString[32];
                  sprintf(codeString, "%d", i2oChain.messageCode());
                  std::string logMsg = "Invalid message code (";
                  logMsg.append(codeString);
                  logMsg.append(") received on new fragment queue!");
                  LOG4CPLUS_ERROR(applicationLogger_, logMsg);
                  break;
                }
              }
          }

        if (nothingHappening) { ::usleep(100); }
      }
    
    FR_DEBUG << "FragColl: DONE!" << endl;
    writer_->stop();
    dqmServiceManager_->stop();
  }

  void FragmentCollector::stop()
  {
    // called from a different thread - trigger completion to the
    // fragment collector, which will cause a completion of the 
    // event processor

    edm::EventBuffer::ProducerBuffer cb(*frag_q_);
    cb.commit();
  }

  void FragmentCollector::processEvent(I2OChain i2oChain)
  {
    ProgressMarker::instance()->processing(true);

    // add the fragment to the fragment store
    bool complete = fragmentStore_.addFragment(i2oChain);

    if(complete)
    {
      int assembledSize = i2oChain.copyFragmentsIntoBuffer(event_area_);

      EventMsgView emsg(&event_area_[0]);
      FR_DEBUG << "FragColl: writing event size " << assembledSize << endl;
      writer_->manageEventMsg(emsg);

      if (eventServer_.get() != NULL)
        {
          eventServer_->processEvent(emsg);
        }

      // tell the resource broker that sent us this event
      // that we are done with it and it can forget about it
      discardManager_->sendDiscardMessage(i2oChain);

      // check for stale fragments
      removeStaleFragments();
    }

    ProgressMarker::instance()->processing(false);
  }

  void FragmentCollector::processHeader(I2OChain i2oChain)
  {
    ProgressMarker::instance()->processing(true);

    // add the fragment to the fragment store
    bool complete = fragmentStore_.addFragment(i2oChain);

    if(complete)
    {
      int assembledSize = i2oChain.copyFragmentsIntoBuffer(event_area_);

      InitMsgView msg(&event_area_[0]);
      FR_DEBUG << "FragColl: writing INIT size " << assembledSize << endl;
      writer_->manageInitMsg(catalog_, disks_, sourceId_, msg, *initMsgCollection_);

      try
      {
        if (initMsgCollection_->addIfUnique(msg))
        {
          // check if any currently connected consumers did not specify
          // an HLT output module label and we now have multiple, different,
          // INIT messages.  If so, we need to complain because the
          // SelectHLTOutput parameter needs to be specified when there
          // is more than one HLT output module (and correspondingly, more
          // than one INIT message)
          if (initMsgCollection_->size() > 1)
          {
            std::map< uint32, boost::shared_ptr<ConsumerPipe> > consumerTable = 
              eventServer_->getConsumerTable();
            std::map< uint32, boost::shared_ptr<ConsumerPipe> >::const_iterator 
              consumerIter;
            for (consumerIter = consumerTable.begin();
                 consumerIter != consumerTable.end();
                 ++consumerIter)
            {
              boost::shared_ptr<ConsumerPipe> consPtr = consumerIter->second;

              // for regular consumers, we need to test whether the consumer
              // configuration specified an HLT output module
              if (! consPtr->isProxyServer())
              {
                if (consPtr->getHLTOutputSelection().empty())
                {
                  // store a warning message in the consumer pipe to be
                  // sent to the consumer at the next opportunity
                  std::string errorString;
                  errorString.append("ERROR: The configuration for this ");
                  errorString.append("consumer does not specify an HLT output ");
                  errorString.append("module.\nPlease specify one of the HLT ");
                  errorString.append("output modules listed below as the ");
                  errorString.append("SelectHLTOutput parameter ");
                  errorString.append("in the InputSource configuration.\n");
                  errorString.append(initMsgCollection_->getSelectionHelpString());
                  errorString.append("\n");
                  consPtr->setRegistryWarning(errorString);
                }
              }
            }
          }
        }
      }
      catch(cms::Exception& excpt)
      {
        char tidString[32];
        sprintf(tidString, "%d", i2oChain.hltTid());
        std::string logMsg = "receiveRegistryMessage: Error processing a ";
        logMsg.append("registry message from URL ");
        logMsg.append(i2oChain.hltURL());
        logMsg.append(" and Tid ");
        logMsg.append(tidString);
        logMsg.append(":\n");
        logMsg.append(excpt.what());
        logMsg.append("\n");
        logMsg.append(initMsgCollection_->getSelectionHelpString());
        FDEBUG(9) << logMsg << std::endl;
        LOG4CPLUS_ERROR(applicationLogger_, logMsg);

        throw excpt;
      }

      FragKey fragKey = i2oChain.fragmentKey();
      smRBSenderList_->registerDataSender(i2oChain.hltURL().c_str(), i2oChain.hltClassName().c_str(),
                                          i2oChain.hltLocalId(), i2oChain.hltInstance(), i2oChain.hltTid(),
                                          i2oChain.fragmentCount()-1, i2oChain.fragmentCount(), msg.size(),
                                          msg.outputModuleLabel(), msg.outputModuleId(),
                                          fragKey.originatorPid_);

      // tell the resource broker that sent us this event
      // that we are done with it and it can forget about it
      discardManager_->sendDiscardMessage(i2oChain);

      // check for stale fragments
      removeStaleFragments();
    }
    ProgressMarker::instance()->processing(false);
  }

  void FragmentCollector::processDQMEvent(I2OChain i2oChain)
  {
    ProgressMarker::instance()->processing(true);

    // add the fragment to the fragment store
    bool complete = fragmentStore_.addFragment(i2oChain);

    if(complete)
    {
      i2oChain.copyFragmentsIntoBuffer(event_area_);

      // the DQM data is now in event_area_ deal with it
      DQMEventMsgView dqmEventView(&event_area_[0]);
      dqmServiceManager_->manageDQMEventMsg(dqmEventView);

      // tell the resource broker that sent us this event
      // that we are done with it and it can forget about it
      discardManager_->sendDiscardMessage(i2oChain);

      // check for stale fragments
      removeStaleFragments();
    }

    ProgressMarker::instance()->processing(false);
  }

  void FragmentCollector::processErrorEvent(I2OChain i2oChain)
  {
    ProgressMarker::instance()->processing(true);

    // add the fragment to the fragment store
    bool complete = fragmentStore_.addFragment(i2oChain);

    if(complete)
    {
      int assembledSize = i2oChain.copyFragmentsIntoBuffer(event_area_);

      FRDEventMsgView emsg(&event_area_[0]);
      FR_DEBUG << "FragColl: writing error event size " << assembledSize << endl;
      writer_->manageErrorEventMsg(catalog_, disks_, sourceId_, emsg);

      // tell the resource broker that sent us this event
      // that we are done with it and it can forget about it
      discardManager_->sendDiscardMessage(i2oChain);

      // check for stale fragments
      removeStaleFragments();
    }

    ProgressMarker::instance()->processing(false);
  }

  /**
   * This method removes stale fragments from the fragmentStore.
   *
   * @return the number of events (fragmentContainers, actually) that
   *         were removed from the fragment_area_.
   */
  int FragmentCollector::removeStaleFragments()
  {
    I2OChain staleEvent;
    bool gotStaleEvent = true;  
    int loopCounter = 0;
    int discardCount = 0;

    while ( gotStaleEvent && loopCounter++ < 10 )
      {
        gotStaleEvent = fragmentStore_.getStaleEvent(staleEvent, staleFragmentTimeout_);
        if ( gotStaleEvent )
          {
            LOG4CPLUS_WARN(applicationLogger_, "Deleting a stale I2OChain.");
            ++discardCount;
          }
      }

    return discardCount;
  }
}
