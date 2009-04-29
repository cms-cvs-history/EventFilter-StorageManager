// $Id: DQMEventRecord.cc,v 1.1.2.4 2009/04/23 13:27:04 mommsen Exp $

#include "EventFilter/StorageManager/interface/DQMEventRecord.h"

#include "IOPool/Streamer/interface/DQMEventMessage.h"
#include "IOPool/Streamer/interface/StreamDQMDeserializer.h"
#include "IOPool/Streamer/interface/StreamDQMSerializer.h"

#include "TROOT.h"


using namespace stor;


DQMEventRecord::DQMEventRecord
(
  DQMKey const dqmKey,
  DQMProcessingParams const dqmParams
) :
DQMInstance(
  dqmKey.runNumber, dqmKey.lumiSection, dqmKey.updateNumber, 
  static_cast<int>(dqmParams._purgeTimeDQM),
  static_cast<int>(dqmParams._readyTimeDQM)
),
_dqmParams(dqmParams)
{
  gROOT->SetBatch(kTRUE);
}

void DQMEventRecord::addDQMEventView(DQMEventMsgView const& view)
{
  _releaseTag = view.releaseTag();

  edm::StreamDQMDeserializer deserializer;
  std::auto_ptr<DQMEvent::TObjectTable> toTablePtr =
    deserializer.deserializeDQMEvent(view);

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
        view.topFolderName(),
        subFolderName,
        object,
        view.eventNumberAtUpdate()
      );
      delete(object);
    }
  }
}


DQMEventRecord::GroupRecord DQMEventRecord::populateAndGetGroup(const std::string groupName)
{
  DQMEventRecord::GroupRecord groupRecord;
  groupRecord._entry->dqmConsumers = _dqmConsumers;

  // Package list of TObjects into a DQMEvent::TObjectTable
  DQMEvent::TObjectTable table;
  DQMGroup *group = getDQMGroup(groupName);

  int subFolderSize = 0;
  for ( std::map<std::string, DQMFolder *>::iterator i1 = 
          group->dqmFolders_.begin(); i1 != group->dqmFolders_.end(); ++i1)
  {
    std::string folderName = i1->first;
    DQMFolder * folder = i1->second;
    for ( std::map<std::string, TObject *>::iterator i2 = 
            folder->dqmObjects_.begin(); i2!=folder->dqmObjects_.end(); ++i2)
    {
      std::string objectName = i2->first;
      TObject *object = i2->second;
      if ( object != NULL ) 
      { 
        if ( table.count(folderName) == 0 )
        {
          std::vector<TObject *> newObjectVector;
          table[folderName] = newObjectVector;
          subFolderSize += 2*sizeof(uint32) + folderName.length();
        }
        std::vector<TObject *> &objectVector = table[folderName];
        objectVector.push_back(object);
      }
    }
  }
  
  edm::StreamDQMSerializer serializer;
  serializer.serializeDQMEvent(table,
    _dqmParams._useCompressionDQM,
    _dqmParams._compressionLevelDQM);

  // Add space for header
  unsigned int sourceSize = serializer.currentSpaceUsed();
  unsigned int totalSize  = sourceSize 
    + sizeof(DQMEventHeader)
    + 12*sizeof(uint32)
    + _releaseTag.length()
    + groupName.length()
    + subFolderSize;
  groupRecord._entry->buffer.resize(totalSize);
  
  edm::Timestamp zeit( ( (unsigned long long)group->getLastUpdate()->GetSec() << 32 ) |
    ( group->getLastUpdate()->GetNanoSec()));
  
  DQMEventMsgBuilder builder(
    (void *)&(groupRecord._entry->buffer[0]), 
    totalSize,
    getRunNumber(),
    group->getLastEvent(),
    zeit,
    getLumiSection(),
    getInstance(),
    _releaseTag,
    groupName,
    table
  ); 
  unsigned char * source = serializer.bufferPointer();
  std::copy(source,source+sourceSize, builder.eventAddress());
  builder.setEventLength(sourceSize);
  if ( _dqmParams._useCompressionDQM ) 
  {
    builder.setCompressionFlag(serializer.currentEventSize());
  }

  return groupRecord;
}

/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
