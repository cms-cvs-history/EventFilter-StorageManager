// $Id: DQMTopLevelFolder.cc,v 1.1.2.1 2011/02/23 09:26:39 mommsen Exp $
/// @file: DQMTopLevelFolder.cc

#include "EventFilter/StorageManager/interface/DQMEventMonitorCollection.h"
#include "EventFilter/StorageManager/interface/DQMTopLevelFolder.h"
#include "EventFilter/StorageManager/interface/QueueID.h"
#include "EventFilter/StorageManager/interface/SharedResources.h"

#include "IOPool/Streamer/interface/DQMEventMessage.h"
#include "IOPool/Streamer/interface/StreamDQMDeserializer.h"
#include "IOPool/Streamer/interface/StreamDQMSerializer.h"

#include "TROOT.h"

#include "toolbox/net/Utils.h"

#include <sstream>
#include <unistd.h>

using namespace stor;


DQMTopLevelFolder::DQMTopLevelFolder
(
  const I2OChain& dqmEvent,
  const DQMProcessingParams& dqmParams,
  DQMEventMonitorCollection& dqmEventMonColl,
  const unsigned int expectedUpdates
) :
_dqmParams(dqmParams),
_dqmEventMonColl(dqmEventMonColl),
_dqmConsumers(dqmEvent.getDQMEventConsumerTags()),
_dqmKey(dqmEvent.dqmKey()),
_expectedUpdates(expectedUpdates),
_nUpdates(0),
_sentEvents(0)
{
  gROOT->SetBatch(kTRUE);
  addDQMEvent(dqmEvent);
}


DQMTopLevelFolder::~DQMTopLevelFolder()
{
  _dqmFolders.clear();
  _tempEventArea.clear();
}


void DQMTopLevelFolder::addDQMEvent(const I2OChain& dqmEvent)
{
  dqmEvent.copyFragmentsIntoBuffer(_tempEventArea);
  DQMEventMsgView view(&_tempEventArea[0]);
  addDQMEvent(view);
}


void DQMTopLevelFolder::addDQMEvent(const DQMEventMsgView& view)
{
  _releaseTag = view.releaseTag();

  edm::StreamDQMDeserializer deserializer;
  std::auto_ptr<DQMEvent::TObjectTable> toTablePtr =
    deserializer.deserializeDQMEvent(view);

  addEvent(toTablePtr);

  ++_nUpdates;
  _lastUpdate = utils::getCurrentTime();

  _dqmEventMonColl.getDQMEventSizeMQ().addSample(
    static_cast<double>(view.size()) / 0x100000
  );
}


bool DQMTopLevelFolder::isReady(const utils::time_point_t& now) const
{
  if ( _nUpdates == 0 ) return false;

  if ( _nUpdates == _expectedUpdates ) return true;

  if ( now > _lastUpdate + _dqmParams._readyTimeDQM ) return true;
  
  return false;
}

void DQMTopLevelFolder::addEvent(std::auto_ptr<DQMEvent::TObjectTable> toTablePtr)
{
  for (
    DQMEvent::TObjectTable::const_iterator it = toTablePtr->begin(),
      itEnd = toTablePtr->end();
    it != itEnd; 
    ++it
  ) 
  {
    const std::string folderName = it->first;

    DQMFoldersMap::iterator pos = _dqmFolders.lower_bound(folderName);
    if ( pos == _dqmFolders.end() || (_dqmFolders.key_comp()(folderName, pos->first)) )
    {
      pos = _dqmFolders.insert(pos, DQMFoldersMap::value_type(
          folderName, DQMFolderPtr( new DQMFolder() )
        ));
    }
    pos->second->addObjects(it->second);
  }
}


bool DQMTopLevelFolder::getRecord(DQMTopLevelFolder::Record& record)
{
  if ( _nUpdates == 0 ) return false;

  record.clear();
  record.tagForEventConsumers(_dqmConsumers);

  // Package list of TObjects into a DQMEvent::TObjectTable
  DQMEvent::TObjectTable table;
  const size_t folderSize = populateTable(table);
  
  edm::StreamDQMSerializer serializer;
  serializer.serializeDQMEvent(table,
    _dqmParams._useCompressionDQM,
    _dqmParams._compressionLevelDQM);

  // Add space for header
  const size_t sourceSize = serializer.currentSpaceUsed();
  const size_t totalSize =
    sourceSize
    + sizeof(DQMEventHeader)
    + 12*sizeof(uint32_t)
    + _releaseTag.length()
    + _dqmKey.topLevelFolderName.length()
    + folderSize;
  
  edm::Timestamp timestamp(utils::nanoseconds_since_epoch(_lastUpdate));
  
  DQMEventMsgBuilder builder(
    record.getBuffer(totalSize),
    totalSize,
    _dqmKey.runNumber,
    ++_sentEvents,
    timestamp,
    _dqmKey.lumiSection,
    _dqmKey.lumiSection,
    (uint32_t)serializer.adler32_chksum(),
    toolbox::net::getHostName().c_str(),
    _releaseTag,
    _dqmKey.topLevelFolderName,
    table
  ); 
  unsigned char* source = serializer.bufferPointer();
  std::copy(source,source+sourceSize, builder.eventAddress());
  builder.setEventLength(sourceSize);
  if ( _dqmParams._useCompressionDQM ) 
  {
    builder.setCompressionFlag(serializer.currentEventSize());
  }

  _dqmEventMonColl.getNumberOfUpdatesMQ().addSample(_nUpdates);
  _dqmEventMonColl.getServedDQMEventSizeMQ().addSample(
    static_cast<double>(record.totalDataSize()) / 0x100000
  );

  return true;
}


size_t DQMTopLevelFolder::populateTable(DQMEvent::TObjectTable& table) const
{
  size_t folderSize = 0;

  for ( DQMFoldersMap::const_iterator it = _dqmFolders.begin(), itEnd = _dqmFolders.end();
        it != itEnd; ++it )
  {
    const std::string folderName = it->first;
    const DQMFolderPtr folder = it->second;

    DQMEvent::TObjectTable::iterator pos = table.lower_bound(folderName);
    if ( pos == table.end() || (table.key_comp()(folderName, pos->first)) )
    {
      std::vector<TObject*> newObjectVector;
      pos = table.insert(pos, DQMEvent::TObjectTable::value_type(folderName, newObjectVector));
      folderSize += 2*sizeof(uint32_t) + folderName.length();
    }
    folder->fillObjectVector(pos->second);
  }
  return folderSize;
}



/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
