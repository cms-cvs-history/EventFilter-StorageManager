#ifndef HLT_JOB_CNTLER_HPP
#define HLT_JOB_CNTLER_HPP
// $Id: JobController.h,v 1.22.6.4 2009/03/11 17:30:56 biery Exp $

#include "EventFilter/StorageManager/interface/FragmentCollector.h"
#include "EventFilter/StorageManager/interface/SMPerformanceMeter.h"
#include "EventFilter/StorageManager/interface/SMFUSenderList.h"
#include "EventFilter/StorageManager/interface/SharedResources.h"

#include "IOPool/Streamer/interface/EventBuffer.h"
#include "IOPool/Streamer/interface/EventMessage.h"
#include "IOPool/Streamer/interface/Messages.h"

#include "boost/shared_ptr.hpp"
#include "boost/thread/thread.hpp"

#include "log4cplus/logger.h"

#include <string>

namespace stor
{

  class JobController
  {
  public:
    JobController(const std::string& my_config,
		  log4cplus::Logger& applicationLogger,
                  SharedResources sharedResources);

    ~JobController();

    void start();
    void stop();
    void join();

    edm::EventBuffer& getFragmentQueue()
    { return collector_->getFragmentQueue(); }

    void setSMRBSenderList(SMFUSenderList* senderList) {
      if (collector_.get() != NULL) collector_->setSMRBSenderList(senderList);
      smRBSenderList_ = senderList;
    }

    void setNumberOfFileSystems(int disks)    { collector_->setNumberOfFileSystems(disks); }
    void setFileCatalog(std::string catalog)  { collector_->setFileCatalog(catalog); }
    void setSourceId(std::string sourceId)    { collector_->setSourceId(sourceId); }
    void setCollateDQM(bool collateDQM)       { collector_->setCollateDQM(collateDQM);}
    void setArchiveDQM(bool archiveDQM)       { collector_->setArchiveDQM(archiveDQM);}
    void setArchiveIntervalDQM(int archiveInterval) {
      collector_->setArchiveIntervalDQM(archiveInterval);
    }
    void setPurgeTimeDQM(int purgeTimeDQM)    { collector_->setPurgeTimeDQM(purgeTimeDQM);}
    void setReadyTimeDQM(int readyTimeDQM)    { collector_->setReadyTimeDQM(readyTimeDQM);}
    void setFilePrefixDQM(std::string filePrefixDQM)  { collector_->setFilePrefixDQM(filePrefixDQM);}
    void setUseCompressionDQM(bool useCompressionDQM)
    { collector_->setUseCompressionDQM(useCompressionDQM);}
    void setCompressionLevelDQM(bool compressionLevelDQM)
    { collector_->setCompressionLevelDQM(compressionLevelDQM);}
    void setFileClosingTestInterval(int value) { fileClosingTestInterval_ = value; }

    std::list<std::string>& get_filelist() { return collector_->get_filelist(); }
    std::list<std::string>& get_currfiles() { return collector_->get_currfiles(); }
    std::vector<uint32>& get_storedEvents() { return collector_->get_storedEvents(); }
    std::vector<std::string>& get_storedNames() { return collector_->get_storedNames(); }
    boost::shared_ptr<stor::SMOnlyStats> get_stats() { return collector_->get_stats(); }

  private:
    void init(const std::string& my_config, SharedResources sharedResources);
    void processCommands();
    static void run(JobController*);

    boost::shared_ptr<FragmentCollector> collector_;
    SMFUSenderList* smRBSenderList_;

    int fileClosingTestInterval_;
    log4cplus::Logger& applicationLogger_;

    boost::shared_ptr<boost::thread> me_;
  };
}

#endif

