#include "EventFilter/StorageManager/interface/XHTMLMaker.h"

#include <string>
#include <iostream>

using namespace std;

int main()
{

  XHTMLMaker* maker = XHTMLMaker::instance();

  XHTMLMaker::Node* body = maker->start( "Fancy Page" );

  XHTMLMaker::AttrMap lmap;
  lmap[ "rel" ] = "stylesheet";
  lmap[ "href" ] = "xhtmlmaker_t.css";
  maker->addNode( "link", maker->getHead(), lmap );

  XHTMLMaker::AttrMap smap;
  smap[ "type" ] = "text/javascript";
  smap[ "src" ] = "xhtmlmaker_t.js";
  XHTMLMaker::Node* s = maker->addNode( "script", maker->getHead(), smap );
  maker->addText( s, " " );

  XHTMLMaker::Node* h1 = maker->addNode( "h1", body );
  maker->addText( h1, "Kinda Fancy Page" );

  XHTMLMaker::AttrMap pmap;
  XHTMLMaker::Node* p = maker->addNode( "p", body );
  maker->addText( p, "Hi there" );

  XHTMLMaker::AttrMap m;
  m[ "href" ] = "http://www.google.com";
  m[ "id" ] = "my_id";
  XHTMLMaker::Node* link =
    maker->addNode( "a", p, m );
  maker->addText( link, "Fancy Link" );

  maker->out( "test.xhtml" );

  return 0;

}
