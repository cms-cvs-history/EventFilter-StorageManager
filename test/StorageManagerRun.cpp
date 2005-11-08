/*----------------------------------------------------------------------

 $Id$

----------------------------------------------------------------------*/  

#include <exception>
#include <iostream>
#include <string>
#include <vector>

#include "FWCore/Framework/interface/EventProcessor.h"
#include "FWCore/Framework/interface/ProductRegistry.h"
#include "FWCore/Utilities/interface/ProblemTracker.h"
#include "FWCore/Utilities/interface/Exception.h"
#include "IOPool/Streamer/interface/HLTInfo.h"
#include "IOPool/Streamer/interface/Utilities.h"
#include "IOPool/Streamer/interface/TestFileReader.h"
#include "EventFilter/StorageManager/interface/JobController.h"

#include "boost/shared_ptr.hpp"

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

void deleteBuffer(void* v)
{
  char* b = (char*)v;
  delete [] b;
}

// -----------------------------------------------

class Main // : public xdaq::Application
{
 public:
  //XDAQ_INSTANTIATOR();
  //Main(xdaq::ApplicationStub* s);

  Main(const string& fu_config_file,
       const string& my_config_file,
       const vector<string>& file_names);
  ~Main();
  
  int run();

 private:
  // disallow the following
  Main(const Main&):jc_("","",deleteBuffer) { }
  Main& operator=(const Main&) { return *this; }

  edm::AssertHandler ah_;
  stor::JobController jc_;
  vector<string> names_;
  typedef boost::shared_ptr<edmtestp::TestFileReader> ReaderPtr;
  typedef vector<ReaderPtr> Readers;
  Readers readers_;
};

// ----------- implementation --------------

Main::~Main() { }

#if 0
Main::Main(xdaq::ApplicationStub* s)
{
}
#endif


Main::Main(const string& fu_config_file,
	   const string& my_config_file,
	   const vector<string>& file_names):
  jc_(edm::getRegFromFile(file_names[0]),my_config_file,&deleteBuffer),
  names_(file_names)
{
  vector<string>::iterator it(names_.begin()),en(names_.end());
  for(;it!=en;++it)
    {
      ReaderPtr p(new edmtestp::TestFileReader(*it,
					       jc_.getFragmentQueue(),
					       jc_.products()));
      readers_.push_back(p);
    }
}

int Main::run()
{
  jc_.start();

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

  jc_.stop();
  jc_.join();

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
  if(argc < 4)
    {
      cerr << "Usage: " << argv[0] << " trigger_config my_config "
	   << "file1 file2 ... fileN\n"
	   << "trigger_config and my_config must be given along with at "
	   << "least one file name";
      throw cms::Exception("config") << "Bad command line arguments\n";
    }
  
  string fu_config_file(argv[1]);
  string my_config_file(argv[2]);
  vector<string> file_names;
  
  for(int i=3;i<argc;++i)
    file_names.push_back(argv[i]);
  
  Main m(fu_config_file,my_config_file,file_names);
  m.run();

  return 0;
}

#endif
