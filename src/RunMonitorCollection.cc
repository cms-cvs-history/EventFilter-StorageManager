// $Id: RunMonitorCollection.cc,v 1.1.2.6 2009/03/13 17:13:53 biery Exp $

#include <string>
#include <sstream>
#include <iomanip>

#include "EventFilter/StorageManager/interface/Exception.h"
#include "EventFilter/StorageManager/interface/RunMonitorCollection.h"

using namespace stor;

RunMonitorCollection::RunMonitorCollection(xdaq::Application *app) :
MonitorCollection(app, "Run")
{
  _infoSpaceItems.push_back(std::make_pair("runNumber", &_runNumber));
  _infoSpaceItems.push_back(std::make_pair("receivedEvents", &_receivedEvents));
  _infoSpaceItems.push_back(std::make_pair("receivedErrorEvents", &_receivedErrorEvents));

  putItemsIntoInfoSpace();
}


void RunMonitorCollection::do_calculateStatistics()
{
  _eventIDsReceived.calculateStatistics();
  _errorEventIDsReceived.calculateStatistics();
  _runNumbersSeen.calculateStatistics();
  _lumiSectionsSeen.calculateStatistics();
}


void RunMonitorCollection::do_updateInfoSpace()
{
  std::string errorMsg =
    "Failed to update values of items in info space " + _infoSpace->name();

  // Lock the infospace to assure that all items are consistent
  try
  {
    _infoSpace->lock();
    _runNumber = static_cast<xdata::UnsignedInteger32>(static_cast<unsigned int>(_runNumbersSeen.getLastSampleValue()));
    _receivedEvents = static_cast<xdata::UnsignedInteger32>(_eventIDsReceived.getSampleCount());
    _receivedErrorEvents = static_cast<xdata::UnsignedInteger32>(_errorEventIDsReceived.getSampleCount());
    _infoSpace->unlock();
  }
  catch(std::exception &e)
  {
    _infoSpace->unlock();
 
    errorMsg += ": ";
    errorMsg += e.what();
    XCEPT_RAISE(stor::exception::Monitoring, errorMsg);
  }
  catch (...)
  {
    _infoSpace->unlock();
 
    errorMsg += " : unknown exception";
    XCEPT_RAISE(stor::exception::Monitoring, errorMsg);
  }

  try
  {
    // The fireItemGroupChanged locks the infospace
    _infoSpace->fireItemGroupChanged(_infoSpaceItemNames, this);
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
