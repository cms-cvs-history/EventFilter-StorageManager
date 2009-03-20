#ifndef HLT_JOB_CNTLER_HPP
#define HLT_JOB_CNTLER_HPP
// $Id: JobController.h,v 1.22.6.12 2009/03/20 10:28:19 mommsen Exp $

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
    JobController(log4cplus::Logger& applicationLogger,
                  SharedResourcesPtr sharedResources);

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

    std::list<std::string>& get_filelist() { return collector_->get_filelist(); }
    std::list<std::string>& get_currfiles() { return collector_->get_currfiles(); }
    std::vector<uint32>& get_storedEvents() { return collector_->get_storedEvents(); }
    std::vector<std::string>& get_storedNames() { return collector_->get_storedNames(); }
    boost::shared_ptr<stor::SMOnlyStats> get_stats() { return collector_->get_stats(); }

  private:
    void init(SharedResourcesPtr sharedResources);
    void processCommands();
    static void run(JobController*);

    boost::shared_ptr<FragmentCollector> collector_;
    SMFUSenderList* smRBSenderList_;

    log4cplus::Logger& applicationLogger_;

    boost::shared_ptr<boost::thread> me_;
  };
}

#endif

