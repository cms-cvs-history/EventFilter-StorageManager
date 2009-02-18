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

void initialize_make_write_terminate( const string& title,
				      const string& h_text,
				      const string& url,
				      const string& url_text,
				      const string& p_text,
				      const string& file_name )
{

  XHTMLMonitor monitor;

  XHTMLMaker maker;

  make_a_page( maker,
	       title,
	       h_text,
	       url,
	       url_text,
	       p_text );

  maker.out( file_name );

}

void make_one_make_two_write_one_write_two( const string& file_one,
					    const string& file_two )
{

  XHTMLMonitor monitor;

  XHTMLMaker maker_one;

  make_a_page( maker_one,
	       "Fancy Page",
	       "Kinda Fancy Page",
	       "http://www.google.com",
	       "Fancy Link",
	       "Hi there" );

  XHTMLMaker maker_two;

  make_a_page( maker_two,
	       "Another Page",
	       "Not So Fancy Page",
	       "http://www.cern.ch",
	       "No Fun Here",
	       "Get Lost" );

  maker_one.out( file_one );

  maker_two.out( file_two );

}

int main()
{

  make_one_make_two_write_one_write_two( "test_1.xhtml", "test_2.xhtml" );

  initialize_make_write_terminate( "Fancy Page",
				   "Kinda Fancy Page",
				   "http://www.google.com",
				   "Fancy Link",
				   "Hi there",
				   "test_3.xhtml" );

  initialize_make_write_terminate( "Another Page",
				   "Not So Fancy Page",
				   "http://www.cern.ch",
				   "No Fun Here",
				   "Get Lost",
				   "test_4.xhtml" );

  return 0;

}
