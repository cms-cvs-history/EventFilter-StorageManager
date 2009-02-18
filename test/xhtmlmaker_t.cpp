#include "EventFilter/StorageManager/interface/XHTMLMonitor.h"
#include "EventFilter/StorageManager/interface/XHTMLMaker.h"

#include <string>
#include <iostream>

using namespace std;

void make_a_page( XHTMLMaker& maker,
		  const string& title,
		  const string& h_text,
		  const string& url,
		  const string& url_text,
		  const string& p_text )
{

  XHTMLMaker::Node* body = maker.start( title );

  XHTMLMaker::AttrMap lmap;
  lmap[ "rel" ] = "stylesheet";
  lmap[ "href" ] = "xhtmlmaker_t.css";
  maker.addNode( "link", maker.getHead(), lmap );

  XHTMLMaker::AttrMap smap;
  smap[ "type" ] = "text/javascript";
  smap[ "src" ] = "xhtmlmaker_t.js";
  XHTMLMaker::Node* s = maker.addNode( "script", maker.getHead(), smap );
  maker.addText( s, " " );

  XHTMLMaker::Node* h1 = maker.addNode( "h1", body );
  maker.addText( h1, h_text );

  XHTMLMaker::AttrMap pmap;
  XHTMLMaker::Node* p = maker.addNode( "p", body );
  maker.addText( p, p_text );

  XHTMLMaker::AttrMap m;
  m[ "href" ] = url;
  m[ "id" ] = "my_id";
  XHTMLMaker::Node* link =
    maker.addNode( "a", p, m );
  maker.addText( link, url_text );

}

int main()
{

  XHTMLMonitor monitor;

  XHTMLMaker maker;

  make_a_page( maker,
	       "Fancy Page",
	       "Kinda Fancy Page",
	       "http://www.google.com",
	       "Fancy Link",
	       "Hi there" );

  maker.out( "test.xhtml" );

  return 0;

}
