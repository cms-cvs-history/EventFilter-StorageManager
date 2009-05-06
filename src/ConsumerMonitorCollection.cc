// $Id: ConsumerMonitorCollection.cc,v 1.1.2.2 2009/04/29 11:51:26 dshpakov Exp $

#include "EventFilter/StorageManager/interface/ConsumerMonitorCollection.h"

using namespace stor;


void ConsumerMonitorCollection::addQueuedEventSample( ConsumerID cid,
						      unsigned int data_size )
{
  boost::mutex::scoped_lock l( _mutex );
  if( _qmap.find( cid ) != _qmap.end() )
    {
      _qmap[ cid ]->addSample( data_size );
    }
  else
    {
      _qmap[ cid ] = boost::shared_ptr<MonitoredQuantity>( new MonitoredQuantity() );
      _qmap[ cid ]->addSample( data_size );
    }
}


void ConsumerMonitorCollection::addServedEventSample( ConsumerID cid,
						      unsigned int data_size )
{
  boost::mutex::scoped_lock l( _mutex );
  if( _smap.find( cid ) != _smap.end() )
    {
      _smap[ cid ]->addSample( data_size );
    }
  else
    {
      _smap[ cid ] = boost::shared_ptr<MonitoredQuantity>( new MonitoredQuantity() );
      _smap[ cid ]->addSample( data_size );
    }
}


bool ConsumerMonitorCollection::getQueued( ConsumerID cid,
					   MonitoredQuantity::Stats& result )
{
  boost::mutex::scoped_lock l( _mutex );
  if( _qmap.find( cid ) == _qmap.end() ) return false;
  _qmap[ cid ]->getStats( result );
  return true;
}


bool ConsumerMonitorCollection::getServed( ConsumerID cid,
					   MonitoredQuantity::Stats& result )
{
  boost::mutex::scoped_lock l( _mutex );
  if( _smap.find( cid ) == _smap.end() ) return false;
  _smap[ cid ]->getStats( result );
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


void ConsumerMonitorCollection::clearConsumers()
{
  boost::mutex::scoped_lock l( _mutex );
  _qmap.clear();
  _smap.clear();
}
