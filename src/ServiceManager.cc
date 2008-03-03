// $Id: ServiceManager.cc,v 1.5 2008/01/29 21:22:56 biery Exp $

#include <EventFilter/StorageManager/interface/ServiceManager.h>
#include "EventFilter/StorageManager/interface/Configurator.h"
#include "EventFilter/StorageManager/interface/InitMsgCollection.h"
#include <FWCore/Utilities/interface/Exception.h>

using namespace std;
using namespace edm;
using boost::shared_ptr;


ServiceManager::ServiceManager(const std::string& config):
  outModPSets_(0),
  managedOutputs_(0)
{
  collectStreamerPSets(config);
} 


ServiceManager::~ServiceManager()
{ 
  managedOutputs_.clear();
}


void ServiceManager::stop()
{
  for(StreamsIterator  it = managedOutputs_.begin(), itEnd = managedOutputs_.end();
      it != itEnd; ++it) {
      (*it)->stop();
  }
}


void ServiceManager::manageInitMsg(std::string catalog, uint32 disks, std::string sourceId, InitMsgView& view, stor::InitMsgCollection& initMsgCollection)
{
  boost::shared_ptr<stor::Parameter> smParameter_ = stor::Configurator::instance()->getParameter();
  for(std::vector<ParameterSet>::iterator it = outModPSets_.begin(), itEnd = outModPSets_.end();
      it != itEnd; ++it) {

      // test if this INIT message is the right one for this output module
      // (that is, whether the SM output module trigger request matches
      // the HLT output module trigger request in the INIT message)
      // Rather than reproduce the code in
      // InitMsgCollection::getElementForSelection for finding the right
      // INIT message for a given trigger selection, we simply use that
      // method to find the "right" INIT message and check if it matches
      // the one passed into this method.  Of course, this requires that
      // the INIT message passed into this method has already been added
      // to the collection.  But we get the (good) side benefit that the
      // error checking (and associated exceptions) in the
      // getElementForSelection method come for free.
      // If we ever switch to something other than the getElementForSelection
      // method, then we may need to keep track of each StreamService that
      // we create so that we make sure we don't create two of them for the
      // same output stream.
      Strings selections = EventSelector::getEventSelectionVString(*it);
      try {
        stor::InitMsgSharedPtr matchingInitMsgPtr =
          initMsgCollection.getElementForSelection(selections);
        if (matchingInitMsgPtr.get() != NULL) {
          InitMsgView matchingView(&(*matchingInitMsgPtr)[0]);
          if (matchingView.startAddress() == view.startAddress() &&
              matchingView.size() == view.size()) {
            shared_ptr<StreamService> stream = shared_ptr<StreamService>(new StreamService((*it),view));
            stream->setCatalog(catalog);
            stream->setNumberOfFileSystems(disks);
            stream->setSourceId(sourceId);
            stream->setFileName(smParameter_ -> fileName());
            stream->setFilePath(smParameter_ -> filePath());
            stream->setMathBoxPath(smParameter_ -> mailboxPath());
            stream->setSetupLabel(smParameter_ -> setupLabel());
            stream->setHighWaterMark(smParameter_ -> highWaterMark());
            stream->setLumiSectionTimeOut(smParameter_ -> lumiSectionTimeOut());
            managedOutputs_.push_back(stream);
            stream->report(cout,3);
          }
        }
      }
      catch (const cms::Exception& excpt) {
        std::string errorString;
        errorString.append("Problem with the SelectEvents parameter ");
        errorString.append("in the definition of Stream ");
        errorString.append((*it).getParameter<string> ("streamLabel"));
        errorString.append("\n");
        errorString.append(excpt.what());
        errorString.append("\n");
        errorString.append(initMsgCollection.getSelectionHelpString());
        errorString.append("\n\n");
        errorString.append("*** Please select trigger paths from one and ");
        errorString.append("only one HLT output module. ***\n");
        throw cms::Exception("ServiceManager","manageInitMsg")
          << errorString << std::endl;
      }
  }
}


void ServiceManager::manageEventMsg(EventMsgView& msg)
{
  bool eventAccepted = false;
  for(StreamsIterator  it = managedOutputs_.begin(), itEnd = managedOutputs_.end(); it != itEnd; ++it)
    eventAccepted = (*it)->nextEvent(msg) || eventAccepted;

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
	filelist_.insert(filelist_.end(), sub_list.begin(), sub_list.end());
  }
  return currfiles_;  
}

/**
 * Returns a map of the trigger selection strings for each output stream.
 */
std::map<std::string, Strings> ServiceManager::getStreamSelectionTable()
{
  std::map<std::string, Strings> selTable;
  for(std::vector<ParameterSet>::iterator it = outModPSets_.begin();
      it != outModPSets_.end(); ++it) {
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
	       if (mod_type == "EventStreamFileWriter") 
		 outModPSets_.push_back(aModInEndPathPset);
	   }
       }
     } catch (cms::Exception & e) {
       std::cerr << "cms::Exception: " << e.explainSelf() << std::endl;
       std::cerr << "std::Exception: " << e.what() << std::endl;
       throw cms::Exception("collectStreamerPSets") << e.explainSelf() << std::endl;
     }
}

