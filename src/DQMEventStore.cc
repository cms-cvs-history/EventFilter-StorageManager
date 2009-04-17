// $Id: DQMEventStore.cc,v 1.1.2.1 2009/04/17 10:42:49 mommsen Exp $

#include "EventFilter/StorageManager/interface/DQMEventStore.h"
#include "EventFilter/StorageManager/interface/Utils.h"

using namespace stor;


DQMEventStore::DQMEventStore(DQMProcessingParams const& dqmParams) :
_dqmParams(dqmParams)
{}


void DQMEventStore::addDQMEvent(I2OChain& dqmEvent)
{

  const DQMKey newKey = dqmEvent.dqmKey();

  // Use efficientAddOrUpdates pattern suggested by Item 24 of 
  // 'Effective STL' by Scott Meyers
  DQMEventRecordMap::iterator pos = _store.lower_bound(newKey);

  if(pos != _store.end() && !(_store.key_comp()(newKey, pos->first)))
  {
    // key already exists
    pos->second.addDQMEventView( getDQMEventView(dqmEvent) );
  }
  else
  {
    // The key does not exist in the map, add it to the map
    // Use pos as a hint to insert, so it can avoid another lookup
    DQMEventRecord record = makeDQMEventRecord(dqmEvent);
    _store.insert(pos, DQMEventRecordMap::value_type(newKey, record));
  }

  // At this point all required data has been copied. Thus release
  // the I2O buffers.
  dqmEvent.release();
}



DQMEventRecord DQMEventStore::makeDQMEventRecord(I2OChain const& dqmEvent)
{
  DQMEventRecord record;
  record.setEventConsumerTags( dqmEvent.getDQMEventConsumerTags() );
  record.addDQMEventView( getDQMEventView(dqmEvent) );

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


bool DQMEventStore::getCompletedDQMEventRecordIfAvailable(DQMEventRecord& record)
{
  return false;
}



/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
