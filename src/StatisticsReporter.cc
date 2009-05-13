// $Id: StatisticsReporter.cc,v 1.1.2.25 2009/05/13 14:48:30 mommsen Exp $

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
_dataSenderMonCollection(app),
_dqmEventMonCollection(app),
_resourceMonCollection(app),
_stateMachineMonCollection(app),
_monitorWL(0),
_doMonitoring(true)
{
  _consumerMonitorCollection.reset( new ConsumerMonitorCollection( app ) );
}


void StatisticsReporter::startWorkLoop(std::string workloopName)
{
  try
  {
    std::string identifier = utils::getIdentifier(_app->getApplicationDescriptor());

    _monitorWL=
      toolbox::task::getWorkLoopFactory()->getWorkLoop(
        identifier + workloopName, "waiting");

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
  if ( _monitorWL && _monitorWL->isActive() ) _monitorWL->cancel();
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
    _dataSenderMonCollection.update();
    _dqmEventMonCollection.update();
    _resourceMonCollection.update();
    _stateMachineMonCollection.update();
    _consumerMonitorCollection->update();
  }
  catch(xcept::Exception &e)
  {
    LOG4CPLUS_ERROR(_app->getApplicationLogger(),
      errorMsg << xcept::stdformat_exception_history(e));

    #ifndef STOR_BYPASS_SENTINEL
    XCEPT_DECLARE_NESTED(stor::exception::Monitoring,
      sentinelException, errorMsg, e);
    _app->notifyQualified("error", sentinelException);
    #endif
  }
  catch(std::exception &e)
  {
    errorMsg += ": ";
    errorMsg += e.what();

    LOG4CPLUS_ERROR(_app->getApplicationLogger(),
      errorMsg);
    
    #ifndef STOR_BYPASS_SENTINEL
    XCEPT_DECLARE(stor::exception::Monitoring,
      sentinelException, errorMsg);
    _app->notifyQualified("error", sentinelException);
    #endif
  }
  catch(...)
  {
    errorMsg += ": Unknown exception";

    LOG4CPLUS_ERROR(_app->getApplicationLogger(),
      errorMsg);
    
    #ifndef STOR_BYPASS_SENTINEL
    XCEPT_DECLARE(stor::exception::Monitoring,
      sentinelException, errorMsg);
    _app->notifyQualified("error", sentinelException);
    #endif
  }

  return _doMonitoring;
}


void StatisticsReporter::reset()
{
  // do not reset the stateMachineMonCollection, as we want to
  // keep the state machine history
  _runMonCollection.reset();
  _fragMonCollection.reset();
  _filesMonCollection.reset();
  _streamsMonCollection.reset();
  _dataSenderMonCollection.reset();
  _dqmEventMonCollection.reset();
  _resourceMonCollection.reset();
  _consumerMonitorCollection->reset();
}


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
