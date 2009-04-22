// $Id: DQMEventStore.cc,v 1.1.2.3 2009/04/21 10:23:40 mommsen Exp $

#include "TTimeStamp.h"

#include "EventFilter/StorageManager/interface/DQMEventStore.h"
#include "EventFilter/StorageManager/interface/Utils.h"

using namespace stor;


DQMEventStore::DQMEventStore(DQMProcessingParams const& dqmParams) :
_dqmParams(dqmParams)
{}


void DQMEventStore::addDQMEvent(I2OChain& dqmEvent)
{
  if ( _dqmParams._collateDQM )
    addDQMEventToStore(dqmEvent);
  else
    addDQMEventToReadyToServe(dqmEvent);

  // At this point all required data has been copied.
  // Thus release the I2O buffers.
  dqmEvent.release();
}


bool DQMEventStore::getCompletedDQMEventRecordIfAvailable(DQMEventRecord::Entry& entry)
{
  if ( _recordsReadyToServe.empty() ) return false;

  entry = _recordsReadyToServe.top();
  _recordsReadyToServe.pop();

  return true;
}


void DQMEventStore::addDQMEventToStore(I2OChain const& dqmEvent)
{
  const DQMKey newKey = dqmEvent.dqmKey();

  // Use efficientAddOrUpdates pattern suggested by Item 24 of 
  // 'Effective STL' by Scott Meyers
  DQMEventRecordMap::iterator pos = _store.lower_bound(newKey);

  if(pos != _store.end() && !(_store.key_comp()(newKey, pos->first)))
  {
    // key already exists
    pos->second->addDQMEventView( getDQMEventView(dqmEvent) );
  }
  else
  {
    // Use pos as a hint to insert a new record, so it can avoid another lookup
    DQMEventRecordPtr record = makeDQMEventRecord(dqmEvent);
    _store.insert(pos, DQMEventRecordMap::value_type(newKey, record));
    
    // At this point, purge old instances from the list
    // writeAndPurgeDQMInstances(false);
  }

  addNextAvailableDQMGroupToReadyToServe( dqmEvent.topFolderName() );
}


void DQMEventStore::addDQMEventToReadyToServe(I2OChain const& dqmEvent)
{
  DQMEventRecordPtr record =
    makeDQMEventRecord(dqmEvent);

  _recordsReadyToServe.push( record->getEntry( dqmEvent.topFolderName() ) );
}


void DQMEventStore::addNextAvailableDQMGroupToReadyToServe(const std::string groupName)
{
  DQMEventRecordPtr record = getNewestReadyDQMEventRecord(groupName);
  
  if ( record )
  {
    _recordsReadyToServe.push( record->getEntry(groupName) );
  }
}


DQMEventRecordPtr
DQMEventStore::makeDQMEventRecord(I2OChain const& dqmEvent)
{
  DQMEventRecordPtr record(
    new DQMEventRecord(dqmEvent.dqmKey(), _dqmParams) 
  );
  record->setEventConsumerTags( dqmEvent.getDQMEventConsumerTags() );
  record->addDQMEventView( getDQMEventView(dqmEvent) );

  return record;
}


boost::shared_ptr<DQMEventMsgView>
DQMEventStore::getDQMEventView(I2OChain const& dqmEvent)
{
  dqmEvent.copyFragmentsIntoBuffer(_tempEventArea);
  boost::shared_ptr<DQMEventMsgView>
    view( new DQMEventMsgView(&_tempEventArea[0]) );
  return view;
}



DQMEventRecordPtr
DQMEventStore::getNewestReadyDQMEventRecord(const std::string groupName)
{
  DQMEventRecordPtr readyRecord;
  TTimeStamp now;
  now.Set();
  int maxTime(0);

  for (DQMEventRecordMap::const_iterator it = _store.begin(),
         itEnd = _store.end();
       it != itEnd;
       ++it)
  {
    DQMGroup *group = it->second->getDQMGroup(groupName);
    if ( group && group->isReady( now.GetSec() ) )
    {
      TTimeStamp *groupTime = group->getLastUpdate();
      if ( ( groupTime->GetSec() > maxTime ) &&
        ( ! group->wasServedSinceUpdate() ) )
      {
        maxTime = groupTime->GetSec();
        readyRecord = it->second;
      }
    }
  }
  readyRecord->getDQMGroup(groupName)->setServedSinceUpdate();
  
  return readyRecord;  
}


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
