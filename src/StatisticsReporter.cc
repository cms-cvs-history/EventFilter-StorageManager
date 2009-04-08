// $Id: StatisticsReporter.cc,v 1.1.2.12 2009/04/06 18:33:29 mommsen Exp $

#include <string>
#include <sstream>

#include "toolbox/task/Action.h"
#include "toolbox/task/WorkLoopFactory.h"
#include "xcept/tools.h"
#include "xdaq/ApplicationDescriptor.h"

#include "EventFilter/StorageManager/interface/Exception.h"
#include "EventFilter/StorageManager/interface/MonitoredQuantity.h"
#include "EventFilter/StorageManager/interface/StatisticsReporter.h"
#include "EventFilter/StorageManager/interface/Utils.h"

using namespace stor;


StatisticsReporter::StatisticsReporter(xdaq::Application *app) :
_app(app),
_runMonCollection(app),
_fragMonCollection(app),
_filesMonCollection(app),
_streamsMonCollection(app),
_doMonitoring(true),
_externallyVisibleState( "Halted" ),
_xdaq_state_name( "Halted" )
{
  xdata::InfoSpace* ispace = _app->getApplicationInfoSpace();

  try
  {
    ispace->fireItemAvailable( "stateName", &_xdaq_state_name );
  }
  catch(xdata::exception::Exception &e)
  {
    std::stringstream oss;
    
    oss << "Failed to put stateName into info space " << ispace->name();
    
    XCEPT_RETHROW(stor::exception::Monitoring, oss.str(), e);
  }
}


void StatisticsReporter::startWorkLoop()
{
  try
  {
    std::string identifier = utils::getIdentifier(_app->getApplicationDescriptor());

    _monitorWL=
      toolbox::task::getWorkLoopFactory()->getWorkLoop(
        identifier + "StatisticsReporter",
        "waiting");

    if ( ! _monitorWL->isActive() )
    {
      toolbox::task::ActionSignature* monitorAction = 
        toolbox::task::bind(this, &StatisticsReporter::monitorAction, 
          identifier + "MonitorAction");
      _monitorWL->submit(monitorAction);

      _monitorWL->activate();
    }
  }
  catch (xcept::Exception& e)
  {
    std::string msg = "Failed to start workloop 'StatisticsReporter' with 'MonitorAction'.";
    XCEPT_RETHROW(stor::exception::Monitoring, msg, e);
  }
}


StatisticsReporter::~StatisticsReporter()
{
  // Stop the monitoring activity
  _doMonitoring = false;

  // Cancel the workloop (will wait until the action has finished)
  _monitorWL->cancel();
}


bool StatisticsReporter::monitorAction(toolbox::task::WorkLoop* wl)
{
  utils::sleep(MonitoredQuantity::ExpectedCalculationInterval());

  std::string errorMsg = "Failed to update the monitoring information";

  try
  {
    _runMonCollection.update();
    _fragMonCollection.update();
    _filesMonCollection.update();
    _streamsMonCollection.update();
    reportStateName();
  }
  catch(xcept::Exception &e)
  {
    LOG4CPLUS_ERROR(_app->getApplicationLogger(),
      errorMsg << xcept::stdformat_exception_history(e));

    XCEPT_DECLARE_NESTED(stor::exception::Monitoring,
      sentinelException, errorMsg, e);
    _app->notifyQualified("error", sentinelException);
  }
  catch(std::exception &e)
  {
    errorMsg += ": ";
    errorMsg += e.what();

    LOG4CPLUS_ERROR(_app->getApplicationLogger(),
      errorMsg);
    
    XCEPT_DECLARE(stor::exception::Monitoring,
      sentinelException, errorMsg);
    _app->notifyQualified("error", sentinelException);
  }
  catch(...)
  {
    errorMsg += ": Unknown exception";

    LOG4CPLUS_ERROR(_app->getApplicationLogger(),
      errorMsg);
    
    XCEPT_DECLARE(stor::exception::Monitoring,
      sentinelException, errorMsg);
    _app->notifyQualified("error", sentinelException);
  }

  return _doMonitoring;
}

/////////////////////////
//// Set state name: ////
/////////////////////////
void StatisticsReporter::setExternallyVisibleState( const std::string& n )
{
  boost::mutex::scoped_lock sl( _state_name_lock );
  _externallyVisibleState = n;
}

/////////////////////////
//// Get state name: ////
/////////////////////////
const std::string& StatisticsReporter::externallyVisibleState() const
{
  boost::mutex::scoped_lock sl( _state_name_lock );
  return _externallyVisibleState;
}

/////////////////////////////////////////
//// Update state name in infospace: ////
/////////////////////////////////////////
void StatisticsReporter::reportStateName()
{

  xdata::InfoSpace* ispace = _app->getApplicationInfoSpace();

  const std::string errorMsg =
    "Failed to update state name in infospace " + ispace->name();

  try
    {
      ispace->lock();
      _xdaq_state_name =
        static_cast<xdata::String>( _externallyVisibleState );
      ispace->unlock();
    }
  catch( ... )
    {
      ispace->unlock();
      XCEPT_RAISE( stor::exception::Monitoring, errorMsg );
    }

  try
  {
    // Notify the info space that the value has changed
    ispace->fireItemValueChanged( "stateName", this );
  }
  catch (xdata::exception::Exception &e)
  {
    XCEPT_RETHROW(stor::exception::Infospace, errorMsg, e);
  }
}


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
