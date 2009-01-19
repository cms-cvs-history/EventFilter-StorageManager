// $Id$

#include "EventFilter/StorageManager/interface/Queue.h"

std::queue<stor::Chain> stor::Queue::_queue;


stor::Queue::Queue()
{

}


stor::Queue::~Queue()
{

}


void stor::Queue::addEvent(Chain &event)
{
  boost::mutex::scoped_lock lock(_mutex);
  _queue.push(event);
  lock.unlock();
}


stor::Chain stor::Queue::popEvent()
{
  stor::Chain chain;
  if ( ! _queue.empty() )
  {
    chain = _queue.front();
  }

  return chain;
}



/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
