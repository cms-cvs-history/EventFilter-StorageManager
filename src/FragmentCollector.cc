
#include "EventFilter/StorageManager/interface/FragmentCollector.h"
#include "EventFilter/StorageManager/test/SillyLockService.h"
#include "IOPool/Streamer/interface/Messages.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "IOPool/Streamer/interface/MsgHeader.h"
#include "IOPool/Streamer/interface/StreamTranslator.h"
#include "IOPool/Streamer/interface/EventMessage.h"
#include "IOPool/Streamer/interface/InitMessage.h"
#include "IOPool/Streamer/interface/EOFRecordBuilder.h"

//#include "IOPool/Streamer/interface/DumpTools.h"

#include "boost/bind.hpp"

#include <algorithm>
#include <utility>
#include <cstdlib>

using namespace edm;
using namespace std;

static const bool debugme = getenv("FRAG_DEBUG")!=0;  
#define FR_DEBUG if(debugme) std::cerr

namespace stor
{

  //FragmentCollector::FragmentCollector(const HLTInfo& h,Deleter d,
  FragmentCollector::FragmentCollector(HLTInfo& h,Deleter d,
                                       const string& config_str):
    cmd_q_(&(h.getCommandQueue())),
    evtbuf_q_(&(h.getEventQueue())),
    frag_q_(&(h.getFragmentQueue())),
    buffer_deleter_(d),
    event_area_(1000*1000*7),
    inserter_(*evtbuf_q_),
    prods_(0),//prods_(&p),
	info_(&h), 
    runNumber_(0),maxFileSize_(1073741824), highWaterMark_(0.9),
    writer_(new edm::StreamerOutSrvcManager(config_str)),
    evtsrv_area_(10),
    oneinN_(10), count_4_oneinN_(0) // added for Event Server by HWKC
  {
    // supposed to have given parameterSet smConfigString to writer_
    // at ctor
  }
  FragmentCollector::FragmentCollector(std::auto_ptr<HLTInfo> info,Deleter d,
                                       const string& config_str):
    cmd_q_(&(info.get()->getCommandQueue())),
    evtbuf_q_(&(info.get()->getEventQueue())),
    frag_q_(&(info.get()->getFragmentQueue())),
    buffer_deleter_(d),
    event_area_(1000*1000*7),
    inserter_(*evtbuf_q_),
    prods_(0),
	info_(info.get()), 
    runNumber_(0),maxFileSize_(1073741824), highWaterMark_(0.9),
    writer_(new edm::StreamerOutSrvcManager(config_str)),
    evtsrv_area_(10),
    oneinN_(10), count_4_oneinN_(0) // added for Event Server by HWKC
  {
    // supposed to have given parameterSet smConfigString to writer_
    // at ctor
  }

  FragmentCollector::~FragmentCollector()
  {
  }

  void FragmentCollector::run(FragmentCollector* t)
  {
    t->processFragments();
  }

  void FragmentCollector::start()
  {
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
    // we recogize - either execute the command, forward it
    // to the command queue, or process it for output to the 
    // event queue.
    bool done=false;

    while(!done)
      {
	EventBuffer::ConsumerBuffer cb(*frag_q_);
	if(cb.size()==0) break;
	FragEntry* entry = (FragEntry*)cb.buffer();
	FR_DEBUG << "FragColl: " << (void*)this << " Got a buffer size="
		 << entry->buffer_size_ << endl;
	switch(entry->code_)
	  {
	  case Header::EVENT:
	    {
	      FR_DEBUG << "FragColl: Got an Event" << endl;
	      processEvent(entry);
	      break;
	    }
	  case Header::DONE:
	    {
	      // make sure that this is actually sent by the controller! (JBK)
              // this does nothing currently
	      FR_DEBUG << "FragColl: Got a Done" << endl;
	      done=true;
	      break;
	    }
	  case Header::INIT:
	    {
	      FR_DEBUG << "FragColl: Got an Init" << endl;
	      processHeader(entry);
	      break;
	    }
	  default:
	    {
	      FR_DEBUG << "FragColl: Got junk" << endl;
	      break; // lets ignore other things for now
	    }
	  }
      }
    
    FR_DEBUG << "FragColl: DONE!" << endl;
    //edm::EventBuffer::ProducerBuffer cb(*evtbuf_q_);
    //long* vp = (long*)cb.buffer();
    //*vp=0;
    //cb.commit(sizeof(long));

    // file is not closed until the writers inside writer_ is destroyed
    if(streamerOnly_)  writer_->stop();
  }

  void FragmentCollector::stop()
  {
    // called from a different thread - trigger completion to the
    // fragment collector, which will cause a completion of the 
    // event processor

    edm::EventBuffer::ProducerBuffer cb(*frag_q_);
    cb.commit();
  }

  void FragmentCollector::processEvent(FragEntry* entry)
  {
    if(entry->totalSegs_==1)
      {
	FR_DEBUG << "FragColl: Got an Event with one segment" << endl;
	FR_DEBUG << "FragColl: Event size " << entry->buffer_size_ << endl;
	FR_DEBUG << "FragColl: Event ID " << entry->id_ << endl;

	// send immediately
        EventMsgView emsg(entry->buffer_address_);
        // See if writing Root or streamer files
        if(!streamerOnly_)
        {
          // Not a valid choice anymore - maybe later we put this back in
          /*
          std::auto_ptr<edm::EventPrincipal> evtp;
          {
            boost::mutex::scoped_lock sl(info_->getExtraLock());
            evtp = StreamTranslator::deserializeEvent(emsg, *prods_);
           }
           inserter_.send(evtp);
           */
        } else {
          FR_DEBUG << "FragColl: writing event size " << entry->buffer_size_ << endl;
          writer_->manageEventMsg(emsg);
        }

        // added for Event Server by HWKC - copy event to Event Server buffer
        count_4_oneinN_++;
        if(count_4_oneinN_ == oneinN_)
        {
          evtsrv_area_.push_back(emsg);
          count_4_oneinN_ = 0;
        }
        if (eventServer_.get() != NULL)
        {
          eventServer_->processEvent(emsg);
        }

	// is the buffer properly released (deleted)? (JBK)
	(*buffer_deleter_)(entry);
	return;
      } // end of single segment test

    pair<Collection::iterator,bool> rc =
      fragment_area_.insert(make_pair(entry->id_,Fragments()));
    
    rc.first->second.push_back(*entry);
    FR_DEBUG << "FragColl: added fragment" << endl;
    
    if((int)rc.first->second.size()==entry->totalSegs_)
      {
	FR_DEBUG << "FragColl: completed an event with "
		 << entry->totalSegs_ << " segments" << endl;
	// we are done with this event
	// assemble parts
	unsigned char* pos = (unsigned char*)&event_area_[0];
	
	int sum=0;
	unsigned int lastpos=0;
	Fragments::iterator
	  i(rc.first->second.begin()),e(rc.first->second.end());

	for(;i!=e;++i)
	  {
	    int dsize = i->buffer_size_;
	    sum+=dsize;
	    unsigned char* from=(unsigned char*)i->buffer_address_;
	    copy(from,from+dsize,pos+lastpos);
            lastpos = lastpos + dsize;
	    // ask deleter to kill off the buffer
	    (*buffer_deleter_)(&(*i));
	  }

        EventMsgView emsg(&event_area_[0]);
        if(!streamerOnly_)
        { // if writing Root files - but not valid now
          /*
          std::auto_ptr<edm::EventPrincipal> evtp;
          {
            boost::mutex::scoped_lock sl(info_->getExtraLock());
            evtp = StreamTranslator::deserializeEvent(emsg, *prods_);
          }
          inserter_.send(evtp);
          */
        } else { // writing streamer files
          FR_DEBUG << "FragColl: writing event size " << sum << endl;
          writer_->manageEventMsg(emsg);
        }

        // added for Event Server by HWKC - copy event to Event Server buffer
        // note that em does not have the correct totalsize in totalSize()
        // the ring buffer must use msgSize() or we send always 7MB events
        count_4_oneinN_++;
        if(count_4_oneinN_ == oneinN_)
        {
          evtsrv_area_.push_back(emsg);
          count_4_oneinN_ = 0;
        }
        if (eventServer_.get() != NULL)
        {
          eventServer_->processEvent(emsg);
        }

	// remove the entry from the map
	fragment_area_.erase(rc.first);
      }
  }
  void FragmentCollector::processHeader(FragEntry* entry)
  {
    InitMsgView msg(entry->buffer_address_);

    //if(entry->totalSegs_==1) // should test if these are fragments
    // currently these are taken from the already combined registry
    // fragments if any - need to change where the fragments are
    // queued here and remade here?
   
    // open file here as there is only one of these per run
    FR_DEBUG << "FragmentCollector: streamer file starting with " << filen_ << endl;
    FR_DEBUG << "FragColl: writing INIT size " << entry->buffer_size_ << endl;
    //dumpInitHeader(&msg);
    // should be passing smConfigSTring to writer_ at construction
    writer_->manageInitMsg(filen_, runNumber_, maxFileSize_, highWaterMark_, path_, mpath_, catalog_, disks_, msg);
  }
}
