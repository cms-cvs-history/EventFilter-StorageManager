// $Id: EventServerProxy.cc,v 1.1.2.7 2011/02/14 16:53:49 mommsen Exp $
/// @file: EventServerProxy.cc

#include "EventFilter/StorageManager/interface/EventConsumerRegistrationInfo.h"
#include "EventFilter/StorageManager/interface/EventServerProxy.h"
#include "EventFilter/StorageManager/interface/SMCurlInterface.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/Utilities/interface/DebugMacros.h"
#include "FWCore/Utilities/interface/Exception.h"
#include "IOPool/Streamer/interface/ClassFiller.h"
#include "IOPool/Streamer/interface/EventMessage.h"
#include "IOPool/Streamer/interface/InitMessage.h"
#include "IOPool/Streamer/interface/OtherMessage.h"
#include "IOPool/Streamer/interface/ConsRegMessage.h"
#include "FWCore/Framework/interface/EventPrincipal.h"
#include "FWCore/Utilities/interface/UnixSignalHandlers.h"

#include <algorithm>
#include <iterator>
#include "curl/curl.h"
#include "boost/scoped_ptr.hpp"


namespace stor
{  
  EventServerProxy::EventServerProxy(edm::ParameterSet const& ps) :
  ecri_(ps),
  consumerId_(0),
  endRunAlreadyNotified_(true),
  alreadySaidHalted_(false),
  alreadySaidWaiting_(false)
  {
    nextRequestTime_ = stor::utils::getCurrentTime();

    registerWithEventServer();
  }


  void EventServerProxy::getOneEvent(CurlInterface::Content& data)
  {
    stor::utils::sleepUntil(nextRequestTime_);

    while ( ! edm::shutdown_flag && ! getEventMaybe(data) ) {}

    if ( ! minEventRequestInterval_.is_not_a_date_time() )
      nextRequestTime_ = stor::utils::getCurrentTime() +
        minEventRequestInterval_;
  }
 

  bool EventServerProxy::getEventMaybe(CurlInterface::Content& data)
  {
    data.clear();
    getOneEventFromEventServer(data);

    if ( edm::shutdown_flag || data.empty() ) return false;

    checkEvent(data);

    return true;
  }
  
  
  void EventServerProxy::getOneEventFromEventServer(CurlInterface::Content& data)
  {
    // build the event request message to send to the storage manager
    char msgBuff[100];
    OtherMessageBuilder requestMessage(
      &msgBuff[0],
      Header::EVENT_REQUEST,
      sizeof(char_uint32)
    );
    uint8 *bodyPtr = requestMessage.msgBody();
    convert(consumerId_, bodyPtr);

    // send the event request
    stor::CurlInterface curl;
    CURLcode result = curl.postBinaryMessage(
      ecri_.sourceURL() + "/geteventdata",
      requestMessage.startAddress(),
      requestMessage.size(),
      data
    );
    
    if ( result != CURLE_OK )
    {
      edm::LogError("EventServerProxy") << "curl perform failed for event: "
        << std::string(&data[0]);
      data.clear();
      // this will end cmsRun 
      //return std::auto_ptr<EventPrincipal>();
      throw cms::Exception("getOneEvent","EventServerProxy")
        << "Could not get event: probably XDAQ not running on Storage Manager "
          << "\n";
    }
    
    if ( data.empty() && !alreadySaidWaiting_) {
      edm::LogInfo("EventServerProxy") << "...waiting for first event from Storage Manager...";
      alreadySaidWaiting_ = true;
    }
  }
  
  
  void EventServerProxy::checkEvent(CurlInterface::Content& data)
  {
    // 29-Jan-2008, KAB:  catch (and re-throw) any exceptions decoding
    // the event data so that we can display the returned HTML and
    // (hopefully) give the user a hint as to the cause of the problem.
    try {
      HeaderView hdrView(&data[0]);
      if (hdrView.code() == Header::DONE) {
        if(!alreadySaidHalted_) {
          alreadySaidHalted_ = true;
          edm::LogInfo("EventServerProxy") << "Storage Manager has stopped";
        }
      }
      else if (hdrView.code() != Header::EVENT) {
        throw cms::Exception("EventServerProxy", "readOneEvent");
      }
      alreadySaidHalted_ = false;
    }
    catch (cms::Exception excpt) {
      const unsigned int MAX_DUMP_LENGTH = 2000;
      std::ostringstream dump;
      dump << "========================================" << std::endl;
      dump << "* Exception decoding the geteventdata response from the storage manager!" << std::endl;
      if (data.size() < MAX_DUMP_LENGTH)
      {
        dump << "* Here is the raw text that was returned:" << std::endl;
        dump << std::string(&data[0]) << std::endl;
      }
      else
      {
        dump << "* Here are the first " << MAX_DUMP_LENGTH <<
          " characters of the raw text that was returned:" << std::endl;
        dump << std::string(&data[0], MAX_DUMP_LENGTH) << std::endl;
      }
      dump << "========================================" << std::endl;
      edm::LogError("EventServerProxy") << dump.str();
      throw excpt;
    }
  }
  
  
  void EventServerProxy::getInitMsg(CurlInterface::Content& data)
  {
    do
    {
      data.clear();
      getInitMsgFromEventServer(data);
    }
    while ( !edm::shutdown_flag && data.empty() );
    
    if (edm::shutdown_flag) {
      throw cms::Exception("readHeader","EventServerProxy")
        << "The header read was aborted by a shutdown request.\n";
    }

    checkInitMsg(data);
  }
  
  
  void EventServerProxy::getInitMsgFromEventServer(CurlInterface::Content& data)
  {
    // build the header request message to send to the storage manager
    char msgBuff[100];
    OtherMessageBuilder requestMessage(
      &msgBuff[0],
      Header::HEADER_REQUEST,
      sizeof(char_uint32)
    );
    uint8 *bodyPtr = requestMessage.msgBody();
    convert(consumerId_, bodyPtr);

    // send the header request
    stor::CurlInterface curl;
    CURLcode result = curl.postBinaryMessage(
      ecri_.sourceURL() + "/getregdata",
      requestMessage.startAddress(),
      requestMessage.size(),
      data
    );

    if ( result != CURLE_OK )
    {
      // connection failed: try to reconnect
      edm::LogError("EventServerProxy") << "curl perform failed for header:"
        << std::string(&data[0]) << std::endl
        << ". Trying to reconnect.";
      data.clear();
      registerWithEventServer();
    }
    
    if( data.empty() )
    {
      if(!alreadySaidWaiting_) {
        edm::LogInfo("EventServerProxy") << "...waiting for header from Storage Manager...";
        alreadySaidWaiting_ = true;
      }
      // sleep for desired amount of time
      sleep(ecri_.headerRetryInterval());
    }
    else
    {
      alreadySaidWaiting_ = false;
    }
  }


  void EventServerProxy::checkInitMsg(CurlInterface::Content& data)
  {
    try {
      HeaderView hdrView(&data[0]);
      if (hdrView.code() != Header::INIT) {
        throw cms::Exception("EventServerProxy", "readHeader");
      }
    }
    catch (cms::Exception excpt) {
      const unsigned int MAX_DUMP_LENGTH = 1000;
      std::ostringstream dump;
      dump << "========================================" << std::endl;
      dump << "* Exception decoding the getregdata response from the storage manager!" << std::endl;
      if (data.size() <= MAX_DUMP_LENGTH)
      {
        dump << "* Here is the raw text that was returned:" << std::endl;
        dump << std::string(&data[0]) << std::endl;
      }
      else
      {
        dump << "* Here are the first " << MAX_DUMP_LENGTH <<
          " characters of the raw text that was returned:" << std::endl;
        dump << std::string(&data[0], MAX_DUMP_LENGTH) << std::endl;
      }
      dump << "========================================" << std::endl;
      edm::LogError("EventServerProxy") << dump.str();
      throw excpt;
    }
  }
  
  
  void EventServerProxy::registerWithEventServer()
  {
    CurlInterface::Content data;

    do
    {
      data.clear();
      connectToEventServer(data);
    }
    while ( !edm::shutdown_flag && !extractConsumerId(data) );

    if (edm::shutdown_flag) {
      throw cms::Exception("registerWithEventServer","EventServerProxy")
          << "Registration was aborted by a shutdown request.\n";
    }
  }
  
  
  void EventServerProxy::connectToEventServer(CurlInterface::Content& data)
  {
    // Serialize the ParameterSet
    edm::ParameterSet consumerPSet = ecri_.getPSet();
    std::string consumerPSetString;
    consumerPSet.allToString(consumerPSetString);

    // build the registration request message to send to the storage manager
    const int bufferSize = 2000;
    char msgBuffer[bufferSize];
    ConsRegRequestBuilder requestMessage(
      msgBuffer, bufferSize, ecri_.consumerName(),
      "normal", consumerPSetString
    );
    
    // send registration request
    stor::CurlInterface curl;
    CURLcode result = CURLE_COULDNT_CONNECT;
    int tries = 0;

    const std::string sourceURL = ecri_.sourceURL();
    const int maxConnectTries = ecri_.maxConnectTries();
    
    while ( result != CURLE_OK && !edm::shutdown_flag )
    {
      ++tries;
      result = curl.postBinaryMessage(
        sourceURL + "/registerConsumer",
        requestMessage.startAddress(),
        requestMessage.size(),
        data
      );
      
      if ( result != CURLE_OK )
      {
        if ( tries >= maxConnectTries )
        {
          edm::LogError("EventServerProxy") << "Giving up waiting for connection after " <<
            tries << " tries for Storage Manager on " << sourceURL << std::endl;
          throw cms::Exception("connectToEventServer","EventServerProxy")
            << "Could not register: probably no Storage Manager is running on "
              << sourceURL;
        }
        else
        {
          edm::LogInfo("EventServerProxy") << "Waiting for connection to StorageManager on " <<
            sourceURL << "... " << tries << "/" << maxConnectTries;
          sleep(ecri_.connectTrySleepTime());
        }
        data.clear();
      }
    }
  }
  
  
  bool EventServerProxy::extractConsumerId(CurlInterface::Content& data)      
  {
    boost::scoped_ptr<ConsRegResponseView> respView;

    try {
      respView.reset( new ConsRegResponseView(&data[0]) );
    }
    catch (cms::Exception excpt) {
      const unsigned int MAX_DUMP_LENGTH = 1000;
      std::ostringstream dump;
      dump << "========================================" << std::endl;
      dump << "* Exception decoding the registerWithEventServer response!" << std::endl;
      if (data.size() <= MAX_DUMP_LENGTH)
      {
        dump << "* Here is the raw text that was returned:" << std::endl;
        dump << std::string(&data[0]) << std::endl;
      }
      else
      {
        dump << "* Here are the first " << MAX_DUMP_LENGTH <<
          " characters of the raw text that was returned:" << std::endl;
        dump << std::string(&data[0], MAX_DUMP_LENGTH) << std::endl;
      }
      dump << "========================================" << std::endl;
      edm::LogError("EventServerProxy") << dump.str();
      throw excpt;
    }
    
    if ( respView->getStatus() == ConsRegResponseBuilder::ES_NOT_READY )
    {
      if(!alreadySaidWaiting_) {
        edm::LogInfo("EventServerProxy") << "...waiting for registration response from Storage Manager...";
        alreadySaidWaiting_ = true;
      }
      // sleep for desired amount of time
      sleep(ecri_.headerRetryInterval());
      return false;
    }
    else
    {
      alreadySaidWaiting_ = false;
    }

    consumerId_ = respView->getConsumerId();
    return true;
  }

} //namespace edm


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
