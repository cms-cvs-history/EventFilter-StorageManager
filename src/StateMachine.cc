#include "EventFilter/StorageManager/interface/StateMachine.h"

#include <typeinfo>

using namespace stor;
using namespace std;

// void StateMachine::handleI2OEventMessage()
// {
//   const Operations& ref = state_cast<Operations const&>();
//   ref.handleI2OEventMessage();
// }

StateMachine::StateMachine
( 
  DiskWriter* dw,
  EventDistributor* ed,
  FragmentStore* fs,
  SharedResources* sr
):
_diskWriter(dw),
_eventDistributor(ed),
_fragmentStore(fs),
_sharedResources(sr)
{
}

Operations const&
StateMachine::getCurrentState() const
{
  return state_cast<Operations const&>();
}

string StateMachine::getCurrentStateName() const
{
  return getCurrentState().stateName();
}

void StateMachine::updateHistory( const TransitionRecord& tr )
{
  _history.push_back( tr );
}

void StateMachine::dumpHistory( ostream& os ) const
{

  cout << "**** Begin transition history ****" << endl;

  for( StateMachine::History::const_iterator j = _history.begin();
       j != _history.end(); ++j )
    {
      os << "  " << *j << endl;
    }

  cout << "**** End transition history ****" << endl;

}

void StateMachine::unconsumed_event( bsc::event_base const &event)
{
  std::cerr << "The " << 
    //event.dynamic_type()
    typeid(event).name()
    << " event is not supported from the "
    << getCurrentStateName() << " state!" << std::endl;
}



/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
