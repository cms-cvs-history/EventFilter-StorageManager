


/*----------------------------------------------------------------------

 $Id:

The EPRunner and Collector stop action need to be defined before 
this can run.

----------------------------------------------------------------------*/  

#include <exception>
#include <iostream>
#include <string>
#include <vector>

#include "DataFormats/Provenance/interface/ProductRegistry.h"
#include "FWCore/Utilities/interface/Exception.h"
#include "FWCore/MessageService/interface/MessageServicePresence.h"
#include "IOPool/Streamer/interface/Utilities.h"
#include "IOPool/Streamer/interface/TestFileReader.h"
#include "IOPool/Streamer/interface/HLTInfo.h"
#include "IOPool/Streamer/interface/ClassFiller.h"
#include "EventFilter/StorageManager/interface/EPRunner.h"
#include "EventFilter/StorageManager/interface/FragmentCollector.h"
#include "EventFilter/StorageManager/interface/InitMsgCollection.h"
#include "EventFilter/StorageManager/interface/Configurator.h"
#include "EventFilter/StorageManager/interface/SharedResources.h"

#include "boost/shared_ptr.hpp"

#include "FWCore/PluginManager/interface/PluginManager.h"
#include "FWCore/PluginManager/interface/standard.h"

#include "log4cplus/configurator.h"
#include "log4cplus/logger.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/unistd.h>

using namespace std;
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
  stor::EPRunner drain_;
  boost::shared_ptr<stor::FragmentCollector> coll_;
  typedef boost::shared_ptr<edmtestp::TestFileReader> ReaderPtr;
  typedef vector<ReaderPtr> Readers;
  Readers readers_;
  log4cplus::Logger logger_;
  stor::SharedResources sharedResources_;
};

// ----------- implementation --------------


Main::~Main() { }


Main::Main(const string& conffile, const vector<string>& file_names):
  names_(file_names),
  prods_(edm::getRegFromFile(file_names[0])),
  drain_(getFileContents(conffile),auto_ptr<HLTInfo>(new HLTInfo(prods_))),
  logger_(log4cplus::Logger::getRoot())  // placeholder, overwritten below
{
  cout << "ctor of Main" << endl;

  log4cplus::BasicConfigurator config;
  config.configure();
  logger_ = log4cplus::Logger::getInstance("main");

  sharedResources_._fragmentQueue.reset(new stor::FragmentQueue(128));

  coll_.reset(new stor::FragmentCollector(*drain_.getInfo(),
                                          logger_,sharedResources_,
                                          conffile));

  boost::shared_ptr<stor::InitMsgCollection>
    initMsgCollection(new stor::InitMsgCollection());
  coll_->setInitMsgCollection(initMsgCollection);

  boost::shared_ptr<stor::Parameter> smParameter =
    stor::Configurator::instance()->getParameter();
  // the following directory path must have "open", "closed", and
  // "log" subdirectories!
  smParameter->setfilePath("/tmp/biery");

  // jbk - the next line should not be needed
  // edm::declareStreamers(prods_);
  vector<string>::iterator it(names_.begin()),en(names_.end());
  for(;it!=en;++it)
    {
      ReaderPtr p(new edmtestp::TestFileReader(*it,
					       drain_.getInfo()->getFragmentQueue(),
					       prods_));
      readers_.push_back(p);
    }
}

int Main::run()
{
  cout << "starting the EP" << endl;
  drain_.start();
  coll_->start();

  cout << "started the EP" << endl;
  // sleep(10);

  // start file readers
  Readers::iterator it(readers_.begin()),en(readers_.end());
  for(;it!=en;++it)
    {
      (*it)->start();
    }
  // wait for all file readers to complete
  for(it=readers_.begin();it!=en;++it)
    {
      (*it)->join();
    }

  // send done to the drain
  edm::EventBuffer::ProducerBuffer b(coll_->getFragmentQueue());
  b.commit();

  coll_->join();
  drain_.join();
  return 0;
}

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

  edm::service::MessageServicePresence theMessageServicePresence;
  edmplugin::PluginManager::configure(edmplugin::standard::config());
  string conffile(argv[1]);
  
  vector<string> file_names;
  
  for(int i=2;i<argc;++i)
    {
      cout << argv[i] << endl;
      file_names.push_back(argv[i]);
    }
  
  try {
    edm::loadExtraClasses();
    cout << "Done loading extra classes" << endl;
    Main m(conffile,file_names);
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

