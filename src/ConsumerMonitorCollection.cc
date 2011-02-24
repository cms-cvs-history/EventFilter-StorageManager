// $Id: ConsumerMonitorCollection.cc,v 1.11.2.5 2011/02/24 14:40:37 mommsen Exp $
/// @file: ConsumerMonitorCollection.cc

#include "EventFilter/StorageManager/interface/ConsumerMonitorCollection.h"
#include "EventFilter/StorageManager/interface/QueueID.h"
#include "EventFilter/StorageManager/interface/MonitoredQuantity.h"

using namespace stor;


ConsumerMonitorCollection::ConsumerMonitorCollection
(
  const utils::duration_t& updateInterval,
  const utils::duration_t& recentDuration
):
MonitorCollection(updateInterval),
_updateInterval(updateInterval),
_recentDuration(recentDuration),
_totalQueuedMQ(updateInterval, recentDuration),
_totalDroppedMQ(updateInterval, recentDuration),
_totalServedMQ(updateInterval, recentDuration)
{}


void ConsumerMonitorCollection::addQueuedEventSample( const QueueID& qid,
						      const unsigned int& data_size )
{
  boost::mutex::scoped_lock l( _mutex );
  addEventSampleToMap(qid, data_size, _qmap);
  _totalQueuedMQ.addSample(data_size);
}


void ConsumerMonitorCollection::addDroppedEvents( const QueueID& qid,
						  const size_t& count )
{
  boost::mutex::scoped_lock l( _mutex );
  addEventSampleToMap(qid, count, _dmap);
  _totalDroppedMQ.addSample(count);
}


void ConsumerMonitorCollection::addServedEventSample( const QueueID& qid,
						      const unsigned int& data_size )
{
  boost::mutex::scoped_lock l( _mutex );
  addEventSampleToMap(qid, data_size, _smap);
  _totalServedMQ.addSample(data_size);
}


void ConsumerMonitorCollection::addEventSampleToMap( const QueueID& qid,
                                                     const unsigned int& data_size,
                                                     ConsStatMap& map )
{
  ConsStatMap::iterator pos = map.lower_bound(qid);

  // 05-Oct-2009, KAB - added a test of whether qid appears before pos->first
  // in the map sort order.  Since lower_bound can return a non-end iterator
  // even when qid is not in the map, we need to complete the test of whether
  // qid is in the map.  (Another way to look at this is we need to implement
  // the full test described in the efficientAddOrUpdates pattern suggested
  // by Item 24 of 'Effective STL' by Scott Meyers.)
  if (pos == map.end() || (map.key_comp()(qid, pos->first)))
  {
    // The key does not exist in the map, add it to the map
    // Use pos as a hint to insert, so it can avoid another lookup
    pos = map.insert(pos,
      ConsStatMap::value_type(qid, 
        MonitoredQuantityPtr(
          new MonitoredQuantity(_updateInterval, _recentDuration)
        )
      )
    );
  }

  pos->second->addSample( data_size );
}


bool ConsumerMonitorCollection::getQueued( const QueueID& qid,
					   MonitoredQuantity::Stats& result ) const
{
  boost::mutex::scoped_lock l( _mutex );
  return getValueFromMap( qid, result, _qmap );
}


bool ConsumerMonitorCollection::getServed( const QueueID& qid,
					   MonitoredQuantity::Stats& result ) const
{
  boost::mutex::scoped_lock l( _mutex );
  return getValueFromMap( qid, result, _smap );
}


bool ConsumerMonitorCollection::getDropped( const QueueID& qid,
					    MonitoredQuantity::Stats& result ) const
{
  boost::mutex::scoped_lock l( _mutex );
  return getValueFromMap( qid, result, _dmap );
}

bool ConsumerMonitorCollection::getValueFromMap( const QueueID& qid,
                                                 MonitoredQuantity::Stats& result,
                                                 const ConsStatMap& map ) const
{
  ConsStatMap::const_iterator pos = map.find(qid);

  if (pos == map.end()) return false;

  pos->second->getStats( result );
  return true;
}


void ConsumerMonitorCollection::getTotalStats( TotalStats& totalStats ) const
{
  _totalQueuedMQ.getStats(totalStats.queuedStats);
  _totalDroppedMQ.getStats(totalStats.droppedStats);
  _totalServedMQ.getStats(totalStats.servedStats);
}

void ConsumerMonitorCollection::resetCounters()
{
  boost::mutex::scoped_lock l( _mutex );
  for( ConsStatMap::iterator i = _qmap.begin(); i != _qmap.end(); ++i )
    i->second->reset();
  for( ConsStatMap::iterator i = _smap.begin(); i != _smap.end(); ++i )
    i->second->reset();
  for( ConsStatMap::iterator i = _dmap.begin(); i != _dmap.end(); ++i )
    i->second->reset();

  _totalQueuedMQ.reset();
  _totalDroppedMQ.reset();
  _totalServedMQ.reset();
}


void ConsumerMonitorCollection::do_calculateStatistics()
{
  boost::mutex::scoped_lock l( _mutex );
  for( ConsStatMap::iterator i = _qmap.begin(); i != _qmap.end(); ++i )
    i->second->calculateStatistics();
  for( ConsStatMap::iterator i = _smap.begin(); i != _smap.end(); ++i )
    i->second->calculateStatistics();
  for( ConsStatMap::iterator i = _dmap.begin(); i != _dmap.end(); ++i )
    i->second->calculateStatistics();

  _totalQueuedMQ.calculateStatistics();
  _totalDroppedMQ.calculateStatistics();
  _totalServedMQ.calculateStatistics();
}


void ConsumerMonitorCollection::do_reset()
{
  boost::mutex::scoped_lock l( _mutex );
  _qmap.clear();
  _smap.clear();
  _dmap.clear();

  _totalQueuedMQ.reset();
  _totalDroppedMQ.reset();
  _totalServedMQ.reset();
}


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
