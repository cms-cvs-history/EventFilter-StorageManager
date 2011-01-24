// $Id: XHTMLMonitor.h,v 1.5 2009/07/20 13:06:11 mommsen Exp $
/// @file: XHTMLMonitor.h 

#ifndef EventFilter_StorageManager_XHTMLMonitor_h
#define EventFilter_StorageManager_XHTMLMonitor_h


namespace stor {

  /**
    Controls the use of XHTMLMaker (xerces is not thread-safe)

    $Author: mommsen $
    $Revision: 1.5 $
    $Date: 2009/07/20 13:06:11 $
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

  };

} // namespace stor

#endif // EventFilter_StorageManager_XHTMLMonitor_h


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
