// $Id: ServiceManager.cc,v 1.15 2008/08/27 22:41:10 biery Exp $

#include <EventFilter/StorageManager/interface/ServiceManager.h>
#include "EventFilter/StorageManager/interface/Configurator.h"
#include <EventFilter/StorageManager/interface/EventStreamService.h>
#include <EventFilter/StorageManager/interface/FRDStreamService.h>
#include "FWCore/Framework/interface/EventSelector.h"
#include <FWCore/Utilities/interface/Exception.h>
#include <typeinfo>

using namespace std;
using namespace edm;
using boost::shared_ptr;


ServiceManager::ServiceManager(const std::string& config):
  outModPSets_(0),
  managedOutputs_(0),
  psetHLTOutputLabels_(0),
  outputModuleIds_(0),
  storedEvents_(0),
  currentlumi_(0),
  timeouttime_(0),
  lasttimechecked_(0),
  errorStreamPSetIndex_(-1),
  errorStreamCreated_(false)
{
  storedNames_.clear();
  collectStreamerPSets(config);
} 


ServiceManager::~ServiceManager()
{ 
  managedOutputs_.clear();
  outputModuleIds_.clear();
  storedEvents_.clear();
  storedNames_.clear();
}


void ServiceManager::stop()
{
  for(StreamsIterator  it = managedOutputs_.begin(), itEnd = managedOutputs_.end();
      it != itEnd; ++it) {
      (*it)->stop();
  }

  psetHLTOutputLabels_.clear();
  for (unsigned int idx = 0; idx < outModPSets_.size(); idx++) {
    psetHLTOutputLabels_.push_back(std::string());  // empty string
  }

  managedOutputs_.clear();
  outputModuleIds_.clear();
  storedEvents_.clear();
  storedNames_.clear();

  currentlumi_ = 0;
  timeouttime_ = 0;
  lasttimechecked_ = 0;
  errorStreamCreated_ = false;
}


void ServiceManager::manageInitMsg(std::string catalog, uint32 disks, std::string sourceId, InitMsgView& view, stor::InitMsgCollection& initMsgCollection)
{
  boost::shared_ptr<stor::Parameter> smParameter_ = stor::Configurator::instance()->getParameter();
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

    // if the SM output PSet did specify an HLT output module...
    else {

      // if the HLT output module labels match...
      if (inputOMLabel == requestedOMLabel) {

        // if we haven't yet created the stream object, go ahead and do
        // that using this INIT message
        if (psetHLTOutputLabels_[psetIdx].empty()) {
          createStreamNow = true;
        }

        // if we already created the stream object, we could complain,
        // but won't (for now) so that this method can support multiple
        // calls with the same INIT message
        else {}
      }

      // if the HLT output module labels do not match, do nothing
      else {}
    }

    if (createStreamNow) {
      shared_ptr<StreamService> stream = shared_ptr<StreamService>(new EventStreamService((*it),view));
      stream->setCatalog(catalog);
      stream->setNumberOfFileSystems(disks);
      stream->setSourceId(sourceId);
      stream->setFileName(smParameter_ -> fileName());
      stream->setFilePath(smParameter_ -> filePath());
      stream->setMaxFileSize(smParameter_ -> maxFileSize());
      stream->setSetupLabel(smParameter_ -> setupLabel());
      stream->setHighWaterMark(smParameter_ -> highWaterMark());
      stream->setLumiSectionTimeOut(smParameter_ -> lumiSectionTimeOut());
      managedOutputs_.push_back(stream);
      outputModuleIds_.push_back(view.outputModuleId());
      storedEvents_.push_back(0);
      storedNames_.push_back(stream->getStreamLabel());
      stream->report(cout,3);

      psetHLTOutputLabels_[psetIdx] = inputOMLabel;
    }
  }
}

void ServiceManager::manageEventMsg(EventMsgView& msg)
{
  int outputIdx = -1;
  for(StreamsIterator it = managedOutputs_.begin(), itEnd = managedOutputs_.end(); it != itEnd; ++it) {
    ++outputIdx;
    if (msg.outModId() != outputModuleIds_[outputIdx])
      continue;

    bool thisEventAccepted = (*it)->nextEvent(msg.startAddress());
    if (!thisEventAccepted)
      continue;

    ++storedEvents_[outputIdx];
    if ((*it)->lumiSection() > currentlumi_) {
      currentlumi_ = (*it)->lumiSection();
      timeouttime_ = (*it)->getCurrentTime();
    }
  }
}

void ServiceManager::manageErrorEventMsg(std::string catalog, uint32 disks, std::string sourceId, FRDEventMsgView& msg)
{
  // if no error stream was configured, we can exit early
  if (errorStreamPSetIndex_ < 0) return;

  // create the error stream, if needed
  if (! errorStreamCreated_) {
    ParameterSet errorStreamPSet = outModPSets_.at(errorStreamPSetIndex_);
    boost::shared_ptr<stor::Parameter> smParameter_ = stor::Configurator::instance()->getParameter();

    shared_ptr<StreamService> stream =
      shared_ptr<StreamService>(new FRDStreamService(errorStreamPSet));
    stream->setCatalog(catalog);
    stream->setNumberOfFileSystems(disks);
    stream->setSourceId(sourceId);
    stream->setFileName(smParameter_ -> fileName());
    stream->setFilePath(smParameter_ -> filePath());
    stream->setMaxFileSize(smParameter_ -> maxFileSize());
    stream->setSetupLabel(smParameter_ -> setupLabel());
    stream->setHighWaterMark(smParameter_ -> highWaterMark());
    stream->setLumiSectionTimeOut(smParameter_ -> lumiSectionTimeOut());
    managedOutputs_.push_back(stream);
    outputModuleIds_.push_back(0xffffffff);
    storedEvents_.push_back(0);
    storedNames_.push_back(stream->getStreamLabel());
    stream->report(cout,3);

    psetHLTOutputLabels_[errorStreamPSetIndex_] = "ResourceBroker Error Output";

    errorStreamCreated_ = true;
  }

  // process the event
  int outputIdx = -1;
  for(StreamsIterator strIter = managedOutputs_.begin(), strIterEnd = managedOutputs_.end(); strIter != strIterEnd; ++strIter) {
    ++outputIdx;
    std::string streamClassName = typeid(*(*strIter)).name();
    if (streamClassName.find("FRDStreamService", 0) == string::npos)
      continue;

    bool thisEventAccepted = (*strIter)->nextEvent(msg.startAddress());
    if (!thisEventAccepted)
      continue;

    ++storedEvents_[outputIdx];

    // for now, we don't have any lumi section information in the
    // FRDEvent messages, so we don't try to do any lumi-boundary processing
  }
}


void ServiceManager::closeFilesIfNeeded()
{
  StreamsIterator itBeg = managedOutputs_.begin();
  StreamsIterator itEnd = managedOutputs_.end();
  for(StreamsIterator it = itBeg; it != itEnd; ++it) {
    (*it)->closeTimedOutFiles();
  }
}

//
// *** get all files from all streams
//
std::list<std::string>& ServiceManager::get_filelist() 
{ 
  filelist_.clear();
  for(StreamsIterator it = managedOutputs_.begin(), itEnd = managedOutputs_.end();
      it != itEnd; ++it) {
      std::list<std::string> sub_list = (*it)->getFileList();
      if(sub_list.size() > 0)
	filelist_.insert(filelist_.end(), sub_list.begin(), sub_list.end());
  } 
  return filelist_; 
}


//
// *** get all current files from all streams
//
std::list<std::string>& ServiceManager::get_currfiles()
{ 
  currfiles_.clear();
  for(StreamsIterator it = managedOutputs_.begin(), itEnd = managedOutputs_.end();
      it != itEnd; ++it) {
      std::list<std::string> sub_list = (*it)->getCurrentFileList();
      if(sub_list.size() > 0)
	currfiles_.insert(currfiles_.end(), sub_list.begin(), sub_list.end());
  }
  return currfiles_;  
}

//
std::vector<uint32>& ServiceManager::get_storedEvents()
{ 
  return storedEvents_;  
}
std::vector<std::string>& ServiceManager::get_storedNames()
{ 
  return storedNames_;  
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
       
       ProcessDesc  pdesc(config.c_str());
       
       boost::shared_ptr<ParameterSet> procPset = pdesc.getProcessPSet();
       
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

