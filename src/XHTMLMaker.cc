#include "EventFilter/StorageManager/interface/XHTMLMaker.h"

#include <iostream>
#include <xercesc/util/PlatformUtils.hpp>
//#include <xercesc/framework/StdOutFormatTarget.hpp>
#include <xercesc/framework/LocalFileFormatTarget.hpp>

using namespace std;
using namespace xercesc;

//////////////////////////////
//// Singleton interface: ////
//////////////////////////////

XHTMLMaker* XHTMLMaker::_instance = (XHTMLMaker*)0;

XHTMLMaker* XHTMLMaker::instance()
{
  if( !_instance ) _instance = new XHTMLMaker();
  return _instance;
}

XHTMLMaker::XHTMLMaker()
{

  XMLPlatformUtils::Initialize();

  DOMImplementation* imp =
    DOMImplementationRegistry::getDOMImplementation( _xs("ls") );

  const XMLCh* xhtml_s = _xs( "html" );
  const XMLCh* p_id = _xs( "-//W3C//DTD XHTML 1.0 Strict//EN" );
  const XMLCh* s_id =
    _xs( "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd" );

  DOMDocumentType* typ =
    imp->createDocumentType( xhtml_s, p_id, s_id );

  const XMLCh* ns_uri = _xs( "http://www.w3.org/1999/xhtml" );

  _doc = imp->createDocument( ns_uri, xhtml_s, typ );

  if( !_doc )
    {
      cerr << "Cannot create document" << endl;
      return;
    }

  _doc->setEncoding( _xs( "utf-8" ) );
  //_doc->setStandalone( true );
  _doc->setVersion( _xs( "1.0" ) );

  _page_started = false;

  _writer =
    ( (DOMImplementationLS*)imp )->createDOMWriter();

  if( !_writer )
    {
      cerr << "Cannot create DOM writer" << endl;
      return;
    }

}

///////////////////////////////////////
//// Initialize page, return body: ////
///////////////////////////////////////

XHTMLMaker::Node* XHTMLMaker::start( const string& title )
{

  if( _page_started )
    {
      cerr << "Page already started" << endl;
      return (Node*)0;
    }

  _page_started = true;

  // Root element:
  Node* el_xhtml = _doc->getDocumentElement();

  // Head:
  Node* el_head = _doc->createElement( _xs( "head" ) );
  el_xhtml->appendChild( el_head );

  // Title:
  Node* el_title = _doc->createElement( _xs( "title" ) );
  el_head->appendChild( el_title );
  DOMText* txt_title = _doc->createTextNode( _xs( title ) );
  el_title->appendChild( txt_title );

  // Body:
  Node* el_body = _doc->createElement( _xs( "body" ) );
  el_xhtml->appendChild( el_body );

  return el_body;

}

////////////////////////////
//// Add child element: ////
////////////////////////////

XHTMLMaker::Node* XHTMLMaker::addNode( const string& name,
				       XHTMLMaker::Node* parent,
				       const AttrMap& attrs )
{
  Node* el = _doc->createElement( _xs( name ) );
  parent->appendChild( el );

  for( AttrMap::const_iterator i = attrs.begin(); i != attrs.end();
	 ++i )
    {
      el->setAttribute( _xs( i->first ), _xs( i->second ) );
    }

  return el;

}

///////////////////
//// Add text: ////
///////////////////

void XHTMLMaker::addText( Node* parent, const string& data )
{
  DOMText* txt = _doc->createTextNode( _xs( data ) );
  parent->appendChild( txt );
}

//////////////////////////////
//// Dump page to stdout: ////
//////////////////////////////

void XHTMLMaker::out()
{

  //_writer->setNewLine( (const XMLCh*)( L"\n" ) );

  if( _writer->canSetFeature( XMLUni::fgDOMWRTSplitCdataSections, true ) )
    {
      _writer->setFeature( XMLUni::fgDOMWRTSplitCdataSections, true );
    }

  if( _writer->canSetFeature( XMLUni::fgDOMWRTDiscardDefaultContent, true ) )
    {
      _writer->setFeature( XMLUni::fgDOMWRTDiscardDefaultContent, true );
    }

  if( _writer->canSetFeature( XMLUni::fgDOMWRTFormatPrettyPrint, true ) )
    {
      _writer->setFeature( XMLUni::fgDOMWRTFormatPrettyPrint, true );
    }

  if( _writer->canSetFeature( XMLUni::fgDOMWRTBOM, true ) )
    {
      _writer->setFeature( XMLUni::fgDOMWRTBOM, true );
    }

  //XMLFormatTarget* ftar = new StdOutFormatTarget();
  XMLFormatTarget* ftar = new LocalFileFormatTarget( "test.xhtml" );
  fflush( stdout );
  _writer->writeNode( ftar, *_doc );
  delete ftar;

  XMLPlatformUtils::Terminate();

  _instance = 0;

}
