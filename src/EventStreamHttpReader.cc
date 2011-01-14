// $Id: CurlInterface.cc,v 1.2 2010/05/11 17:58:19 mommsen Exp $
/// @file: EventStreamHttpReader.cc

#include "EventFilter/StorageManager/src/EventStreamHttpReader.h"
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


namespace edm
{  
  EventStreamHttpReader::EventStreamHttpReader
  (
    edm::ParameterSet const& ps,
    edm::InputSourceDescription const& desc
  ):
  edm::StreamerInputSource(ps, desc),
  sourceurl_(ps.getParameter<std::string>("sourceURL")),
  consumerName_(ps.getUntrackedParameter<std::string>("consumerName","Unknown")),
  maxConnectTries_(ps.getUntrackedParameter<int>("maxConnectTries", 300)),
  connectTrySleepTime_(ps.getUntrackedParameter<int>("connectTrySleepTime", 10)),
  headerRetryInterval_(ps.getUntrackedParameter<int>("headerRetryInterval",5)),
  consumerId_(0),
  endRunAlreadyNotified_(true),
  runEnded_(false),
  alreadySaidHalted_(false),
  alreadySaidWaiting_(false)
  {
    // Default in StreamerInputSource is 'false'
    inputFileTransitionsEachEvent_ =
      ps.getUntrackedParameter<bool>("inputFileTransitionsEachEvent", true);
    
    double maxEventRequestRate = ps.getUntrackedParameter<double>("maxEventRequestRate",1.0);
    minEventRequestInterval_ = stor::utils::seconds_to_duration(
      maxEventRequestRate > 0 ? 
      1 / maxEventRequestRate :
      60
    );
    nextRequestTime_ = stor::utils::getCurrentTime() + minEventRequestInterval_;

    ParameterSet psCopy(ps);
    edm::ParameterSet selectEventsParamSet =
      ps.getUntrackedParameter("SelectEvents", edm::ParameterSet());
    if (! selectEventsParamSet.empty()) {
      Strings path_specs = 
        selectEventsParamSet.getParameter<Strings>("SelectEvents");
      if (! path_specs.empty()) {
        psCopy.addParameter<Strings>("TrackedEventSelection", path_specs);
      }
    }

    consumerPSetString_ = psCopy.toString(true);

    registerWithEventServer();

    readHeader();
  }

  EventStreamHttpReader::~EventStreamHttpReader()
  {
  }

  edm::EventPrincipal* EventStreamHttpReader::read()
  {
    // repeat a http get every N seconds until we get an event
    // wait for Storage Manager event server buffer to not be empty
    // only way to stop is specify a maxEvents parameter
    // or kill the Storage Manager so the http get fails.

    // try to get an event repeat until we get one, this allows
    // re-registration if the SM is halted or stopped

    bool gotEvent = false;
    edm::EventPrincipal* result = 0;
    while ((!gotEvent) && (!runEnded_) && (!edm::shutdown_flag))
    {
       result = getOneEvent();
       if(result != 0) gotEvent = true;
    }
    // need next line so we only return a null pointer once for each end of run
    if(runEnded_) runEnded_ = false;
    return result;
  }

  edm::EventPrincipal* EventStreamHttpReader::getOneEvent()
  {
    // repeat a http get every N seconds until we get an event
    // wait for Storage Manager event server buffer to not be empty
    // only way to stop is specify a maxEvents parameter or cntrol-c.
    // If the Storage Manager is killed so the http get fails, we
    // end the job as we would be in an unknown state (If SM is up
    // and we have a network problem we just try to get another event,
    // but if SM is killed/dead we want to register.)

    sleepUntilNextRequest();

    std::string data;
    
    do
    {
      data.clear();
      getOneEventFromEventServer(data);
    }
    while ( !edm::shutdown_flag && data.empty() );

    if ( edm::shutdown_flag ) return 0;
    return extractEvent(data);
  }
  
  
  void EventStreamHttpReader::getOneEventFromEventServer(std::string& data)
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
        //return std::auto_ptr<edm::EventPrincipal>();
        throw cms::Exception("getOneEvent","EventStreamHttpReader")
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
  
  
  edm::EventPrincipal* EventStreamHttpReader::extractEvent(std::string& data)
  {
    // first check if done message
    OtherMessageView msgView(&data[0]);

    if (msgView.code() == Header::DONE) {
      // decide if we need to notify that a run has ended
      if(!endRunAlreadyNotified_) {
        endRunAlreadyNotified_ = true;
        setEndRun();
        runEnded_ = true;
      }
      return 0;
    } else {
      // reset need-to-set-end-run flag when we get the first event (here any event)
      endRunAlreadyNotified_ = false;
      alreadySaidHalted_ = false;
      
      // 29-Jan-2008, KAB:  catch (and re-throw) any exceptions decoding
      // the event data so that we can display the returned HTML and
      // (hopefully) give the user a hint as to the cause of the problem.
      edm::EventPrincipal* evtPtr = 0;
      try {
        HeaderView hdrView(&data[0]);
        if (hdrView.code() != Header::EVENT) {
          throw cms::Exception("EventStreamHttpReader", "readOneEvent");
        }
        EventMsgView eventView(&data[0]);
        evtPtr = deserializeEvent(eventView);
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
      return evtPtr;
    }
  }
  
  
  inline void EventStreamHttpReader::sleepUntilNextRequest()
  {
    stor::utils::sleepUntil(nextRequestTime_);
    nextRequestTime_ = stor::utils::getCurrentTime() +
      minEventRequestInterval_;
  }
  
  
  void EventStreamHttpReader::readHeader()
  {
    std::string data;
    
    do
    {
      data.clear();
      getHeaderFromEventServer(data);
    }
    while ( !edm::shutdown_flag && !extractInitMsg(data) );
    
    if (edm::shutdown_flag) {
      throw cms::Exception("readHeader","EventStreamHttpReader")
        << "The header read was aborted by a shutdown request.\n";
    }
  }
  
  
  void EventStreamHttpReader::getHeaderFromEventServer(std::string& data)
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
  }


  bool EventStreamHttpReader::extractInitMsg(std::string& data)
  {
    if( data.empty() )
    {
      if(!alreadySaidWaiting_) {
        std::cout << "...waiting for header from Storage Manager..." << std::endl;
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
    
    try {
      HeaderView hdrView(&data[0]);
      if (hdrView.code() != Header::INIT) {
        throw cms::Exception("EventStreamHttpReader", "readHeader");
      }
      InitMsgView initView(&data[0]);
      deserializeAndMergeWithRegistry(initView);
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

    return true;
  }
  
  
  void EventStreamHttpReader::registerWithEventServer()
  {
    std::string data;

    do
    {
      data.clear();
      connectToEventServer(data);
    }
    while ( !edm::shutdown_flag && !extractConsumerId(data) );

    if (edm::shutdown_flag) {
      throw cms::Exception("registerWithEventServer","EventStreamHttpReader")
          << "Registration was aborted by a shutdown request.\n";
    }
  }
  
  
  void EventStreamHttpReader::connectToEventServer(std::string& data)
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
          throw cms::Exception("connectToEventServer","EventStreamHttpReader")
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
  
  
  bool EventStreamHttpReader::extractConsumerId(std::string& data)      
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
