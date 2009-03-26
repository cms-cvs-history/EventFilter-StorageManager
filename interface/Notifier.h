// -*- c++ -*-                                                                              
// $Id: Notifier.h,v 1.1.2.1 2009/03/25 13:22:35 dshpakov Exp $

#ifndef NOTIFIER_H
#define NOTIFIER_H

// Interface class for handling RCMS notifier

#include <string>

namespace stor
{

  class Notifier
  {

  public:

    Notifier() {}

    virtual ~Notifier() {};

    virtual void reportNewState( const std::string& stateName ) = 0;

  };

}

#endif // NOTIFIER_H
