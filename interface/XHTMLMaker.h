// -*- c++ -*-
// $Id: XHTMLMaker.h,v 1.1.2.3 2009/02/06 11:58:49 mommsen Exp $

#ifndef XHTMLMAKER_H
#define XHTMLMAKER_H

#include <xercesc/dom/DOM.hpp>
#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMWriter.hpp>
#include <xercesc/util/XMLString.hpp>

#include <map>
#include <string>
#include <iostream>

class XHTMLMaker
{

public:

  // Typedefs:
  typedef xercesc::DOMElement Node;
  typedef std::map<std::string,std::string> AttrMap;

  // Instance:
  static XHTMLMaker* instance();

  // Destructor:
  ~XHTMLMaker();

  // Initialize page and return body element:
  Node* start( const std::string& title );

  // Useful for css and javascript:
  Node* getHead() const { return _head; }

  // Add child:
  Node* addNode( const std::string& name,
                 Node* parent,
                 const AttrMap& attrs );

  // Add child to top level:
  Node* addNode( const std::string& name, const AttrMap& attrs )
  {
    return addNode( name, (Node*)0, attrs );
  }

  // Add child without attributes:
  Node* addNode( const std::string& name, Node* parent )
  {
    AttrMap empty;
    return addNode( name, parent, empty );
  }

  // Add child to top without attributes:
  Node* addNode( const std::string& name )
  {
    return addNode( name, (Node*)0 );
  }

  // Add text:
  void addText( Node* parent, const std::string& data );

  // Add a double value:
  void addText( Node* parent, const double& value, const int& precision = 2 );

  // Dump the page to stdout:
  void out();

  // Dump the page to a local file:
  void out( const std::string& dest );

  // Dump the page to a string:
  void out( std::string& dest );

  // Dump the page into an output stream:
  void out( std::ostream& dest );

private:

  XHTMLMaker();

  static XHTMLMaker* _instance;
  xercesc::DOMDocument* _doc;
  xercesc::DOMWriter* _writer;

  Node* _head;

  bool _page_started;

  // String to XMLCh:
  XMLCh* _xs( const std::string& str )
  {
    return xercesc::XMLString::transcode( str.data() );
  }

  // Set DOMWriter features:
  void _setWriterFeatures();

  // Cleanup:
  void _cleanup();

};

#endif

/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
