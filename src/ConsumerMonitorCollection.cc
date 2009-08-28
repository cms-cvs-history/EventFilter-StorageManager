// $Id: ConsumerMonitorCollection.cc,v 1.6 2009/08/24 15:07:40 mommsen Exp $
/// @file: ConsumerMonitorCollection.cc

#include "EventFilter/StorageManager/interface/ConsumerMonitorCollection.h"
#include "EventFilter/StorageManager/interface/QueueID.h"
#include "EventFilter/StorageManager/interface/MonitoredQuantity.h"

using namespace stor;


ConsumerMonitorCollection::ConsumerMonitorCollection(const utils::duration_t& updateInterval):
MonitorCollection(updateInterval),
_updateInterval(updateInterval)
{}


void ConsumerMonitorCollection::addQueuedEventSample( const QueueID& qid,
						      const unsigned int& data_size )
{
  boost::mutex::scoped_lock l( _mutex );
  addEventSampleToMap(qid, data_size, _qmap);
}


void ConsumerMonitorCollection::addServedEventSample( const QueueID& qid,
						      const unsigned int& data_size )
{
  boost::mutex::scoped_lock l( _mutex );
  addEventSampleToMap(qid, data_size, _smap);
}


void ConsumerMonitorCollection::addEventSampleToMap( const QueueID& qid,
                                                     const unsigned int& data_size,
                                                     ConsStatMap& map )
{
  ConsStatMap::iterator pos = map.lower_bound(qid);

  if (pos == map.end())
  {
    // The key does not exist in the map, add it to the map
    // Use pos as a hint to insert, so it can avoid another lookup
    pos = map.insert(pos,
      ConsStatMap::value_type(qid, 
        boost::shared_ptr<MonitoredQuantity>(new MonitoredQuantity(_updateInterval,10))
      )
    );
  }
  pos->second->addSample( data_size );
}


bool ConsumerMonitorCollection::getQueued( const QueueID& qid,
					   MonitoredQuantity::Stats& result )
{
  boost::mutex::scoped_lock l( _mutex );
  return getValueFromMap( qid, result, _qmap );
}


bool ConsumerMonitorCollection::getServed( const QueueID& qid,
					   MonitoredQuantity::Stats& result )
{
  boost::mutex::scoped_lock l( _mutex );
  return getValueFromMap( qid, result, _smap );
}


bool ConsumerMonitorCollection::getValueFromMap( const QueueID& qid,
                                                 MonitoredQuantity::Stats& result,
                                                 const ConsStatMap& map )
{
  ConsStatMap::const_iterator pos = map.lower_bound(qid);

  if (pos == map.end()) return false;

  pos->second->getStats( result );
  return true;
}


void ConsumerMonitorCollection::resetCounters()
{
  boost::mutex::scoped_lock l( _mutex );
  for( ConsStatMap::iterator i = _qmap.begin(); i != _qmap.end(); ++i )
    i->second->reset();
  for( ConsStatMap::iterator i = _smap.begin(); i != _smap.end(); ++i )
    i->second->reset();
}


void ConsumerMonitorCollection::do_calculateStatistics()
{
  boost::mutex::scoped_lock l( _mutex );
  for( ConsStatMap::iterator i = _qmap.begin(); i != _qmap.end(); ++i )
    i->second->calculateStatistics();
  for( ConsStatMap::iterator i = _smap.begin(); i != _smap.end(); ++i )
    i->second->calculateStatistics();
}


void ConsumerMonitorCollection::do_reset()
{
  boost::mutex::scoped_lock l( _mutex );
  _qmap.clear();
  _smap.clear();
}


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
