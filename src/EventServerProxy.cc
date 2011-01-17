// $Id: EventServerProxy.cc,v 1.42.2.2 2011/01/14 12:01:57 mommsen Exp $
/// @file: EventServerProxy.cc

#include "EventFilter/StorageManager/interface/EventServerProxy.h"
#include "EventFilter/StorageManager/interface/SMCurlInterface.h"
#include "EventFilter/StorageManager/interface/CurlInterface.h"
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
  sourceurl_(ps.getParameter<std::string>("sourceURL")),
  consumerName_(ps.getUntrackedParameter<std::string>("consumerName","Unknown")),
  maxConnectTries_(ps.getUntrackedParameter<int>("maxConnectTries", 300)),
  connectTrySleepTime_(ps.getUntrackedParameter<int>("connectTrySleepTime", 10)),
  headerRetryInterval_(ps.getUntrackedParameter<int>("headerRetryInterval",5)),
  consumerId_(0),
  endRunAlreadyNotified_(true),
  alreadySaidHalted_(false),
  alreadySaidWaiting_(false)
  {
    double maxEventRequestRate = ps.getUntrackedParameter<double>("maxEventRequestRate",1.0);
    minEventRequestInterval_ = stor::utils::seconds_to_duration(
      maxEventRequestRate > 0 ? 
      1 / maxEventRequestRate :
      60
    );
    nextRequestTime_ = stor::utils::getCurrentTime() + minEventRequestInterval_;

    edm::ParameterSet psCopy(ps);
    edm::ParameterSet selectEventsParamSet =
      ps.getUntrackedParameter("SelectEvents", edm::ParameterSet());
    if (! selectEventsParamSet.empty()) {
      Strings path_specs = 
        selectEventsParamSet.getParameter<Strings>("SelectEvents");
      if (! path_specs.empty()) {
        psCopy.addParameter<Strings>("TrackedEventSelection", path_specs);
      }
    }

    psCopy.allToString(consumerPSetString_);

    registerWithEventServer();
  }


  void EventServerProxy::getOneEvent(std::string& data)
  {
    sleepUntilNextRequest();
    
    do
    {
      data.clear();
      getOneEventFromEventServer(data);
    }
    while ( !edm::shutdown_flag && data.empty() );

    if ( edm::shutdown_flag ) return;

    checkEvent(data);
  }
  
  
  void EventServerProxy::getOneEventFromEventServer(std::string& data)
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

    do
    {
      // send the event request
      stor::CurlInterface curl;
      CURLcode result = curl.postBinaryMessage(
        sourceurl_ + "/geteventdata",
        requestMessage.startAddress(),
        requestMessage.size(),
        data
      );
      
      if ( result != CURLE_OK )
      {
        std::cerr << "curl perform failed for event:" << std::endl;
        std::cerr << data << std::endl;
        data.clear();
        // this will end cmsRun 
        //return std::auto_ptr<EventPrincipal>();
        throw cms::Exception("getOneEvent","EventServerProxy")
          << "Could not get event: probably XDAQ not running on Storage Manager "
            << "\n";
      }
      
      if ( data.empty() )
      {
        if(!alreadySaidWaiting_) {
          std::cout << "...waiting for event from Storage Manager..." << std::endl;
          alreadySaidWaiting_ = true;
        }
        sleepUntilNextRequest();
      }
      else
      {
        alreadySaidWaiting_ = false;
      }
    } while (data.empty() && !edm::shutdown_flag);
  }
  
  
  void EventServerProxy::checkEvent(std::string& data)
  {
    // 29-Jan-2008, KAB:  catch (and re-throw) any exceptions decoding
    // the event data so that we can display the returned HTML and
    // (hopefully) give the user a hint as to the cause of the problem.
    try {
      HeaderView hdrView(&data[0]);
      if (hdrView.code() != Header::EVENT) {
        throw cms::Exception("EventServerProxy", "readOneEvent");
      }
    }
    catch (cms::Exception excpt) {
      const unsigned int MAX_DUMP_LENGTH = 2000;
      std::cout << "========================================" << std::endl;
      std::cout << "* Exception decoding the geteventdata response from the storage manager!" << std::endl;
      if (data.length() <= MAX_DUMP_LENGTH) {
        std::cout << "* Here is the raw text that was returned:" << std::endl;
        std::cout << data << std::endl;
      }
      else {
        std::cout << "* Here are the first " << MAX_DUMP_LENGTH <<
          " characters of the raw text that was returned:" << std::endl;
        std::cout << (data.substr(0, MAX_DUMP_LENGTH)) << std::endl;
      }
      std::cout << "========================================" << std::endl;
      throw excpt;
    }
  }
  
  
  inline void EventServerProxy::sleepUntilNextRequest()
  {
    stor::utils::sleepUntil(nextRequestTime_);
    nextRequestTime_ = stor::utils::getCurrentTime() +
      minEventRequestInterval_;
  }
  
  
  void EventServerProxy::getInitMsg(std::string& data)
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
  
  
  void EventServerProxy::getInitMsgFromEventServer(std::string& data)
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
      sourceurl_ + "/getregdata",
      requestMessage.startAddress(),
      requestMessage.size(),
      data
    );

    if ( result != CURLE_OK )
    {
      // connection failed: try to reconnect
      std::cerr << "curl perform failed for header:" << std::endl;
      std::cerr << data << std::endl;
      std::cerr << "Trying to reconnect." << std::endl;
      data.clear();
      registerWithEventServer();
    }
    
    if( data.empty() )
    {
      if(!alreadySaidWaiting_) {
        std::cout << "...waiting for header from Storage Manager..." << std::endl;
        alreadySaidWaiting_ = true;
      }
      // sleep for desired amount of time
      sleep(headerRetryInterval_);
    }
    else
    {
      alreadySaidWaiting_ = false;
    }
  }


  void EventServerProxy::checkInitMsg(std::string& data)
  {
    try {
      HeaderView hdrView(&data[0]);
      if (hdrView.code() != Header::INIT) {
        throw cms::Exception("EventServerProxy", "readHeader");
      }
    }
    catch (cms::Exception excpt) {
      const unsigned int MAX_DUMP_LENGTH = 1000;
      std::cout << "========================================" << std::endl;
      std::cout << "* Exception decoding the getregdata response from the storage manager!" << std::endl;
      if (data.length() <= MAX_DUMP_LENGTH) {
        std::cout << "* Here is the raw text that was returned:" << std::endl;
        std::cout << data << std::endl;
      }
      else {
        std::cout << "* Here are the first " << MAX_DUMP_LENGTH <<
          " characters of the raw text that was returned:" << std::endl;
        std::cout << (data.substr(0, MAX_DUMP_LENGTH)) << std::endl;
      }
      std::cout << "========================================" << std::endl;
      throw excpt;
    }
  }
  
  
  void EventServerProxy::registerWithEventServer()
  {
    std::string data;

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
  
  
  void EventServerProxy::connectToEventServer(std::string& data)
  {
    // build the registration request message to send to the storage manager
    const int bufferSize = 2000;
    char msgBuffer[bufferSize];
    ConsRegRequestBuilder requestMessage(
      msgBuffer, bufferSize, consumerName_,
      "normal", consumerPSetString_
    );
    
    // send registration request
    stor::CurlInterface curl;
    CURLcode result = CURLE_COULDNT_CONNECT;
    int tries = 0;
    
    while ( result != CURLE_OK && !edm::shutdown_flag )
    {
      ++tries;
      result = curl.postBinaryMessage(
        sourceurl_ + "/registerConsumer",
        requestMessage.startAddress(),
        requestMessage.size(),
        data
      );
      
      if ( result != CURLE_OK )
      {
        if ( tries >= maxConnectTries_ )
        {
          std::cerr << "Giving up waiting for connection after " << tries 
            << " tries for Storage Manager on " << sourceurl_ << std::endl;
          throw cms::Exception("connectToEventServer","EventServerProxy")
            << "Could not register: probably no Storage Manager is running on "
              << sourceurl_;
        }
        else
        {
          std::cout << "Waiting for connection to StorageManager on " << sourceurl_ << "... " 
            << tries << "/" << maxConnectTries_
            << std::endl;
          sleep(connectTrySleepTime_);
        }
      }
    }
  }
  
  
  bool EventServerProxy::extractConsumerId(std::string& data)      
  {
    boost::scoped_ptr<ConsRegResponseView> respView;

    try {
      respView.reset( new ConsRegResponseView(&data[0]) );
    }
    catch (cms::Exception excpt) {
      const unsigned int MAX_DUMP_LENGTH = 1000;
      std::cout << "========================================" << std::endl;
      std::cout << "* Exception decoding the registerWithEventServer response!" << std::endl;
      if (data.length() <= MAX_DUMP_LENGTH) {
        std::cout << "* Here is the raw text that was returned:" << std::endl;
        std::cout << data << std::endl;
      }
      else {
        std::cout << "* Here are the first " << MAX_DUMP_LENGTH <<
          " characters of the raw text that was returned:" << std::endl;
        std::cout << (data.substr(0, MAX_DUMP_LENGTH)) << std::endl;
      }
      std::cout << "========================================" << std::endl;
      throw excpt;
    }
    
    if ( respView->getStatus() == ConsRegResponseBuilder::ES_NOT_READY )
    {
      if(!alreadySaidWaiting_) {
        std::cout << "...waiting for registration response from Storage Manager..." << std::endl;
        alreadySaidWaiting_ = true;
      }
      // sleep for desired amount of time
      sleep(headerRetryInterval_);
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
