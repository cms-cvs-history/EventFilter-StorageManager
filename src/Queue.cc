// $Id: Queue.cc,v 1.1.2.1 2009/01/19 18:14:06 mommsen Exp $

#include "EventFilter/StorageManager/interface/Queue.h"


stor::Queue::Queue()
{

}


stor::Queue::~Queue()
{

}


void stor::Queue::addEvent(I2OChain &event)
{
  boost::mutex::scoped_lock lock(_mutex);
  _queue.push(event);
  lock.unlock();
}


stor::I2OChain stor::Queue::popEvent()
{
  stor::I2OChain chain;

  boost::mutex::scoped_lock lock(_mutex);
  if ( ! _queue.empty() )
  {
    chain = _queue.front();
    _queue.pop();
  }
  lock.unlock();

  return chain;
}


bool stor::Queue::empty()
{
  bool empty;

  boost::mutex::scoped_lock lock(_mutex);
  empty = _queue.empty();
  lock.unlock();

  return empty;
}


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
