
#include "EventFilter/StorageManager/interface/JobController.h"

#include "boost/bind.hpp"

using namespace std;
using namespace edm;

using boost::thread;
using boost::bind;

/*
  the state are not being set properly
 */

namespace stor
{
  JobController::~JobController()
  {
  }

  JobController::JobController(log4cplus::Logger& applicationLogger,
                               boost::shared_ptr<SharedResources> sharedResources) :
  applicationLogger_(applicationLogger)      
  {
    init(sharedResources);
  } 

  void JobController::init(boost::shared_ptr<SharedResources> sharedResources)
  {
    std::auto_ptr<HLTInfo> inf(new HLTInfo());

    std::auto_ptr<FragmentCollector> 
	coll(new FragmentCollector(inf,applicationLogger_,
				   sharedResources));

    collector_.reset(coll.release());
    //ep_runner_.reset(ep.release());
  }

  void JobController::run(JobController* t)
  {
    t->processCommands();
  }

  void JobController::start()
  {
    // called from a differnt thread to start things going

    me_.reset(new boost::thread(boost::bind(JobController::run,this)));
    collector_->start();
    //ep_runner_->start();
  }

  void JobController::stop()
  {
    // called from a different thread - trigger completion to the
    // job controller, which will cause a completion of the 
    // fragment collector and event processor

    //edm::EventBuffer::ProducerBuffer cb(ep_runner_->getInfo()->getCommandQueue());
    edm::EventBuffer::ProducerBuffer cb(collector_->getCommandQueue());
    MsgCode mc(cb.buffer(),MsgCode::DONE);
    mc.setCode(MsgCode::DONE);
    cb.commit(mc.codeSize());

    // should we wait here until the event processor and fragment
    // collectors are done?  Right now the wait is in the join.
  }

  void JobController::join()
  {
    // invoked from a different thread - block until "me_" is done
    if(me_) me_->join();
  }

  void JobController::processCommands()
  {
    // called with this jobcontrollers own thread.
    // wait for command messages, and periodically send "file check"
    // messages to the FragmentCollector
    while(1)
      {
        // 02-Sep-2008, KAB: avoid the creation of a consumer buffer
        // (which blocks) if there are no messages on the queue
        if(!(collector_->getCommandQueue().empty()))
          {
            //edm::EventBuffer::ConsumerBuffer cb(ep_runner_->getInfo()->getCommandQueue());
            edm::EventBuffer::ConsumerBuffer cb(collector_->getCommandQueue());
            MsgCode mc(cb.buffer(),cb.size());

            if(mc.getCode()==MsgCode::DONE) break;

            // if this is an intialization message, then it is a new system
            // attempting to connect or an old system reconnecting
            // we must verify that the configuration in the HLTInfo object
            // is consistent with this new one.

            // right now we will ignore all messages
          }
        else
          {
            ::sleep(1);
          }
      }    

    // do not exit the thread until all subthreads are complete

    collector_->stop();
    collector_->join();
    //ep_runner_->join();
  }
}
