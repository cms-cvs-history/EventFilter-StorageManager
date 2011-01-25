// $Id: WebPageHelper.h,v 1.12.2.2 2011/01/25 11:28:37 mommsen Exp $
/// @file: WebPageHelper.h

#ifndef EventFilter_StorageManager_WebPageHelper_h
#define EventFilter_StorageManager_WebPageHelper_h

#include <map>
#include <string>

#include "xdaq/ApplicationDescriptor.h"
#include "xgi/Input.h"
#include "xgi/Output.h"

#include "EventFilter/Utilities/interface/Css.h"

#include "EventFilter/StorageManager/interface/XHTMLMaker.h"


namespace stor {

  /**
   * Helper class to handle web page requests
   *
   * $Author: mommsen $
   * $Revision: 1.12.2.2 $
   * $Date: 2011/01/25 11:28:37 $
   */
  
  class WebPageHelper
  {
  public:

    WebPageHelper
    (
      xdaq::ApplicationDescriptor*,
      const std::string& cvsVersion
    );


    /**
     * Create event filter style sheet
     */
    void css(xgi::Input *in, xgi::Output *out)
    { css_.css(in,out); }
        
    
  protected:

    /**
      Get base url
    */
    std::string baseURL() const;

    /**
     * Returns the webpage body with the standard header as XHTML node
     */
    XHTMLMaker::Node* createWebPageBody
    (
      XHTMLMaker&,
      const std::string& pageTitle,
      const std::string& externallyVisibleState,
      const std::string& innerStateName,
      const std::string& errorMsg
    );


  private:

    //Prevent copying of the WebPageHelper
    WebPageHelper(WebPageHelper const&);
    WebPageHelper& operator=(WebPageHelper const&);

    const std::string _cvsVersion;
    evf::Css css_;
    
    
  protected:

    xdaq::ApplicationDescriptor* _appDescriptor;

    XHTMLMaker::AttrMap _tableAttr;
    XHTMLMaker::AttrMap _rowAttr;
    XHTMLMaker::AttrMap _tableLabelAttr;
    XHTMLMaker::AttrMap _tableValueAttr;
    XHTMLMaker::AttrMap _specialRowAttr;

    std::map<unsigned int, std::string> _alarmColors;

  };

} // namespace stor

#endif // EventFilter_StorageManager_WebPageHelper_h 


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
