// $Id: ErrorEventSelector.cc,v 1.1.2.1 2009/03/06 19:17:29 biery Exp $

#include "EventFilter/StorageManager/interface/ErrorEventSelector.h"

using namespace stor;

bool ErrorEventSelector::acceptEvent( const I2OChain& ioc )
{

  return true;

}
