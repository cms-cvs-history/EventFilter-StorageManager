// $Id$

#include "EventFilter/StorageManager/interface/FragmentMonitorCollection.h"

using namespace stor;


void FragmentMonitorCollection::do_calculateStatistics()
{
  allFragmentSizes.calculateStatistics();
  eventFragmentSizes.calculateStatistics();
  dqmEventFragmentSizes.calculateStatistics();
}


void FragmentMonitorCollection::do_updateInfoSpace()
{
  // get relevant stats from allFragmentSizes and store them in the infospace
  // get relevant stats from eventFragmentSizes and store them in the infospace
  // get relevant stats from dqmEventFragmentSizes and store them in the infospace
}


xercesc::DOMElement* FragmentMonitorCollection::do_addDOMElement(xercesc::DOMElement *parent)
{
  //Create the HTML snipped from ownded MonitoredQuantities
  return parent;
}



/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
