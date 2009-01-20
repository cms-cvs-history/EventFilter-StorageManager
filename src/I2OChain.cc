// $Id: I2OChain.cc,v 1.1.2.1 2009/01/19 18:14:06 mommsen Exp $

#include "EventFilter/StorageManager/interface/I2OChain.h"


stor::I2OChain::I2OChain() :
id(-1)
{
  
}


stor::I2OChain::~I2OChain()
{
  
}


bool stor::I2OChain::empty()
{
  return true;
}


bool stor::I2OChain::complete()
{
  return false;
}


void stor::I2OChain::addToChain(I2OChain &chain)
{

}


stor::I2OChain stor::I2OChain::copy()
{
  I2OChain chain;

  return chain;
}



/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
