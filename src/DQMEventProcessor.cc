// $Id: DQMEventProcessor.cc,v 1.1.2.1 2009/01/19 18:14:06 mommsen Exp $

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


const QueueID DQMEventProcessor::registerDQMEventConsumer
(
  boost::shared_ptr<DQMEventConsumerRegistrationInfo> registrationInfo
)
{
  return _dqmEventConsumerQueueCollection.registerDQMEventConsumer(registrationInfo);
}


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
