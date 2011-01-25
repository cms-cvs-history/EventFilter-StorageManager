// $Id: XHTMLMonitor.h,v 1.5.14.1 2011/01/24 12:18:39 mommsen Exp $
/// @file: XHTMLMonitor.h 

#ifndef EventFilter_StorageManager_XHTMLMonitor_h
#define EventFilter_StorageManager_XHTMLMonitor_h

#include "boost/thread/mutex.hpp"

namespace stor {

  /**
    Controls the use of XHTMLMaker (xerces is not thread-safe)

    $Author: mommsen $
    $Revision: 1.5.14.1 $
    $Date: 2011/01/24 12:18:39 $
  */
  
  class XHTMLMonitor
  {
    
  public:
    
    /**
      Constructor
    */
    XHTMLMonitor();

    /**
      Destructor
    */
    ~XHTMLMonitor();

  private:

    static boost::mutex _xhtmlMakerMutex;

  };

} // namespace stor

#endif // EventFilter_StorageManager_XHTMLMonitor_h


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
