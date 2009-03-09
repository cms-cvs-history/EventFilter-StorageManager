// $Id: ErrorStreamSelector.cc,v 1.1.2.2 2009/03/06 20:47:11 biery Exp $

#include "EventFilter/StorageManager/interface/ErrorStreamSelector.h"

using namespace stor;

bool ErrorStreamSelector::acceptEvent( const I2OChain& ioc )
{

  return true;

}
