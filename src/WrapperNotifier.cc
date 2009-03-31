// $Id: $

#include "EventFilter/StorageManager/interface/WrapperNotifier.h"

#include "xdata/InfoSpace.h"

using namespace stor;

WrapperNotifier::WrapperNotifier( xdaq2rc::RcmsStateNotifier& rcmsNotifier ):
  _rcms_notifier( rcmsNotifier ),
  _infospace(0)
{}

void WrapperNotifier::reportNewState( const std::string& stateName )
{
  _rcms_notifier.stateChanged( stateName,
			       std::string( "StorageManager is now " ) + stateName );
}

void WrapperNotifier::setupInfospace( xdata::InfoSpace* ispace )
{

  if( !ispace ) return;

  ispace->fireItemAvailable( "rcmsStateListener",
                             _rcms_notifier.getRcmsStateListenerParameter() );

  ispace->fireItemAvailable( "foundRcmsStateListener",
                             _rcms_notifier.getFoundRcmsStateListenerParameter() );

  _rcms_notifier.findRcmsStateListener();
  _rcms_notifier.subscribeToChangesInRcmsStateListener( ispace );

}
