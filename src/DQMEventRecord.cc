// $Id: DQMEventRecord.cc,v 1.1.2.1 2009/04/17 17:28:56 mommsen Exp $

#include "EventFilter/StorageManager/interface/DQMEventRecord.h"

#include "IOPool/Streamer/interface/DQMEventMessage.h"
#include "IOPool/Streamer/interface/StreamDQMDeserializer.h"


using namespace stor;


DQMEventRecord::DQMEventRecord
(
  DQMKey const& dqmKey,
  DQMProcessingParams const& dqmParams
) :
DQMInstance(
  dqmKey.runNumber, dqmKey.lumiSection, dqmKey.updateNumber, 
  static_cast<int>(dqmParams._purgeTimeDQM),
  static_cast<int>(dqmParams._readyTimeDQM)
)
{}

void DQMEventRecord::addDQMEventView(boost::shared_ptr<DQMEventMsgView> view)
{
  edm::StreamDQMDeserializer deserializer;
  std::auto_ptr<DQMEvent::TObjectTable> toTablePtr =
    deserializer.deserializeDQMEvent(*view);

  DQMEvent::TObjectTable::const_iterator toIter;
  for (
    DQMEvent::TObjectTable::const_iterator it = toTablePtr->begin(),
      itEnd = toTablePtr->end();
    it != itEnd; 
    it++
  ) 
  {
    std::string subFolderName = it->first;
    std::vector<TObject *> toList = it->second;

    for (unsigned int tdx = 0; tdx < toList.size(); tdx++) 
    {
      TObject *object = toList[tdx];
      updateObject(
        view->topFolderName(),
        subFolderName,
        object,
        view->eventNumberAtUpdate()
      );
      delete(object);
    }
  }
}


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
