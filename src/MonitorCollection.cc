// $Id$

#include <functional>

#include "boost/bind.hpp"

#include "EventFilter/StorageManager/interface/MonitorCollection.h"

using namespace stor;


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


xercesc::DOMElement* MonitorCollection::addDOMElement(xercesc::DOMElement *parent)
{
  // do any operations that are common for all child classes

  return do_addDOMElement(parent);
}


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
