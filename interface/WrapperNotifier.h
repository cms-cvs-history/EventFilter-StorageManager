// -*- c++ -*-
// $Id: WrapperNotifier.h,v 1.6 2009/07/13 13:27:45 dshpakov Exp $

#ifndef WRAPPERNOTIFIER_H
#define WRAPPERNOTIFIER_H

#include "EventFilter/StorageManager/interface/Notifier.h"

#include "xdaq/Application.h"
#include "xdaq2rc/RcmsStateNotifier.h"


namespace stor
{

  /**
     Notifier implementation used by StorageManager

     $Author: $
     $Revision: $
     $Date: $
  */
  class WrapperNotifier: public Notifier
  {
    
  public:

    /**
       Constructor
    */
    WrapperNotifier( xdaq::Application* app );

    /**
       Report new state to RCMS
    */
    void reportNewState( const std::string& stateName );

    /**
       Access logger
    */
    Logger& getLogger() { return _app->getApplicationLogger(); }

    /**
       Send message to sentinel
    */
    void tellSentinel( const std::string& level,
                       xcept::Exception& e )
    {
      _app->notifyQualified( level, e );
    }

  private:

    xdaq2rc::RcmsStateNotifier _rcms_notifier;
    xdaq::Application* _app;

    unsigned long instanceNumber() const
    {
      return _app->getApplicationDescriptor()->getInstance();
    }

  };

}

#endif // WRAPPERNOTIFIER_H



/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
