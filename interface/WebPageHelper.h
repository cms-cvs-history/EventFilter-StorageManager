// $Id$

#ifndef StorageManager_WebPageHelper_h
#define StorageManager_WebPageHelper_h

#include <string>

#include "toolbox/mem/Pool.h"
#include "xdaq/ApplicationDescriptor.h"
#include "xgi/Output.h"

#include "EventFilter/StorageManager/interface/FragmentMonitorCollection.h"
#include "EventFilter/StorageManager/interface/XHTMLMaker.h"

namespace stor {

  /**
   * WebPageHelper
   *
   * $Author:$
   * $Revision:$
   * $Date:$
   */
  
  class WebPageHelper
  {
  public:
    
    explicit WebPageHelper(xdaq::ApplicationDescriptor*);

    void defaultWebPage
    (
      xgi::Output*, 
      const std::string stateName,
      const unsigned int runNumber,
      const unsigned int receivedEvents,
      const FragmentMonitorCollection&,
      toolbox::mem::Pool*,
      const int nLogicalDisk,
      const std::string filePath
    );


  private:
    
    XHTMLMaker::Node* createWebPageBody(const std::string stateName);
    
    void addDOMforResourceUsage
    (
      xercesc::DOMElement *parent,
      toolbox::mem::Pool*,
      const int nLogicalDisk,
      const std::string filePath
    );

  
  private:

    xdaq::ApplicationDescriptor *_appDescriptor;
    
  };
  
} // namespace stor

#endif // StorageManager_WebPageHelper_h 


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
