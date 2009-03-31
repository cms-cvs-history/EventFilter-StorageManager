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
  EventDistributor* ed,
  FragmentStore* fs,
  Notifier* n,
  SharedResourcesPtr sr
):
_eventDistributor(ed),
_fragmentStore(fs),
_notifier(n),
_sharedResources(sr),
_initialized( false )
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

void StateMachine::setExternallyVisibleState( const std::string& s )
{
  if( _initialized )
    {
      if( _sharedResources->_statisticsReporter.get() != 0 )
        {
          _sharedResources->_statisticsReporter->setExternallyVisibleState( s );
        }
    }
}

/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
