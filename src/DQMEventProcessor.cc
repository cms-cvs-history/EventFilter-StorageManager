// $Id: DQMEventProcessor.cc,v 1.1.2.3 2009/03/12 03:46:18 paterno Exp $

#include "EventFilter/StorageManager/interface/DQMEventProcessor.h"

using namespace stor;


DQMEventProcessor::DQMEventProcessor()
{

}


DQMEventProcessor::~DQMEventProcessor()
{

}


void DQMEventProcessor::processNextDQMEvent()
{

}


QueueID DQMEventProcessor::registerDQMEventConsumer
(
  DQMEventConsumerRegistrationInfo const& ri
)
{
  return _dqmEventConsumerQueueCollection.createQueue(ri.consumerID(),
                                                      ri.queuePolicy(),
                                                      ri.maxQueueSize());
}


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
