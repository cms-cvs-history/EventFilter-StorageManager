// $Id: WebPageHelper.cc,v 1.56.2.5 2011/01/24 12:18:39 mommsen Exp $
/// @file: WebPageHelper.cc

#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdio.h>

#include "EventFilter/StorageManager/interface/AlarmHandler.h"
#include "EventFilter/StorageManager/interface/Utils.h"
#include "EventFilter/StorageManager/interface/WebPageHelper.h"

#include "toolbox/net/Utils.h"

namespace stor
{

  WebPageHelper::WebPageHelper(xdaq::ApplicationDescriptor* appDesc) :
  _appDescriptor(appDesc)
  {
    // set application icon for hyperdaq
    appDesc->setAttribute("icon", "/evf/images/smicon.jpg");
    
    _tableAttr[ "frame" ] = "void";
    _tableAttr[ "rules" ] = "group";
    _tableAttr[ "class" ] = "states";
    _tableAttr[ "cellspacing" ] = "0";
    _tableAttr[ "cellpadding" ] = "2";
    _tableAttr[ "width" ] = "100%";
    _tableAttr[ "valign" ] = "top";
    
    _rowAttr[ "valign" ] = "top";
    
    _specialRowAttr = _rowAttr;
    _specialRowAttr[ "class" ] = "special";
    
    _alarmColors[ AlarmHandler::OKAY ] = "#FFFFFF";
    _alarmColors[ AlarmHandler::WARNING ] = "#FFE635";
    _alarmColors[ AlarmHandler::ERROR ] = "#FF9F36";
    _alarmColors[ AlarmHandler::FATAL ] = "#FF2338";
    
    _tableLabelAttr[ "align" ] = "left";
    _tableLabelAttr[ "valign" ] = "middle";
    
    _tableValueAttr[ "align" ] = "right";
    _tableValueAttr[ "valign" ] = "middle";
  }

  
  std::string WebPageHelper::baseURL() const
  {
    return _appDescriptor->getContextDescriptor()->getURL() + "/" + _appDescriptor->getURN();
  }
  
  
  XHTMLMaker::Node* WebPageHelper::createWebPageBody
  (
    XHTMLMaker& maker,
    const std::string& pageTitle,
    const std::string& externallyVisibleState,
    const std::string& innerStateName,
    const std::string& errorMsg
  )
  {
    std::ostringstream title;
    title << _appDescriptor->getClassName()
      << " instance " << _appDescriptor->getInstance()
      << " - " << pageTitle;
    XHTMLMaker::Node* body = maker.start(title.str());
    
    std::ostringstream stylesheetLink;
    stylesheetLink << "/" << _appDescriptor->getURN()
      << "/styles.css";
    XHTMLMaker::AttrMap stylesheetAttr;
    stylesheetAttr[ "rel" ] = "stylesheet";
    stylesheetAttr[ "type" ] = "text/css";
    stylesheetAttr[ "href" ] = stylesheetLink.str();
    maker.addNode("link", maker.getHead(), stylesheetAttr);
    
    XHTMLMaker::AttrMap tableAttr;
    tableAttr[ "border" ] = "0";
    tableAttr[ "cellspacing" ] = "7";
    tableAttr[ "width" ] = "100%";
    XHTMLMaker::Node* table = maker.addNode("table", body, tableAttr);
    
    XHTMLMaker::Node* tableRow = maker.addNode("tr", table, _rowAttr);
    
    XHTMLMaker::AttrMap tableDivAttr;
    tableDivAttr[ "align" ] = "left";
    tableDivAttr[ "width" ] = "64";
    XHTMLMaker::Node* tableDiv = maker.addNode("td", tableRow, tableDivAttr);
    
    XHTMLMaker::AttrMap smLinkAttr;
    smLinkAttr[ "href" ] = _appDescriptor->getContextDescriptor()->getURL()
      + "/" + _appDescriptor->getURN();
    XHTMLMaker::Node* smLink = maker.addNode("a", tableDiv, smLinkAttr);
    
    XHTMLMaker::AttrMap smImgAttr;
    smImgAttr[ "align" ] = "middle";
    smImgAttr[ "src" ] = "/evf/images/smicon.jpg"; // $XDAQ_DOCUMENT_ROOT is prepended to this path
    smImgAttr[ "alt" ] = "main";
    smImgAttr[ "width" ] = "64";
    smImgAttr[ "height" ] = "64";
    smImgAttr[ "border" ] = "0";
    maker.addNode("img", smLink, smImgAttr);
    
    tableDiv = maker.addNode("td", tableRow);
    tableAttr[ "cellspacing" ] = "1";
    XHTMLMaker::Node* instanceTable = maker.addNode("table", tableDiv, tableAttr);
    XHTMLMaker::Node* instanceTableRow = maker.addNode("tr", instanceTable, _rowAttr);
    tableDivAttr[ "width" ] = "30%";
    XHTMLMaker::Node* instanceTableDiv = maker.addNode("td", instanceTableRow, tableDivAttr);
    XHTMLMaker::AttrMap fontAttr;
    fontAttr[ "size" ] = "+2";
    XHTMLMaker::Node* header = maker.addNode("font", instanceTableDiv, fontAttr);
    header = maker.addNode("b", header);
    maker.addText(header, _appDescriptor->getClassName());
    
    const std::string cvsVersion = "$Name:  $";
    if ( cvsVersion.length() > 9 ) {
      const std::string smVersion = "(" + cvsVersion.substr(7, cvsVersion.length()-9) + ")";
      maker.addText(instanceTableDiv, smVersion);
    }
    
    tableDivAttr[ "width" ] = "30%";
    instanceTableDiv = maker.addNode("td", instanceTableRow, tableDivAttr);
    header = maker.addNode("font", instanceTableDiv, fontAttr);
    header = maker.addNode("b", header);
    std::ostringstream instance;
    instance << "Instance " << _appDescriptor->getInstance();
    maker.addText(header, instance.str());
    
    tableDivAttr[ "width" ] = "40%";
    instanceTableDiv = maker.addNode("td", instanceTableRow, tableDivAttr);
    header = maker.addNode("font", instanceTableDiv, fontAttr);
    header = maker.addNode("b", header);
    maker.addText(header, externallyVisibleState);
    
    instanceTableRow = maker.addNode("tr", instanceTable, _rowAttr);
    
    instanceTableDiv = maker.addNode("td", instanceTableRow);
    fontAttr[ "size" ] = "-1";
    XHTMLMaker::Node* timestamp = maker.addNode("font", instanceTableDiv, fontAttr);
    maker.addText(timestamp,
      "Page last updated: " + utils::asctimeUTC(utils::getCurrentTime()) );
    
    instanceTableDiv = maker.addNode("td", instanceTableRow);
    XHTMLMaker::Node* hostname = maker.addNode("font", instanceTableDiv, fontAttr);
    maker.addText(hostname, "on " + toolbox::net::getHostName() );
    
    instanceTableDiv = maker.addNode("td", instanceTableRow);
    XHTMLMaker::Node* innerState = maker.addNode("font", instanceTableDiv, fontAttr);
    maker.addText(innerState, innerStateName);
    
    tableDivAttr[ "align" ] = "right";
    tableDivAttr[ "width" ] = "64";
    tableDiv = maker.addNode("td", tableRow, tableDivAttr);
    
    XHTMLMaker::AttrMap xdaqLinkAttr;
    xdaqLinkAttr[ "href" ] = "/urn:xdaq-application:lid=3";
    XHTMLMaker::Node* xdaqLink = maker.addNode("a", tableDiv, xdaqLinkAttr);
    
    XHTMLMaker::AttrMap xdaqImgAttr;
    xdaqImgAttr[ "align" ] = "middle";
    xdaqImgAttr[ "src" ] = "/hyperdaq/images/HyperDAQ.jpg"; // $XDAQ_DOCUMENT_ROOT is prepended to this path
    xdaqImgAttr[ "alt" ] = "HyperDAQ";
    xdaqImgAttr[ "width" ] = "64";
    xdaqImgAttr[ "height" ] = "64";
    xdaqImgAttr[ "border" ] = "0";
    maker.addNode("img", xdaqLink, xdaqImgAttr);
    
    // Status message box (reason for failed state, etc.):
    if( ! errorMsg.empty() )
    {
      maker.addNode( "hr", body );
      XHTMLMaker::Node* msg_box = maker.addNode( "p", body );
      maker.addText( msg_box, errorMsg );
    }
    
    maker.addNode( "hr", body );
    
    return body;
  }

} // namespace stor


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
