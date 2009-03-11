/*----------------------------------------------------------------------

 $Id: StorageManagerRun.cpp,v 1.14.12.3 2009/03/03 22:07:16 biery Exp $

----------------------------------------------------------------------*/  

#include <exception>
#include <iostream>
#include <string>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/unistd.h>

#include "DataFormats/Provenance/interface/ProductRegistry.h"
#include "FWCore/Utilities/interface/Exception.h"
#include "FWCore/MessageService/interface/MessageServicePresence.h"
#include "IOPool/Streamer/interface/HLTInfo.h"
#include "IOPool/Streamer/interface/TestFileReader.h"
#include "EventFilter/StorageManager/interface/JobController.h"
#include "FWCore/PluginManager/interface/PluginManager.h"
#include "FWCore/PluginManager/interface/standard.h"
#include "EventFilter/StorageManager/interface/SharedResources.h"

#include "IOPool/Streamer/interface/StreamerInputFile.h"
#include "IOPool/Streamer/interface/InitMessage.h"
#include "IOPool/Streamer/interface/StreamerInputSource.h"

#include "DataFormats/Streamer/interface/StreamedProducts.h"

#include "IOPool/Streamer/interface/ClassFiller.h"

#include "boost/shared_ptr.hpp"

#include "log4cplus/configurator.h"
#include "log4cplus/logger.h"

// #include "xdaq/include/xdaq/Application.h"


/*
  An event processor cannot be started without an initialization
  message that includes a product registry.  The product registry
  is necessary to configure the input and output modules.  Unfortunately
  this registry comes from the remote system.

  What this means is that something happens that I cannot remember.

  JBK Note: with the threading use in all these objects, once the 
  stop is called, the processing cannot be restarted.
 */

using namespace std;

// -----------------------------------------------

// this dleter is particular to this test.  It should really be
// defined in TestFileReader.h

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

class Main // : public xdaq::Application
{
 public:
  //XDAQ_INSTANTIATOR();
  //Main(xdaq::ApplicationStub* s);

  //Main(const string& fu_config_file,
  Main(const string& my_config_file,
       const vector<string>& file_names);
  ~Main();
  
  int run();

 private:
  // disallow the following
  Main& operator=(const Main&) { return *this; }

  stor::JobController* jc_;
  vector<string> names_;
  typedef boost::shared_ptr<edmtestp::TestFileReader> ReaderPtr;
  typedef vector<ReaderPtr> Readers;
  Readers readers_;
  log4cplus::Logger logger_;
  stor::SharedResources sharedResources_;
};

// ----------- implementation --------------

Main::~Main() { 
 
   delete jc_;
 
}

#if 0
Main::Main(xdaq::ApplicationStub* s)
{
}
#endif


//Main::Main(const string& fu_config_file,
Main::Main(const string& my_config_file,
	   const vector<string>& file_names):
  names_(file_names),
  logger_(log4cplus::Logger::getRoot())  // placeholder, overwritten below
{
  StreamerInputFile stream_reader(file_names[0]);
  const InitMsgView* init =  stream_reader.startMessage();
  std::auto_ptr<edm::SendJobHeader> header = edm::StreamerInputSource::deserializeRegistry(*init);

  edm::ProductRegistry pr;
  const edm::SendDescs& descs = header->descs();
  edm::SendDescs::const_iterator i(descs.begin()), e(descs.end());
  for(; i != e; ++i) {
        pr.copyProduct(*i);
        //FDEBUG(6) << "StreamInput prod = " << i->className() << endl;
    }

  log4cplus::BasicConfigurator config;
  config.configure();
  logger_ = log4cplus::Logger::getInstance("main");

  sharedResources_._fragmentQueue.reset(new stor::FragmentQueue(128));

  //jc_ = new stor::JobController(pr,
  jc_ = new stor::JobController(getFileContents(my_config_file),
                                logger_,sharedResources_);

  vector<string>::iterator it(names_.begin()),en(names_.end());
  for(;it!=en;++it)
    {
      ReaderPtr p(new edmtestp::TestFileReader(*it,
					       jc_->getFragmentQueue(),
					       pr));
					       //jc_->products()));
      readers_.push_back(p);
    }
}

int Main::run()
{
  jc_->start();

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

  jc_->stop();
  jc_->join();

  return 0;
}


#if 1

void printusage(char* cmdname)
{
  cerr << "The last argument must be --file X" << endl;  
}

int main(int argc, char* argv[])
{
  // pull options out of command line
  //if(argc < 4)
  if(argc < 3)
    {
      //cerr << "Usage: " << argv[0] << " trigger_config my_config "
      cerr << "Usage: " << argv[0] << " my_config "
	   << "file1 file2 ... fileN\n"
	   << "my_config must be given along with at "
	   << "least one file name";
      throw cms::Exception("config") << "Bad command line arguments\n";
    }
  
  edm::service::MessageServicePresence theMessageServicePresence;
  edmplugin::PluginManager::configure(edmplugin::standard::config());

  //string fu_config_file(argv[1]);
  string my_config_file(argv[1]);
  vector<string> file_names;
  
  //for(int i=3;i<argc;++i)
  for(int i=2;i<argc;++i)
    file_names.push_back(argv[i]);

  try {
    edm::loadExtraClasses(); 
    //Main m(fu_config_file,my_config_file,file_names);
    Main m(my_config_file,file_names);
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

  return 0;
}

#endif
