#include "EventFilter/StorageManager/interface/XHTMLMaker.h"

#include <string>
#include <iostream>

using namespace std;

int main()
{

  XHTMLMaker* maker = XHTMLMaker::instance();

  XHTMLMaker::Node* body = maker->start( "Fancy Page" );

  XHTMLMaker::Node* h1 = maker->addNode( "h1", body );
  maker->addText( h1, "Kinda Fancy Page" );

  XHTMLMaker::Node* p = maker->addNode( "p", body );
  maker->addText( p, "Hi there" );

  XHTMLMaker::AttrMap m;
  m[ "href" ] = "http://www.google.com";
  XHTMLMaker::Node* link =
    maker->addNode( "a", p, m );
  maker->addText( link, "Fancy Link" );

  maker->out( "test.xhtml" );

  return 0;

}
