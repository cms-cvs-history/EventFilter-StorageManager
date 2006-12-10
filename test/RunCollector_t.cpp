


/*----------------------------------------------------------------------

 $Id:

----------------------------------------------------------------------*/  
// The FragmentCollector no longer puts events into the EventBuffer
// so the drain will not get any events

#include <exception>
#include <iostream>
#include <string>
#include <vector>
#include <memory>

#include "DataFormats/Common/interface/ProductRegistry.h"
#include "FWCore/Utilities/interface/Exception.h"
#include "IOPool/Streamer/interface/Utilities.h"
#include "IOPool/Streamer/interface/TestFileReader.h"
#include "IOPool/Streamer/interface/HLTInfo.h"
#include "IOPool/Streamer/interface/ClassFiller.h"
#include "EventFilter/StorageManager/interface/FragmentCollector.h"
#include "IOPool/Streamer/interface/Messages.h"

#include "boost/shared_ptr.hpp"
#include "boost/bind.hpp"

#include "PluginManager/PluginManager.h"

#include <cstdlib>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/unistd.h>

using namespace std;
using namespace edm;
using stor::HLTInfo;

namespace {
  string getFileContents(const string& conffile)
  {
    struct stat b;
    if(stat(conffile.c_str(),&b)<0)
      {
	cerr << "Cannot stat() file " << conffile << endl;
	abort();
      }
    
    fstream ist(conffile.c_str());
    if(!ist)
      {
	cerr << "Could not open file " << conffile << endl;
	abort();
      }
    
    string rc(b.st_size,' ');
    ist.getline(&rc[0],b.st_size,fstream::traits_type::eof());
    return rc;
  }
}

static const bool drain_debug = getenv("DRAIN_DEBUG")!=0;
#define DR_DEBUG if(drain_debug) std::cerr


// -----------------------------------------------

class Drain
{
 public:
  explicit Drain(stor::HLTInfo& i);
  ~Drain();

  void start();
  void join() { me_->join(); }

 private:
  static void run(Drain*);
  void readData();

  int count_;
  edm::EventExtractor ext_;
  boost::shared_ptr<boost::thread> me_;
};

Drain::Drain(stor::HLTInfo& i):
  count_(),
  ext_(i.getEventQueue())
{
}

Drain::~Drain()
{
}

void Drain::start()
{
  me_.reset(new boost::thread(boost::bind(Drain::run,this)));
}

void Drain::run(Drain* d)
{
  cout << "Drain::run " << (void*)d << endl;
  d->readData();
}

void Drain::readData()
{
  while(1)
    {
      DR_DEBUG << "Drain: getting event" << endl;
      std::auto_ptr<edm::EventPrincipal> ep = ext_.extract();
      DR_DEBUG << "Drain: got event " << (void*)ep.get() << endl;
      // if(ep.release()==0) break;
      if(ep.get()==0) break;
      ++count_;
    }

  cout << "Drain: got " << count_ << " events" << endl;
}

// -----------------------------------------------

static void deleteBuffer(void* v)
{
	stor::FragEntry* fe = (stor::FragEntry*)v;
	delete [] (char*)fe->buffer_address_;
}

// -----------------------------------------------

class Main
{
 public:
  Main(const string& conffile, const vector<string>& file_names);
  ~Main();
  
  int run();

 private:

  // disallow the following
  Main(const Main& m);
  Main& operator=(const Main&) { return *this; }

  vector<string> names_;
  edm::ProductRegistry prods_;
  stor::HLTInfo info_;
  stor::FragmentCollector coll_;
  //stor::FragmentCollector coll2_;
  Drain drain_;
  typedef boost::shared_ptr<edmtestp::TestFileReader> ReaderPtr;
  typedef vector<ReaderPtr> Readers;
  Readers readers_;
};

// ----------- implementation --------------


Main::~Main() { }


Main::Main(const string& conffile, const vector<string>& file_names):
  names_(file_names),
  prods_(edm::getRegFromFile(file_names[0])),
  info_(prods_),
  coll_(info_,deleteBuffer,conffile),
  //coll2_(info_,deleteBuffer,prods_),
  drain_(info_)
{
  cout << "ctor of Main" << endl;
  // jbk - the next line should not be needed
  // edm::declareStreamers(prods_);
  vector<string>::iterator it(names_.begin()),en(names_.end());
  for(;it!=en;++it)
    {
      ReaderPtr p(new edmtestp::TestFileReader(*it,
					       info_.getFragmentQueue(),
					       prods_));
      readers_.push_back(p);
    }
  coll_.set_outoption(true); // to write out streamer files not root files
}

int Main::run()
{
  cerr << "starting the collector and drain" << endl;
  drain_.start();
  coll_.start();
  //coll2_.start();
  cerr << "started the collector and drain" << endl;
  // sleep(10);

  // start file readers
  Readers::iterator it(readers_.begin()),en(readers_.end());
  for(;it!=en;++it)
    {
      (*it)->start();
    }
  cerr << "started readers" << endl;
  // wait for all file readers to complete
  for(it=readers_.begin();it!=en;++it)
    {
      (*it)->join();
    }
  cerr << "readers done" << endl;

  // send MsgCode::DONE to the drain
  // jbk - this is broken now - it only sends a null
  edm::EventBuffer::ProducerBuffer b(info_.getFragmentQueue());
  b.commit();
  //edm::EventBuffer::ProducerBuffer b2(info_.getFragmentQueue());
  edm::EventBuffer::ProducerBuffer b2(info_.getEventQueue());
  b2.commit();

  cerr << "waiting for coll to be done" << endl;
  coll_.join();
  cerr << "coll done" << endl;
  cerr << "waiting for drain to be done" << endl;
  //coll2_.join();
  drain_.join();
  cerr << "drain done" << endl;
  return 0;
}

// ---------------------------------------------

int main(int argc, char* argv[])
{
  // pull options out of command line
  if(argc < 3)
    {
      cout << "Usage: " << argv[0] << " "
	   << "config_file inpfile1 inpfile2 ... inpfileN"
	   << endl;
      return 0;
      //throw cms::Exception("config") << "Bad command line arguments\n";
    }

  seal::PluginManager::get()->initialise();
  string conffile(argv[1]);
      cout << "config = " << argv[1] << endl;
  
  vector<string> file_names;
  
  for(int i=2;i<argc;++i)
    {
      cout << argv[i] << endl;
      file_names.push_back(argv[i]);
    }
  
  try {
    edm::loadExtraClasses();
    cout << "Done loading extra classes" << endl;
    Main m(getFileContents(conffile),file_names);
    m.run();
  }
  catch(cms::Exception& e)
    {
      cerr << "Caught an exception:\n" << e.what() << endl;
      throw;
    }
  catch(std::exception& e)
    {
      cerr << "Caught an exception:\n" << e.what() << endl;
      throw;
    }
  catch(...)
  {
      cerr << "Caught unknown exception\n" << endl;
  }

  cout << "Main is done!" << endl;
  return 0;
}

