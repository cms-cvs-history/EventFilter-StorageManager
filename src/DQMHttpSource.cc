// $Id: DQMHttpSource.cc,v 1.22.2.2 2011/02/24 15:05:09 mommsen Exp $
/// @file: DQMHttpSource.cc

#include "EventFilter/StorageManager/interface/CurlInterface.h"
#include "EventFilter/StorageManager/src/DQMHttpSource.h"
#include "EventFilter/StorageManager/src/EventServerProxy.icc"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "FWCore/Utilities/interface/Exception.h"
#include "IOPool/Streamer/interface/StreamDQMDeserializer.h"

#include "TClass.h"

#include <string>
#include <vector>


namespace edm
{  
  DQMHttpSource::DQMHttpSource
  (
    const ParameterSet& pset,
    const InputSourceDescription& desc
  ) :
  edm::RawInputSource(pset, desc),
  _dqmStore(0),
  _dqmEventServerProxy(pset)
  {}


  std::auto_ptr<Event> DQMHttpSource::readOneEvent()
  {
    stor::CurlInterface::Content data;

    _dqmEventServerProxy.getOneEvent(data);
    if ( data.empty() ) return std::auto_ptr<edm::Event>();

    HeaderView hdrView(&data[0]);
    if (hdrView.code() == Header::DONE)
      return std::auto_ptr<edm::Event>();

    const DQMEventMsgView dqmEventMsgView(&data[0]);
    addEventToDQMBackend(dqmEventMsgView);

    // make a fake event containing no data but the evId and runId from DQMEvent
    // and the time stamp from the event at update
    std::auto_ptr<Event> e = makeEvent(
      dqmEventMsgView.runNumber(),
      dqmEventMsgView.lumiSection(),
      dqmEventMsgView.eventNumberAtUpdate(),
      dqmEventMsgView.timeStamp()
    );

    return e;
  }
  
  
  void DQMHttpSource::addEventToDQMBackend(const DQMEventMsgView& dqmEventMsgView)
  {
    initializeDQMStore();
    
    edm::StreamDQMDeserializer deserializeWorker;
    std::auto_ptr<DQMEvent::TObjectTable> toTablePtr =
      deserializeWorker.deserializeDQMEvent(dqmEventMsgView);
    
    for (DQMEvent::TObjectTable::const_iterator tableIter = toTablePtr->begin(),
           tableIterEnd = toTablePtr->end(); tableIter != tableIterEnd; ++tableIter)
    {
      const std::string subFolderName = tableIter->first;
      std::vector<TObject*> toList = tableIter->second;
      _dqmStore->makeDirectory(subFolderName);  // fetch or create
      _dqmStore->setCurrentFolder(subFolderName);

      for (std::vector<TObject*>::const_iterator objectIter = toList.begin(),
             objectIterEnd = toList.end(); objectIter != objectIterEnd; ++objectIter)
      {
        _dqmStore->extract(*objectIter, _dqmStore->pwd(), true);
        // TObject cloned into DQMStore. Thus, delete it here.
        delete *objectIter;
      }
    }
  }
  

  void DQMHttpSource::initializeDQMStore()
  {
    if ( ! _dqmStore )
      _dqmStore = edm::Service<DQMStore>().operator->();
    
    if ( ! _dqmStore )
      throw cms::Exception("readOneEvent", "DQMHttpSource")
        << "Unable to lookup the DQMStore service!\n";
  }

} // namespace edm


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
