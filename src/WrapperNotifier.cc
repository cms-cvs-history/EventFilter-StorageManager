// $Id: WrapperNotifier.cc,v 1.4 2009/07/20 13:07:28 mommsen Exp $
/// @file: WrapperNotifier.cc

#include "EventFilter/StorageManager/interface/WrapperNotifier.h"

#include "xdata/InfoSpace.h"


using namespace stor;

WrapperNotifier::WrapperNotifier( xdaq::Application* app ):
  rcms_notifier_( xdaq2rc::RcmsStateNotifier( app->getApplicationLogger(),
                                              app->getApplicationDescriptor(),
                                              app->getApplicationContext() )
                  ),
  app_( app )
{
  xdata::InfoSpace *ispace = app->getApplicationInfoSpace();
  
  ispace->fireItemAvailable( "rcmsStateListener",
    rcms_notifier_.getRcmsStateListenerParameter() );
  ispace->fireItemAvailable( "foundRcmsStateListener",
    rcms_notifier_.getFoundRcmsStateListenerParameter() );
  rcms_notifier_.findRcmsStateListener();
  rcms_notifier_.subscribeToChangesInRcmsStateListener( ispace );
}


void WrapperNotifier::reportNewState( const std::string& stateName )
{
  rcms_notifier_.stateChanged( stateName,
			       std::string( "StorageManager is now " ) + stateName );
}


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
