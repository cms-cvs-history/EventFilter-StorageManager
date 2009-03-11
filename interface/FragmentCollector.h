#ifndef HLT_FRAG_COLL_HPP
#define HLT_FRAG_COLL_HPP

/*
  All buffers passed in on queues are owned by the fragment collector.

  JBK - I think the frag_q is going to need to be a pair of pointers.
  The first is the address of the object that needs to be deleted 
  using the Deleter function.  The second is the address of the buffer
  of information that we manipulate (payload of the received object).

  The code should allow for deleters to be functors or simple functions.
 */

#include "IOPool/Streamer/interface/HLTInfo.h"
#include "IOPool/Streamer/interface/EventBuffer.h"
#include "IOPool/Streamer/interface/Utilities.h"
#include "IOPool/Streamer/interface/MsgTools.h"
#include "IOPool/Streamer/interface/EventMessage.h"

#include "DataFormats/Provenance/interface/ProductRegistry.h"

#include "EventFilter/StorageManager/interface/EvtMsgRingBuffer.h"
#include "EventFilter/StorageManager/interface/ServiceManager.h"
#include "EventFilter/StorageManager/interface/DQMServiceManager.h"
#include "EventFilter/StorageManager/interface/SMPerformanceMeter.h"
#include "EventFilter/StorageManager/interface/SMFUSenderList.h"
#include "EventFilter/StorageManager/interface/FragmentStore.h"
#include "EventFilter/StorageManager/interface/SharedResources.h"
#include "EventFilter/StorageManager/interface/DiscardManager.h"

#include "boost/shared_ptr.hpp"
#include "boost/thread/thread.hpp"

#include "log4cplus/logger.h"

#include <vector>
#include <map>
#include <string>

namespace stor
{
  struct FragmentContainer
  {
    FragmentContainer():creationTime_(time(0)),lastFragmentTime_(0) { }
    std::map<int, FragEntry> fragmentMap_;
    time_t creationTime_;
    time_t lastFragmentTime_;
  };

  class FragmentCollector
  {
  public:
    typedef std::vector<unsigned char> Buffer;

    // This is not the most efficient way to store and manipulate this
    // type of data.  It is like this because there is not much time
    // available to create the prototype.
    typedef std::map<stor::FragKey, FragmentContainer> Collection;

    FragmentCollector(HLTInfo& h,
		      log4cplus::Logger& applicationLogger,
                      SharedResources sharedResources,
                      const std::string& config_str="");
    FragmentCollector(std::auto_ptr<HLTInfo>,
		      log4cplus::Logger& applicationLogger,
                      SharedResources sharedResources,
                      const std::string& config_str="");
    ~FragmentCollector();

    void start();
    void join();
    void stop();

    edm::EventBuffer& getFragmentQueue() { return *frag_q_; }
    edm::EventBuffer& getCommandQueue() { return *cmd_q_; }
    
    void setSMRBSenderList(SMFUSenderList* senderList) { smRBSenderList_ = senderList; }

  private:
    static void run(FragmentCollector*);
    void processFragments();
    void processEvent(I2OChain i2oChain);
    void processHeader(I2OChain i2oChain);
    void processDQMEvent(I2OChain i2oChain);
    void processErrorEvent(I2OChain i2oChain);

    int removeStaleFragments();

    edm::EventBuffer* cmd_q_;
    edm::EventBuffer* frag_q_;

    Buffer event_area_;
    Collection fragment_area_;
    boost::shared_ptr<boost::thread> me_;
    const edm::ProductRegistry* prods_; // change to shared_ptr ? 
    stor::HLTInfo* info_;  // cannot be const when using EP_Runner?

    time_t lastStaleCheckTime_;
    int staleFragmentTimeout_;

  public:

    void setNumberOfFileSystems(int disks)   { disks_        = disks; }
    void setFileCatalog(std::string catalog) { catalog_      = catalog; }
    void setSourceId(std::string sourceId)   { sourceId_     = sourceId; }

    void setCollateDQM(bool collateDQM)
    { dqmServiceManager_->setCollateDQM(collateDQM); }

    void setArchiveDQM(bool archiveDQM)
    { dqmServiceManager_->setArchiveDQM(archiveDQM); }

    void setArchiveIntervalDQM(int archiveInterval)
    { dqmServiceManager_->setArchiveInterval(archiveInterval); }

    void setPurgeTimeDQM(int purgeTimeDQM)
    { dqmServiceManager_->setPurgeTime(purgeTimeDQM);}

    void setReadyTimeDQM(int readyTimeDQM)
    { dqmServiceManager_->setReadyTime(readyTimeDQM);}

    void setFilePrefixDQM(std::string filePrefixDQM)
    { dqmServiceManager_->setFilePrefix(filePrefixDQM);}

    void setUseCompressionDQM(bool useCompressionDQM)
    { dqmServiceManager_->setUseCompression(useCompressionDQM);}

    void setCompressionLevelDQM(int compressionLevelDQM)
    { dqmServiceManager_->setCompressionLevel(compressionLevelDQM);}

    std::list<std::string>& get_filelist() { return writer_->get_filelist();  }
    std::list<std::string>& get_currfiles() { return writer_->get_currfiles(); }
    std::vector<uint32>& get_storedEvents() { return writer_->get_storedEvents(); }
    std::vector<std::string>& get_storedNames() { return writer_->get_storedNames(); }
    boost::shared_ptr<stor::SMOnlyStats> get_stats() { return writer_->get_stats(); }
  private:
    uint32 runNumber_;
    uint32 disks_;
    std::string catalog_;
    std::string sourceId_;
    log4cplus::Logger& applicationLogger_;
    boost::shared_ptr<FragmentQueue> newFragmentQueue_;
    boost::shared_ptr<DiscardManager> discardManager_;

    std::auto_ptr<edm::ServiceManager> writer_;
    std::auto_ptr<stor::DQMServiceManager> dqmServiceManager_;

    boost::shared_ptr<EventServer> eventServer_;
    boost::shared_ptr<DQMEventServer> DQMeventServer_;
    boost::shared_ptr<InitMsgCollection> initMsgCollection_;
    SMFUSenderList* smRBSenderList_;

    FragmentStore fragmentStore_;
  };
}

#endif
/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
