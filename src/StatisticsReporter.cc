// $Id: StatisticsReporter.cc,v 1.1.2.1 2009/02/16 16:13:04 mommsen Exp $

#include <string>
#include <sstream>

#include "toolbox/task/Action.h"
#include "toolbox/task/WorkLoopFactory.h"
#include "xcept/tools.h"
#include "xdaq/ApplicationDescriptor.h"

#include "EventFilter/StorageManager/interface/Exception.h"
#include "EventFilter/StorageManager/interface/MonitoredQuantity.h"
#include "EventFilter/StorageManager/interface/StatisticsReporter.h"

using namespace stor;


StatisticsReporter::StatisticsReporter(xdaq::Application *app) :
_app(app),
_runMonCollection(app),
_fragMonCollection(app),
_doMonitoring(true)
{
  try
  {
    xdaq::ApplicationDescriptor *appDesc = _app->getApplicationDescriptor();
    std::ostringstream identifier;
    identifier << appDesc->getClassName() << appDesc->getInstance() << "/";

    _monitorWL=
      toolbox::task::getWorkLoopFactory()->getWorkLoop(
        identifier.str() + "StatisticsReporter",
        "waiting");

    if ( ! _monitorWL->isActive() )
    {
      toolbox::task::ActionSignature* monitorAction = 
        toolbox::task::bind(this, &StatisticsReporter::monitorAction, 
          identifier.str() + "MonitorAction");
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
  unsigned int timeLeft = MonitoredQuantity::EXPECTED_CALCULATION_INTERVAL;
  while (timeLeft != 0)
  {
    timeLeft = ::sleep(timeLeft);
  }

  std::string errorMsg = "Failed to update the monitoring information";

  try
  {
    _runMonCollection.update();
    _fragMonCollection.update();
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


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
