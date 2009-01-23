// -*- c++ -*-

#ifndef XHTMLMAKER_H
#define XHTMLMAKER_H

#include <xercesc/dom/DOM.hpp>
#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMWriter.hpp>
#include <xercesc/util/XMLString.hpp>

#include <map>
#include <string>

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

  // Dump page to stdout:
  void out();

private:

  XHTMLMaker();

  static XHTMLMaker* _instance;
  xercesc::DOMDocument* _doc;
  xercesc::DOMWriter* _writer;

  bool _page_started;

  // String to XMLCh:
  XMLCh* _xs( const std::string& str )
  {
    return xercesc::XMLString::transcode( str.data() );
  }

};

#endif
