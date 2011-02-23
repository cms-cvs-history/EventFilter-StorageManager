// $Id: DQMEventStore.cc,v 1.12.2.1 2011/01/24 12:18:39 mommsen Exp $
/// @file: DQMEventStore.cc

#include "TROOT.h"
#include "TTimeStamp.h"

#include "EventFilter/StorageManager/interface/DQMEventMonitorCollection.h"
#include "EventFilter/StorageManager/interface/DQMEventStore.h"
#include "EventFilter/StorageManager/interface/I2OChain.h"
#include "EventFilter/StorageManager/interface/QueueID.h"
#include "EventFilter/StorageManager/interface/StatisticsReporter.h"
#include "EventFilter/StorageManager/interface/Utils.h"

using namespace stor;


DQMEventStore::DQMEventStore(SharedResourcesPtr sr) :
_dqmEventMonColl(sr->_statisticsReporter->getDQMEventMonitorCollection()),
_initMsgColl(sr->_initMsgCollection)
{
  gROOT->SetBatch(kTRUE);
}


DQMEventStore::~DQMEventStore()
{
  clear();
}


void
DQMEventStore::clear()
{
  _store.clear();
  while ( ! _topLevelFoldersReadyToServe.empty() )
    _topLevelFoldersReadyToServe.pop();
}


void
DQMEventStore::purge()
{
  addReadyTopLevelFoldersToReadyToServe();
  _store.clear();
}


void
DQMEventStore::setParameters(DQMProcessingParams const& dqmParams)
{
  clear();
  _dqmParams = dqmParams;
}

void
DQMEventStore::addDQMEvent(I2OChain const& dqmEvent)
{
  if ( _dqmParams._collateDQM )
    addDQMEventToStore(dqmEvent);
  else
    addDQMEventToReadyToServe(dqmEvent);
}

bool
DQMEventStore::getCompletedTopLevelFolderIfAvailable
(
  DQMTopLevelFolder::Record& record
)
{
  if ( _topLevelFoldersReadyToServe.empty() ) return false;

  record = _topLevelFoldersReadyToServe.front();
  _topLevelFoldersReadyToServe.pop();

  return true;
}

void
DQMEventStore::addDQMEventToStore(I2OChain const& dqmEvent)
{
  const DQMKey newKey = dqmEvent.dqmKey();

  // Use efficientAddOrUpdates pattern suggested by Item 24 of 

  // 'Effective STL' by Scott Meyers
  DQMTopLevelFolderMap::iterator pos = _store.lower_bound(newKey);

  if(pos != _store.end() && !(_store.key_comp()(newKey, pos->first)))
  {
    // key already exists
    pos->second->addDQMEvent(dqmEvent);
  }
  else
  {
    // Use pos as a hint to insert a new record, so it can avoid another lookup
    DQMTopLevelFolderPtr topLevelFolder( new DQMTopLevelFolder(
        dqmEvent, _dqmParams, _dqmEventMonColl, _initMsgColl->maxMsgCount()
      ));
    _store.insert(pos, DQMTopLevelFolderMap::value_type(newKey, topLevelFolder));
  }

  addReadyTopLevelFoldersToReadyToServe();
}


void
DQMEventStore::addDQMEventToReadyToServe(I2OChain const& dqmEvent)
{
  DQMTopLevelFolderPtr topLevelFolder( new DQMTopLevelFolder(
      dqmEvent, _dqmParams, _dqmEventMonColl, _initMsgColl->maxMsgCount()
    ));
  addTopLevelFolderToReadyToServe(topLevelFolder);
}


void
DQMEventStore::addReadyTopLevelFoldersToReadyToServe()
{
  DQMTopLevelFolderPtr topLevelFolder;

  while ( getNextReadyTopLevelFolder(topLevelFolder) )
  {
    addTopLevelFolderToReadyToServe(topLevelFolder);
  }
}


void
DQMEventStore::addTopLevelFolderToReadyToServe(const DQMTopLevelFolderPtr topLevelFolder)
{
  DQMTopLevelFolder::Record record;
  topLevelFolder->getRecord(record);
  _topLevelFoldersReadyToServe.push(record);
}



bool
DQMEventStore::getNextReadyTopLevelFolder(DQMTopLevelFolderPtr& topLevelFolder)
{
  utils::time_point_t now = utils::getCurrentTime();

  DQMTopLevelFolderMap::iterator it = _store.begin();
  DQMTopLevelFolderMap::const_iterator itEnd = _store.end();
  while ( it !=  itEnd && ! it->second->isReady(now) ) ++it;
  
  if ( it == itEnd ) return false;

  topLevelFolder = it->second;
  _store.erase(it);
  return true;
}



/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
