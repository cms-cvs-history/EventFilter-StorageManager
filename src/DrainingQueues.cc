#include "EventFilter/StorageManager/interface/DiskWriter.h"
#include "EventFilter/StorageManager/interface/EventDistributor.h"
#include "EventFilter/StorageManager/interface/FragmentStore.h"
#include "EventFilter/StorageManager/interface/I2OChain.h"
#include "EventFilter/StorageManager/interface/SharedResources.h"
#include "EventFilter/StorageManager/interface/StateMachine.h"

#include <iostream>
#include <unistd.h>

using namespace std;
using namespace stor;

DrainingQueues::DrainingQueues( my_context c ): my_base(c)
{
  TransitionRecord tr( stateName(), true );
  outermost_context().updateHistory( tr );
}

DrainingQueues::~DrainingQueues()
{
  TransitionRecord tr( stateName(), false );
  outermost_context().updateHistory( tr );
}

string DrainingQueues::do_stateName() const
{
  return string( "DrainingQueues" );
}

void
DrainingQueues::do_noFragmentToProcess() const
{
  //std::cout << stateName() << "::noFragmentToProcess()" << std::endl;
  if ( allQueuesAndWorkersAreEmpty() )
    {
      SharedResourcesPtr sharedRes = outermost_context().getSharedResources();
      boost::shared_ptr<CommandQueue> commandQueue =
        sharedRes->_commandQueue;
      event_ptr stMachEvent( new StopDone() );
      // do we really want enq_wait here?
      // it could cause deadlock if the command queue is full...
      commandQueue->enq_wait( stMachEvent );
    }
}

bool
DrainingQueues::allQueuesAndWorkersAreEmpty() const
{
  SharedResourcesPtr sharedResources = outermost_context().getSharedResources();

  // the order is important here - upstream entities first,
  // followed by more downstream entities

  EventDistributor *ed = outermost_context().getEventDistributor();
  if ( ed->full() ) return false;

  processStaleFragments();
  FragmentStore *fs = outermost_context().getFragmentStore();
  if ( ! fs->empty() ) return false;

  if ( ! sharedResources->_streamQueue->empty() ) return false;

  if ( ! sharedResources->_diskWriter->empty() ) return false;
  
  //  if ( ! sharedResources->_dqmEventQueue->empty() ) return false;

  //DQMEventProcessor *dqmEP = outermost_context.getDQMEventProcessor();
  //if ( ! dqmEP->empty() ) return false;

  return true;
}

void
DrainingQueues::processStaleFragments() const
{
  I2OChain staleEvent;
  bool gotStaleEvent = true;  
  int loopCounter = 0;

  EventDistributor *ed = outermost_context().getEventDistributor();

  while ( gotStaleEvent && !ed->full() && loopCounter++ < 10 )
  {
    gotStaleEvent = 
      outermost_context().getFragmentStore()->getStaleEvent(staleEvent, 0);
    if ( gotStaleEvent )
    {
      outermost_context().getSharedResources()->_discardManager->sendDiscardMessage(staleEvent);
      ed->addEventToRelevantQueues(staleEvent);
    }
  }
}


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
