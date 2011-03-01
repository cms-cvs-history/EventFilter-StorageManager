// $Id: WrapperNotifier.h,v 1.9.10.2 2011/02/28 17:56:15 mommsen Exp $
/// @file: WrapperNotifier.h 

#ifndef EventFilter_StorageManager_WrapperNotifier_h
#define EventFilter_StorageManager_WrapperNotifier_h

#include "EventFilter/StorageManager/interface/Notifier.h"

#include "xdaq/Application.h"
#include "xdaq2rc/RcmsStateNotifier.h"


namespace stor
{

  /**
     Notifier implementation used by StorageManager

     $Author: mommsen $
     $Revision: 1.9.10.2 $
     $Date: 2011/02/28 17:56:15 $
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

  private:

    xdaq2rc::RcmsStateNotifier rcmsNotifier_;
    xdaq::Application* app_;

  };

} // namespace stor

#endif // EventFilter_StorageManager_WrapperNotifier_h



/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
