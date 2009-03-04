// $Id: MonitorCollection.cc,v 1.1.2.8 2009/03/02 18:08:22 biery Exp $

#include <sstream>

#include "toolbox/net/URN.h"
#include "xdata/InfoSpaceFactory.h"

#include "EventFilter/StorageManager/interface/MonitorCollection.h"
#include "EventFilter/StorageManager/interface/Exception.h"

using namespace stor;


MonitorCollection::MonitorCollection
(
  xdaq::Application *app,
  const std::string infoSpaceName
)
{
  // Get the infospace
  toolbox::net::URN urn = 
    app->createQualifiedInfoSpace(infoSpaceName);
  _infoSpace = xdata::getInfoSpaceFactory()->get(urn.toString());

}

void MonitorCollection::update()
{
  calculateStatistics();
  updateInfoSpace();
}


void MonitorCollection::calculateStatistics()
{
  // do any operations that are common for all child classes

  do_calculateStatistics();
}


void MonitorCollection::updateInfoSpace()
{
  // do any operations that are common for all child classes

  do_updateInfoSpace();
}


void MonitorCollection::putItemsIntoInfoSpace()
{
  MonitorCollection::infoSpaceItems_t::const_iterator itor;

  for (
    itor=_infoSpaceItems.begin(); 
    itor!=_infoSpaceItems.end();
    ++itor)
  {
    try
    {
      // fireItemAvailable locks the infospace internally
      _infoSpace->fireItemAvailable(itor->first, itor->second);
    }
    catch(xdata::exception::Exception &e)
    {
      std::stringstream oss;
      
      oss << "Failed to put " << itor->first;
      oss << " into info space " << _infoSpace->name();
      
      XCEPT_RETHROW(stor::exception::Monitoring, oss.str(), e);
    }

    // keep a list of info space names for the fireItemGroupChanged
    _infoSpaceItemNames.push_back(itor->first);
  }
}


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
