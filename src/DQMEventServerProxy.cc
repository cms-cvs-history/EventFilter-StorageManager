// $Id: DQMEventServerProxy.cc,v 1.1.2.8 2011/02/17 13:18:08 mommsen Exp $
/// @file: DQMEventServerProxy.cc

#include "EventFilter/StorageManager/interface/DQMEventServerProxy.h"
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
  DQMEventServerProxy::DQMEventServerProxy(edm::ParameterSet const& ps) :
  dcri_(ps),
  consumerId_(0),
  alreadySaidHalted_(false),
  alreadySaidWaiting_(false)
  {
    nextRequestTime_ = stor::utils::getCurrentTime();

    registerWithDQMEventServer();
  }


  void DQMEventServerProxy::getOneDQMEvent(CurlInterface::Content& data)
  {
    stor::utils::sleepUntil(nextRequestTime_);

    while ( ! edm::shutdown_flag && ! getDQMEventMaybe(data) ) {}

    if ( ! minEventRequestInterval_.is_not_a_date_time() )
      nextRequestTime_ = stor::utils::getCurrentTime() +
        minEventRequestInterval_;
  }
 

  bool DQMEventServerProxy::getDQMEventMaybe(CurlInterface::Content& data)
  {
    data.clear();
    getOneDQMEventFromDQMEventServer(data);

    if ( edm::shutdown_flag || data.empty() ) return false;

    checkDQMEvent(data);

    return true;
  }
  
  
  void DQMEventServerProxy::getOneDQMEventFromDQMEventServer(CurlInterface::Content& data)
  {
    // build the event request message to send to the storage manager
    char msgBuff[100];
    OtherMessageBuilder requestMessage(
      &msgBuff[0],
      Header::DQMEVENT_REQUEST,
      sizeof(char_uint32)
    );
    uint8 *bodyPtr = requestMessage.msgBody();
    convert(consumerId_, bodyPtr);

    // send the event request
    stor::CurlInterface curl;
    CURLcode result = curl.postBinaryMessage(
      dcri_.sourceURL() + "/getDQMeventdata",
      requestMessage.startAddress(),
      requestMessage.size(),
      data
    );
    
    if ( result != CURLE_OK )
    {
      edm::LogError("DQMEventServerProxy") << "curl perform failed for DQM event: "
        << std::string(&data[0]);
      data.clear();
      throw cms::Exception("getOneDQMEvent","DQMEventServerProxy")
        << "Could not get DQM event: probably XDAQ not running on Storage Manager\n";
    }
    
    if ( data.empty() && !alreadySaidWaiting_) {
      edm::LogInfo("DQMEventServerProxy") << "...waiting for first DQM event from Storage Manager...";
      alreadySaidWaiting_ = true;
    }
  }
  
  
  void DQMEventServerProxy::checkDQMEvent(CurlInterface::Content& data)
  {
    // 29-Jan-2008, KAB:  catch (and re-throw) any exceptions decoding
    // the event data so that we can display the returned HTML and
    // (hopefully) give the user a hint as to the cause of the problem.
    try {
      HeaderView hdrView(&data[0]);
      if (hdrView.code() == Header::DONE) {
        if(!alreadySaidHalted_) {
          alreadySaidHalted_ = true;
          edm::LogInfo("DQMEventServerProxy") << "Storage Manager has stopped";
        }
      }
      else if (hdrView.code() != Header::DQM_EVENT) {
        throw cms::Exception("DQMEventServerProxy", "checkDQMEvent")
          << "Did not receive a DQM event, but an event with header code "
            << hdrView.code();
      }
      alreadySaidHalted_ = false;
    }
    catch (cms::Exception excpt) {
      const unsigned int MAX_DUMP_LENGTH = 2000;
      std::ostringstream dump;
      dump << "========================================" << std::endl;
      dump << "* Exception decoding the getDQMeventdata response from the storage manager!" << std::endl;
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
      edm::LogError("DQMEventServerProxy") << dump.str();
      throw excpt;
    }
  }
  
  
  void DQMEventServerProxy::registerWithDQMEventServer()
  {
    CurlInterface::Content data;

    do
    {
      data.clear();
      connectToDQMEventServer(data);
    }
    while ( !edm::shutdown_flag && !extractConsumerId(data) );

    if (edm::shutdown_flag) {
      throw cms::Exception("registerWithDQMEventServer","DQMEventServerProxy")
          << "Registration was aborted by a shutdown request.\n";
    }
  }
  
  
  void DQMEventServerProxy::connectToDQMEventServer(CurlInterface::Content& data)
  {
    // Serialize the ParameterSet
    edm::ParameterSet consumerPSet = dcri_.getPSet();
    std::string consumerPSetString;
    consumerPSet.allToString(consumerPSetString);

    // build the registration request message to send to the storage manager
    const int bufferSize = 2000;
    char msgBuffer[bufferSize];
    ConsRegRequestBuilder requestMessage(
      msgBuffer, bufferSize, dcri_.consumerName(),
      "normal", consumerPSetString
    );
    
    // send registration request
    stor::CurlInterface curl;
    CURLcode result = CURLE_COULDNT_CONNECT;
    int tries = 0;

    const std::string sourceURL = dcri_.sourceURL();
    const int maxConnectTries = dcri_.maxConnectTries();
    
    while ( result != CURLE_OK && !edm::shutdown_flag )
    {
      ++tries;
      result = curl.postBinaryMessage(
        sourceURL + "/registerDQMConsumer",
        requestMessage.startAddress(),
        requestMessage.size(),
        data
      );

      if ( result != CURLE_OK )
      {
        if ( tries >= maxConnectTries )
        {
          edm::LogError("DQMEventServerProxy") << "Giving up waiting for connection after " <<
            tries << " tries for Storage Manager on " << sourceURL << std::endl;
          throw cms::Exception("connectToDQMEventServer","DQMEventServerProxy")
            << "Could not register: probably no Storage Manager is running on "
              << sourceURL;
        }
        else
        {
          edm::LogInfo("DQMEventServerProxy") << "Waiting for connection to StorageManager on " <<
            sourceURL << "... " << tries << "/" << maxConnectTries;
          sleep(dcri_.connectTrySleepTime());
        }
        data.clear();
      }
    }
  }
  
  
  bool DQMEventServerProxy::extractConsumerId(CurlInterface::Content& data)      
  {
    boost::scoped_ptr<ConsRegResponseView> respView;

    try {
      respView.reset( new ConsRegResponseView(&data[0]) );
    }
    catch (cms::Exception excpt) {
      const unsigned int MAX_DUMP_LENGTH = 1000;
      std::ostringstream dump;
      dump << "========================================" << std::endl;
      dump << "* Exception decoding the registerWithDQMEventServer response!" << std::endl;
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
      edm::LogError("DQMEventServerProxy") << dump.str();
      throw excpt;
    }
    
    if ( respView->getStatus() == ConsRegResponseBuilder::ES_NOT_READY )
    {
      if(!alreadySaidWaiting_) {
        edm::LogInfo("DQMEventServerProxy") << "...waiting for registration response from Storage Manager...";
        alreadySaidWaiting_ = true;
      }
      // sleep for desired amount of time
      sleep(dcri_.retryInterval());
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
