// $Id: ConsumerUtils.cc,v 1.1.2.2 2009/04/13 18:45:43 biery Exp $

#include "EventFilter/StorageManager/interface/ConsumerUtils.h"
#include "EventFilter/StorageManager/interface/EventConsumerRegistrationInfo.h"
#include "EventFilter/StorageManager/interface/Exception.h"

#include "IOPool/Streamer/interface/ConsRegMessage.h"
#include "IOPool/Streamer/interface/OtherMessage.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "xgi/Input.h"
#include "xgi/Output.h"
#include "xgi/exception/Exception.h"

#include <string>
#include <vector>

using namespace stor;

////////////////////////////////////////////
//// Create consumer registration info: ////
////////////////////////////////////////////
ConsRegPtr stor::parseEventConsumerRegistration( xgi::Input* in,
						 utils::duration_t sts )
{

  if( in == 0 )
    {
      XCEPT_RAISE( xgi::exception::Exception,
		   "Null xgi::Input* in parseEventConsumerRegistration" );
    }

  std::string name = "unknown";
  std::string pset_str = "<>";

  const std::string l_str = in->getenv( "CONTENT_LENGTH" );
  unsigned long l = std::atol( l_str.c_str() );

  if( l > 0 )
    {
      std::auto_ptr< std::vector<char> > buf( new std::vector<char>(l) );
      in->read( &(*buf)[0], l );
      ConsRegRequestView req( &(*buf)[0] );
      name = req.getConsumerName();
      pset_str = req.getRequestParameterSet();
    }
  else
    {
      XCEPT_RAISE( stor::exception::ConsumerRegistration,
		   "Bad request length" );
    }

  const edm::ParameterSet pset( pset_str );

  //
  //// Check if HLT output module is there: ////
  //

  std::string sel_hlt_out = "";

  try
    {
      // new-style consumer
      sel_hlt_out = pset.getParameter<std::string>( "TrackedHLTOutMod" );
    }
  catch( edm::Exception& e )
    {
      // old-style consumer or param not specified
      sel_hlt_out =
	pset.getUntrackedParameter<std::string>( "SelectHLTOutput", "" );
    }

  if( sel_hlt_out == "" )
    {
      XCEPT_RAISE( stor::exception::ConsumerRegistration,
		   "No HLT output module specified" );
    }

  // Maximum event rate:
  double max_rate = 1.0;
  try
    {
      max_rate = pset.getParameter<double>( "TrackedMaxRate" );
    }
  catch( edm::Exception& e )
    {
      max_rate =
	pset.getUntrackedParameter<double>( "maxEventRequestRate", 1.0 );
    }

  // Event filters:
  EventConsumerRegistrationInfo::FilterList sel_events;
  try
    {
      sel_events = pset.getParameter<Strings>( "TrackedEventSelection" );
    }
  catch( edm::Exception& e )
    {
      edm::ParameterSet tmpPSet1 =
        pset.getUntrackedParameter<edm::ParameterSet>( "SelectEvents",
                                                       edm::ParameterSet() );
      if ( ! tmpPSet1.empty() )
        {
          sel_events = tmpPSet1.getParameter<Strings>( "SelectEvents" );
        }
    }

  // Number of retries:
  unsigned int max_conn_retr = 5;
  try
    {
      max_conn_retr = pset.getParameter<int>( "TrackedMaxConnectTries" );
    }
  catch( edm::Exception& e )
    {
      pset.getUntrackedParameter<int>( "maxConnectTries", 5 );
    }

  // Retry interval:
  unsigned int conn_retr_interval = 10;
  try
    {
      conn_retr_interval =
	pset.getParameter<int>( "TrackedConnectTrySleepTime" );
    }
  catch( edm::Exception& e )
    {
      conn_retr_interval =
	pset.getUntrackedParameter<int>( "connectTrySleepTime", 10 );
    }

  // Header retry interval:
  unsigned int hdr_retr_interval = 5;
  try
    {
      hdr_retr_interval =
	pset.getParameter<int>( "TrackedHeaderRetryInterval" );
    }
  catch( edm::Exception& e )
    {
      hdr_retr_interval =
	pset.getUntrackedParameter<int>( "headerRetryInterval", 5 );
    }

  ConsRegPtr cr( new EventConsumerRegistrationInfo( max_conn_retr,
						    conn_retr_interval,
						    name,
						    hdr_retr_interval,
						    max_rate,
						    sel_events,
						    sel_hlt_out,
						    sts ) );
  return cr;

}

/////////////////////////////
//// Write HTTP headers: ////
/////////////////////////////
void stor::writeHTTPHeaders( xgi::Output* out )
{
  out->getHTTPResponseHeader().addHeader( "Content-Type",
					  "application/octet-stream" );
  out->getHTTPResponseHeader().addHeader( "Content-Transfer-Encoding",
					  "binary" );
}

//////////////////////////////
//// Send ID to consumer: ////
//////////////////////////////
void stor::writeEventConsumerRegistration( xgi::Output* out, ConsumerID cid )
{

  const int buff_size = 1000;
  std::vector<unsigned char> buff( buff_size );

  ConsRegResponseBuilder rb( &buff[0], buff.capacity(), 0, cid.value );
  ConsRegResponseView rv( &buff[0] );
  const unsigned int len = rv.size();

  writeHTTPHeaders( out );
  out->write( (char*)(&buff[0]), len );

}

////////////////////////////////////////
//// Tell consumer we're not ready: ////
////////////////////////////////////////
void stor::writeNotReady( xgi::Output* out )
{

  const int buff_size = 1000;
  std::vector<unsigned char> buff( buff_size );

  ConsRegResponseBuilder rb( &buff[0], buff.capacity(),
			     ConsRegResponseBuilder::ES_NOT_READY, 0 );
  ConsRegResponseView rv( &buff[0] );
  const unsigned int len = rv.size();

  writeHTTPHeaders( out );
  out->write( (char*)(&buff[0]), len );

}

////////////////////////////////////////
//// Send empty buffer to consumer: ////
////////////////////////////////////////
void stor::writeEmptyBuffer( xgi::Output* out )
{
  char buff;
  writeHTTPHeaders( out );
  out->write( &buff, 0 );
}

//////////////////////////////////////////////////
//// Extract consumer ID from header request: ////
//////////////////////////////////////////////////
ConsumerID stor::getConsumerID( xgi::Input* in )
{

  if( in == 0 )
    {          
      XCEPT_RAISE( xgi::exception::Exception,
		   "Null xgi::Input* in getConsumerID" );
    }

  const std::string l_str = in->getenv( "CONTENT_LENGTH" );
  unsigned long l = std::atol( l_str.c_str() );

  unsigned int cid_int = 0;

  if( l > 0 )
    {
      std::auto_ptr< std::vector<char> > buf( new std::vector<char>(l) );
      in->read( &(*buf)[0], l );
      OtherMessageView req( &(*buf)[0] );
      if( req.code() == Header::HEADER_REQUEST ||
	  req.code() == Header::EVENT_REQUEST )
	{
	  uint8* ptr = req.msgBody();
	  cid_int = convert32( ptr );
	}
      else
	{
	  XCEPT_RAISE( stor::exception::Exception,
		       "Bad request code in getConsumerID" );
	}
    }
  else
    {
      XCEPT_RAISE( stor::exception::Exception,
		   "Bad request length in getConsumerID" );
    }

  return ConsumerID( cid_int );

}

///////////////////////
//// Write header: ////
///////////////////////
void stor::writeConsumerHeader( xgi::Output* out, InitMsgSharedPtr ptr )
{
  const unsigned int len = ptr->size();
  std::vector<unsigned char> buff( len );
  for( unsigned int i = 0; i < len; ++i )
    {
      buff[i] = (*ptr)[i];
    }
  writeHTTPHeaders( out );
  out->write( (char*)(&buff[0]), len );
}

///////////////////////
//// Write event: ////
///////////////////////
void stor::writeConsumerEvent( xgi::Output* out, const I2OChain& evt )
{

  writeHTTPHeaders( out );

  const unsigned int nfrags = evt.fragmentCount();
  for ( unsigned int i = 0; i < nfrags; ++i )
   {
     const unsigned int len = evt.dataSize( idx );
     unsigned char* location = evt.dataLocation( idx );
     out->write( (char*)location, len );
   } 

}
