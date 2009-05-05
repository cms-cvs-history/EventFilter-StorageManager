// $Id: ConsumerMonitorCollection.cc,v 1.1.2.1 2009/04/28 14:39:50 dshpakov Exp $

#include "EventFilter/StorageManager/interface/ConsumerMonitorCollection.h"

using namespace stor;


void ConsumerMonitorCollection::addQueuedEventSample( ConsumerID cid,
						      unsigned int data_size )
{
  boost::mutex::scoped_lock l( _mutex );
  if( _qmap.find( cid ) == _qmap.end() )
    {
      _qmap[ cid ] = data_size;
    }
  else
    {
      _qmap[ cid ] += data_size;
    }
}


void ConsumerMonitorCollection::addServedEventSample( ConsumerID cid,
						      unsigned int data_size )
{
  boost::mutex::scoped_lock l( _mutex );
  if( _smap.find( cid ) == _smap.end() )
    {
      _smap[ cid ] = data_size;
    }
  else
    {
      _smap[ cid ] += data_size;
    }
}


bool ConsumerMonitorCollection::getQueued( ConsumerID cid,
					   unsigned int& result )
{
  boost::mutex::scoped_lock l( _mutex );
  if( _qmap.find( cid ) == _qmap.end() ) return false;
  result = _qmap[ cid ];
  return true;
}


bool ConsumerMonitorCollection::getServed( ConsumerID cid,
					   unsigned int& result )
{
  boost::mutex::scoped_lock l( _mutex );
  if( _smap.find( cid ) == _smap.end() ) return false;
  result = _smap[ cid ];
  return true;
}


void ConsumerMonitorCollection::resetCounters()
{
  boost::mutex::scoped_lock l( _mutex );
  for( ConsStatMap::iterator i = _qmap.begin(); i != _qmap.end(); ++i )
    i->second = 0;
  for( ConsStatMap::iterator i = _smap.begin(); i != _smap.end(); ++i )
    i->second = 0;
}


void ConsumerMonitorCollection::clearConsumers()
{
  boost::mutex::scoped_lock l( _mutex );
  _qmap.clear();
  _smap.clear();
}