// $Id: ServiceManager.cc,v 1.18.6.8 2009/03/17 14:39:16 biery Exp $

#include <EventFilter/StorageManager/interface/ServiceManager.h>
#include "FWCore/Framework/interface/EventSelector.h"
#include "FWCore/ParameterSet/interface/PythonProcessDesc.h"
#include <FWCore/Utilities/interface/Exception.h>
#include <typeinfo>

using namespace std;
using namespace edm;
using boost::shared_ptr;


ServiceManager::ServiceManager(stor::DiskWritingParams dwParams):
  outModPSets_(0),
  psetHLTOutputLabels_(0),
  outputModuleIds_(0),
  currentlumi_(0),
  timeouttime_(0),
  lasttimechecked_(0),
  errorStreamPSetIndex_(-1),
  errorStreamCreated_(false),
  samples_(1000),
  period4samples_(10),
  diskWritingParams_(dwParams)
{
  collectStreamerPSets(dwParams._streamConfiguration);
  pmeter_ = new stor::SMPerformanceMeter();
  pmeter_->init(samples_, period4samples_);
} 


ServiceManager::~ServiceManager()
{ 
  outputModuleIds_.clear();
  delete pmeter_;
}


void ServiceManager::start()
{
  psetHLTOutputLabels_.clear();
  for (unsigned int idx = 0; idx < outModPSets_.size(); idx++) {
    psetHLTOutputLabels_.push_back(std::string());  // empty string
  }

  outputModuleIds_.clear();

  currentlumi_ = 0;
  timeouttime_ = 0;
  lasttimechecked_ = 0;
  errorStreamCreated_ = false;
  pmeter_->init(samples_, period4samples_);
}


void ServiceManager::stop()
{
}


void ServiceManager::manageInitMsg(InitMsgView& view, stor::InitMsgCollection& initMsgCollection)
{
  std::string inputOMLabel = view.outputModuleLabel();
  int psetIdx = -1;
  for(std::vector<ParameterSet>::iterator it = outModPSets_.begin(), itEnd = outModPSets_.end();
      it != itEnd; ++it) {
    ++psetIdx;
    bool createStreamNow = false;

    // if this ParameterSet corresponds to the error stream, skip over it
    // since the error stream doesn't use the INIT messages.
    if (psetIdx == errorStreamPSetIndex_) continue;

    // test if this INIT message is the right one for this output module
    // (that is, whether the HLT output module specified in the 
    // SM output module SelectHLTOutput parameter matches the HLT output
    // module in the INIT message)

    // fetch the SelectHLTOutput parameter from the SM output PSet
    std::string requestedOMLabel =
      it->getUntrackedParameter<std::string>("SelectHLTOutput", std::string());

    // if the SM output PSet didn't specify an HLT output module...
    //
    // (Note that the SelectHLTOutput parameter is optional.  If it is not
    // specified, we create the stream using the first INIT message that
    // we receive.  However, if we get multiple, different, INIT messages,
    // we complain loudly.
    // By allowing it to be optional, though, we provide some level
    // of backward compatibility - setups that only have one HLT output
    // module aren't forced to add this parameter.)
    if (requestedOMLabel.empty()) {

      // if we haven't yet created the stream object, go ahead and do
      // that using this INIT message
      if (psetHLTOutputLabels_[psetIdx].empty()) {
        createStreamNow = true;
      }

      // if we already created the stream object and this is a different
      // INIT message than what we used to create it, we need to complain
      // because SM output PSets are required to have a SelectHLTOutput
      // parameter in the presence of multiple INIT messages (multiple
      // HLT output modules)
      else if (inputOMLabel != psetHLTOutputLabels_[psetIdx]) {
        std::string errorString;
        errorString.append("ERROR: The configuration for Stream ");
        errorString.append((*it).getParameter<string> ("streamLabel"));
        errorString.append(" does not specify an HLT output module.\n");
        errorString.append("Please specify one of the HLT output modules ");
        errorString.append("listed below as the SelectHLTOutput parameter ");
        errorString.append("in the EventStreamFileWriter configuration ");
        errorString.append("for Stream ");
        errorString.append((*it).getParameter<string> ("streamLabel"));
        errorString.append(".\n");
        errorString.append(initMsgCollection.getSelectionHelpString());
        errorString.append("\n");
        throw cms::Exception("ServiceManager","manageInitMsg")
          << errorString << std::endl;
      }
    }
  }
}

/**
 * Returns a map of the trigger selection strings for each output stream.
 */
std::map<std::string, Strings> ServiceManager::getStreamSelectionTable()
{
  std::map<std::string, Strings> selTable;
  int psetIdx = -1;
  for(std::vector<ParameterSet>::iterator it = outModPSets_.begin();
      it != outModPSets_.end(); ++it) {
    ++psetIdx;
    if (psetIdx == errorStreamPSetIndex_) continue;

    std::string streamLabel = it->getParameter<string> ("streamLabel");
    if (streamLabel.size() > 0) {
      selTable[streamLabel] = EventSelector::getEventSelectionVString(*it);
    }
  }
  return selTable;
}

//
// *** wrote similar example code in IOPool/Streamer/test/ParamSetWalker_t.cpp 
// *** this method is diluted version of same code.
// *** if more items needs to be extracted for config, refer to example code
//
void ServiceManager::collectStreamerPSets(const std::string& config)
{

     try{
       
       PythonProcessDesc py_pdesc(config.c_str());
       boost::shared_ptr<ProcessDesc> pdesc = py_pdesc.processDesc();

       boost::shared_ptr<ParameterSet> procPset = pdesc->getProcessPSet();
       
        ParameterSet allTrigPaths = procPset->
	 getUntrackedParameter<ParameterSet>("@trigger_paths");
       
       if (allTrigPaths.empty())
         throw cms::Exception("collectStreamerPSets","ServiceManager")
	   << "No Trigger or End Path Found in the Config File" <<endl;
       
       std::vector<std::string> allEndPaths = 
	 procPset->getParameter<std::vector<std::string> >("@end_paths");
       
       if (allEndPaths.empty())
	 throw cms::Exception("collectStreamerPSets","ServiceManager")
	   << "No End Path Found in the Config File" <<endl;
       
       for(std::vector<std::string>::iterator it = allEndPaths.begin(), itEnd = allEndPaths.end();
	   it != itEnd;
	   ++it) {
	   std::vector<std::string> anEndPath = procPset->getParameter<std::vector<std::string> >((*it));
	   for(std::vector<std::string>::iterator i = anEndPath.begin(), iEnd = anEndPath.end();
	       i != iEnd; ++i) {
	       ParameterSet aModInEndPathPset = 
		 procPset->getParameter<ParameterSet>((*i));
	       if (aModInEndPathPset.empty())
		 throw cms::Exception("collectStreamerPSets","ServiceManager")
		   << "Empty End Path Found in the Config File" <<endl;
	      
	       std::string mod_type = aModInEndPathPset.getParameter<std::string> ("@module_type");
	       if (mod_type == "EventStreamFileWriter") {
		 outModPSets_.push_back(aModInEndPathPset);
                 psetHLTOutputLabels_.push_back(std::string());  // empty string
               }
               else if (mod_type == "ErrorStreamFileWriter" ||
                        mod_type == "FRDStreamFileWriter") {
                 errorStreamPSetIndex_ = outModPSets_.size();
                 outModPSets_.push_back(aModInEndPathPset);
                 psetHLTOutputLabels_.push_back(std::string());  // empty string
               }
	   }
       }
     } catch (cms::Exception & e) {
       std::cerr << "cms::Exception: " << e.explainSelf() << std::endl;
       std::cerr << "std::Exception: " << e.what() << std::endl;
       throw cms::Exception("collectStreamerPSets") << e.explainSelf() << std::endl;
     }
}

boost::shared_ptr<stor::SMOnlyStats> ServiceManager::get_stats()
{ 
// Copy measurements for a different thread potentially
// TODO create each time or use a data member?
    boost::shared_ptr<stor::SMOnlyStats> outstats(new stor::SMOnlyStats() );

    if ( pmeter_->getStats().shortTermCounter_->hasValidResult() )
    {
      stor::SMPerfStats stats = pmeter_->getStats();

      outstats->instantBandwidth_= stats.shortTermCounter_->getValueRate();
      outstats->instantRate_     = stats.shortTermCounter_->getSampleRate();
      outstats->instantLatency_  = 1000000.0 / outstats->instantRate_;

      double now = stor::ForeverCounter::getCurrentTime();
      outstats->totalSamples_    = stats.longTermCounter_->getSampleCount();
      outstats->duration_        = stats.longTermCounter_->getDuration(now);
      outstats->meanBandwidth_   = stats.longTermCounter_->getValueRate(now);
      outstats->meanRate_        = stats.longTermCounter_->getSampleRate(now);
      outstats->meanLatency_     = 1000000.0 / outstats->meanRate_;

      outstats->maxBandwidth_    = stats.maxBandwidth_;
      outstats->minBandwidth_    = stats.minBandwidth_;
    }

    // for time period bandwidth performance measurements
    if ( pmeter_->getStats().shortPeriodCounter_->hasValidResult() )
    {
      stor::SMPerfStats stats = pmeter_->getStats();

      outstats->instantBandwidth2_= stats.shortPeriodCounter_->getValueRate();
      outstats->instantRate2_     = stats.shortPeriodCounter_->getSampleRate();
      outstats->instantLatency2_  = 1000000.0 / outstats->instantRate2_;

      double now = stor::ForeverCounter::getCurrentTime();
      outstats->totalSamples2_    = stats.longTermCounter_->getSampleCount();
      outstats->duration2_        = stats.longTermCounter_->getDuration(now);
      outstats->meanBandwidth2_   = stats.longTermCounter_->getValueRate(now);
      outstats->meanRate2_        = stats.longTermCounter_->getSampleRate(now);
      outstats->meanLatency2_     = 1000000.0 / outstats->meanRate2_;

      outstats->maxBandwidth2_    = stats.maxBandwidth2_;
      outstats->minBandwidth2_    = stats.minBandwidth2_;
    }
    outstats->receivedVolume_ = pmeter_->totalvolumemb();
    outstats->samples_ = samples_;
    outstats->period4samples_ = period4samples_;
    return outstats;
}
