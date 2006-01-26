/*
   Author: Harry Cheung, FNAL

   Description:
     Storage Manager XDAQ application. It can receive and collect
     I2O frames to remake event data and then processes them
     through an event processor. Two cmsRun style configuration 
     files are normally needed, one for the Storage Manager and
     one for the FUEventProcessor that produced the incoming
     events so that the registry can be made at startup.
     See CMS EventFilter wiki page for further notes.

   Modification:
     version 1.1 2005/11/23
       Initial implementation, the FUEventProcessor configuration
       is not used yet. This prototype uses a sample streamer
       data file that must be located at the running directory
       to get the registry. Also the SM config file name is also
       hardwired rather than read from the XML file.

*/

#include <exception>
#include <iostream>
#include <string>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/unistd.h>

#include "EventFilter/StorageManager/test/testStorageManager.h"

#include "FWCore/Framework/interface/EventProcessor.h"
#include "FWCore/Framework/interface/ProductRegistry.h"
#include "FWCore/Utilities/interface/ProblemTracker.h"
#include "FWCore/Utilities/interface/DebugMacros.h"
#include "FWCore/Utilities/interface/Exception.h"
#include "FWCore/MessageLogger/interface/MessageLoggerSpigot.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "IOPool/Streamer/interface/HLTInfo.h"
#include "IOPool/Streamer/interface/Utilities.h"
#include "IOPool/Streamer/interface/TestFileReader.h"
#include "EventFilter/StorageManager/interface/JobController.h"
#include "PluginManager/PluginManager.h"

#include "EventFilter/StorageManager/interface/i2oStorageManagerMsg.h"

#include "xcept/tools.h"

#include "i2o/Method.h"
#include "i2o/utils/include/i2o/utils/AddressMap.h"

#include "toolbox/mem/Pool.h"
#include "xcept/tools.h"
#include "xgi/Method.h"

#include <exception>
#include <iostream>

#include "boost/shared_ptr.hpp"

using namespace edm;
using namespace std;

// -----------------------------------------------

namespace {
  // should be moved into a common utility library
  // but we only need it for testing in this program

  string getFileContents(const string& conffile)
  {
    struct stat b;
    if(stat(conffile.c_str(),&b)<0)
      {
        cerr << "Cannot stat() file " << conffile << endl;
        abort();
      }
    
    fstream ist(conffile.c_str());
    if(!ist)
      {
        cerr << "Could not open file " << conffile << endl;
        abort();
      }
    
    string rc(b.st_size,' ');
    ist.getline(&rc[0],b.st_size,fstream::traits_type::eof());
    return rc;
  }
}


static void deleteSMBuffer(void* Ref)
{
  // release the memory pool buffer
  // once the fragment collector is done with it
  stor::FragEntry* entry = (stor::FragEntry*)Ref;
  toolbox::mem::Reference *ref=(toolbox::mem::Reference*)entry->buffer_object_;
  ref->release();
}
// -----------------------------------------------

testStorageManager::testStorageManager(xdaq::ApplicationStub * s)
  throw (xdaq::exception::Exception): xdaq::Application(s),
  fsm_(0)//, ah_(0)
{
  LOG4CPLUS_INFO(this->getApplicationLogger(),"Making testStorageManager");

  //ah_ = new edm::AssertHandler();
  fsm_ = new evf::EPStateMachine(getApplicationLogger());
  fsm_->init<testStorageManager>(this);
  xdata::InfoSpace *ispace = getApplicationInfoSpace();
  // default configuration
  // not yet using configuration in XML file, using instead a
  // file in the local area named SMConfig.cfg
  ispace->fireItemAvailable("STparameterSet",&offConfig_);
  ispace->fireItemAvailable("FUparameterSet",&fuConfig_);
  ispace->fireItemAvailable("stateName",&fsm_->stateName_);

  // Bind specific messages to functions
  i2o::bind(this,
            &testStorageManager::receiveRegistryMessage,
            I2O_SM_PREAMBLE,
            XDAQ_ORGANIZATION_ID);
  i2o::bind(this,
            &testStorageManager::receiveDataMessage,
            I2O_SM_DATA,
            XDAQ_ORGANIZATION_ID);
  i2o::bind(this,
            &testStorageManager::receiveOtherMessage,
            I2O_SM_OTHER,
            XDAQ_ORGANIZATION_ID);

  // Bind web interface
  xgi::bind(this,&testStorageManager::defaultWebPage, "Default");
  xgi::bind(this,&testStorageManager::css, "styles.css");


  eventcounter_ = 0;
  framecounter_ = 0;
  pool_is_set_ = 0;
  pool_ = 0;

 // for performance measurements
  samples_ = 100; // measurements every 25MB (about)
  databw_ = 0.;
  datarate_ = 0.;
  datalatency_ = 0.;
  totalsamples_ = 0;
  duration_ = 0.;
  meandatabw_ = 0.;
  meandatarate_ = 0.;
  meandatalatency_ = 0.;
  pmeter_ = new sto::SMPerformanceMeter();
  pmeter_->init(samples_);
  maxdatabw_ = 0.;
  mindatabw_ = 999999.;

}

testStorageManager::~testStorageManager()
{
  delete fsm_;
  //delete ah_;
}

void testStorageManager::configureAction(toolbox::Event::Reference e) 
  throw (toolbox::fsm::exception::Exception)
{
  // Get into the ready state
  // get the configuration here or in enable?

  // do this here? JBK - moved to data member
  // edm::MessageLoggerSpigot theMessageLoggerSpigot;
  seal::PluginManager::get()->initialise();

  //streamer format file to get registry
  // string sample_file("SMSampleFile.dat");
  // SM config file - should be from XML
  // string my_config_file("SMConfig.cfg");
  // for the real thing, give the JobController a configuration string and
  // save the registry data for checking with the one coming over the network

  string sample_config(fuConfig_.value_);
  string my_config(offConfig_.value_);

  // the rethrows below need to be XDAQ exception types (JBK)

  try {
    jc_.reset(new stor::JobController(sample_config,
                  my_config, &deleteSMBuffer));
  }
  catch(cms::Exception& e)
    {
      cerr << "Caught an exception:\n" << e.what() << endl;
      throw;
    }
  catch(seal::Error& e)
    {
      cerr << "Caught an exception:\n" << e.explainSelf() << endl;
      throw;
    }
  catch(std::exception& e)
    {
      cerr << "Caught an exception:\n" << e.what() << endl;
      throw;
    }
  catch(...)
  {
      cerr << "Caught unknown exception\n" << endl;
  }

}

void testStorageManager::enableAction(toolbox::Event::Reference e) 
  throw (toolbox::fsm::exception::Exception)
{
  // Get into running state
  jc_->start();
}

void testStorageManager::haltAction(toolbox::Event::Reference e) 
  throw (toolbox::fsm::exception::Exception)
{
  // Get into halted state
  jc_->stop();
  jc_->join();
  // we must destroy the eventprocessor to finish processing
  jc_.reset();
}

void testStorageManager::suspendAction(toolbox::Event::Reference e) 
  throw (toolbox::fsm::exception::Exception)
{
  // Get into suspend state: but not valid for Storage Manager?!
  LOG4CPLUS_INFO(this->getApplicationLogger(),
    "Suspend state not implemented in testStorageManager");
}

void testStorageManager::resumeAction(toolbox::Event::Reference e) 
  throw (toolbox::fsm::exception::Exception)
{
  // Get into suspend state: but not valid for Storage Manager?!
  LOG4CPLUS_INFO(this->getApplicationLogger(),
    "Resume not implemented in testStorageManager");
}

#include "xoap/include/xoap/SOAPEnvelope.h"
#include "xoap/include/xoap/SOAPBody.h"
#include "xoap/include/xoap/domutils.h"

xoap::MessageReference testStorageManager::fireEvent(xoap::MessageReference msg)
  throw (xoap::exception::Exception)
{
  xoap::SOAPPart     part      = msg->getSOAPPart();
  xoap::SOAPEnvelope env       = part.getEnvelope();
  xoap::SOAPBody     body      = env.getBody();
  DOMNode            *node     = body.getDOMNode();
  DOMNodeList        *bodyList = node->getChildNodes();
  DOMNode            *command  = 0;
  std::string        commandName;

  for (unsigned int i = 0; i < bodyList->getLength(); i++)
    {
      command = bodyList->item(i);

      if(command->getNodeType() == DOMNode::ELEMENT_NODE)
        {
          commandName = xoap::XMLCh2String(command->getLocalName());
          return fsm_->processFSMCommand(commandName);
        }
    }

  XCEPT_RAISE(xoap::exception::Exception, "Command not found");
}

////////////////////////////// I2O frame call back functions ///////////////

void testStorageManager::receiveRegistryMessage(toolbox::mem::Reference *ref)
{
  // get the pool pointer for statistics if not already set
  if(pool_is_set_ == 0)
  {
    pool_ = ref->getBuffer()->getPool();
   pool_is_set_ = 1;
  }

  I2O_MESSAGE_FRAME         *stdMsg =
    (I2O_MESSAGE_FRAME*)ref->getDataLocation();
  I2O_SM_PREAMBLE_MESSAGE_FRAME *msg    =
    (I2O_SM_PREAMBLE_MESSAGE_FRAME*)stdMsg;
  FDEBUG(10) << "testStorageManager: Received registry message from HLT " << msg->hltURL
             << " application " << msg->hltClassName << " id " << msg->hltLocalId
             << " instance " << msg->hltInstance << " tid " << msg->hltTid << std::endl;
  FDEBUG(10) << "testStorageManager: registry size " << msg->dataSize << "\n";
  // can get rid of this if not dumping the data for checking
  std::string temp4print(msg->data,msg->dataSize);
  FDEBUG(10) << "testStorageManager: registry data = " << temp4print << std::endl;
  // should be checking the registry with the one being used
  // release the frame buffer now that we are finished
  ref->release();
  
  framecounter_++;

  // for bandwidth performance measurements
  unsigned long actualFrameSize = (unsigned long)sizeof(I2O_SM_PREAMBLE_MESSAGE_FRAME);
  addMeasurement(actualFrameSize);

}

void testStorageManager::receiveDataMessage(toolbox::mem::Reference *ref)
{
  // get the pool pointer for statistics
  if(pool_is_set_ == 0)
  {
    pool_ = ref->getBuffer()->getPool();
   pool_is_set_ = 1;
  }

  I2O_MESSAGE_FRAME         *stdMsg =
    (I2O_MESSAGE_FRAME*)ref->getDataLocation();
  I2O_SM_DATA_MESSAGE_FRAME *msg    =
    (I2O_SM_DATA_MESSAGE_FRAME*)stdMsg;
  FDEBUG(10) << "testStorageManager: Received data message from HLT at " << msg->hltURL 
             << " application " << msg->hltClassName << " id " << msg->hltLocalId
             << " instance " << msg->hltInstance << " tid " << msg->hltTid << std::endl;
  FDEBUG(10) << "                 for run " << msg->runID << " event " << msg->eventID
             << " total frames = " << msg->numFrames << std::endl;
  FDEBUG(10) << "testStorageManager: Frame " << msg->frameCount << " of " 
             << msg->numFrames-1 << std::endl;
  int len = msg->dataSize;
  FDEBUG(10) << "testStorageManager: received data frame size = " << len << std::endl;

  // put pointers into fragment collector queue
  EventBuffer::ProducerBuffer b(jc_->getFragmentQueue());
  /* stor::FragEntry* fe = */ new (b.buffer()) stor::FragEntry(ref, msg->data, len);
  b.commit(sizeof(stor::FragEntry));

  // don't release buffers until the all frames from a chain are received
  // and JobController/FragmentCollector are finished with them.
  // This is done in the deleter.

  framecounter_++;

  // for bandwidth performance measurements
  // does not work for chains transferred locally!
  unsigned long actualFrameSize = (unsigned long)sizeof(I2O_SM_DATA_MESSAGE_FRAME);
  addMeasurement(actualFrameSize);
}

void testStorageManager::receiveOtherMessage(toolbox::mem::Reference *ref)
{
  // get the pool pointer for statistics
  if(pool_is_set_ == 0)
  {
    pool_ = ref->getBuffer()->getPool();
   pool_is_set_ = 1;
  }

  I2O_MESSAGE_FRAME         *stdMsg =
    (I2O_MESSAGE_FRAME*)ref->getDataLocation();
  I2O_SM_OTHER_MESSAGE_FRAME *msg    =
    (I2O_SM_OTHER_MESSAGE_FRAME*)stdMsg;
  FDEBUG(10) << "testStorageManager: Received other message from HLT " << msg->hltURL
             << " application " << msg->hltClassName << " id " << msg->hltLocalId
             << " instance " << msg->hltInstance << " tid " << msg->hltTid << std::endl;
  FDEBUG(10) << "testStorageManager: message content " << msg->otherData << "\n";

  // Not yet processing any Other messages type

  // release the frame buffer now that we are finished
  ref->release();

  framecounter_++;

  // for bandwidth performance measurements
  unsigned long actualFrameSize = (unsigned long)sizeof(I2O_SM_OTHER_MESSAGE_FRAME);
  addMeasurement(actualFrameSize);
}

////////////////////////////// Performance      ////////////////////////////
void testStorageManager::addMeasurement(unsigned long size)
{
  // for bandwidth performance measurements
  if ( pmeter_->addSample(size) )
  {
    LOG4CPLUS_INFO(getApplicationLogger(),
      toolbox::toString("measured latency: %f for size %d",pmeter_->latency(), size));
    LOG4CPLUS_INFO(getApplicationLogger(),
      toolbox::toString("latency:  %f, rate: %f,bandwidth %f, size: %d\n",
      pmeter_->latency(),pmeter_->rate(),pmeter_->bandwidth(),size));
    // new measurement; so update
    databw_ = pmeter_->bandwidth();
    datarate_ = pmeter_->rate();
    datalatency_ = pmeter_->latency();
    totalsamples_ = pmeter_->totalsamples();
    duration_ = pmeter_->duration();
    meandatabw_ = pmeter_->meanbandwidth();
    meandatarate_ = pmeter_->meanrate();
    meandatalatency_ = pmeter_->meanlatency();
    if(databw_ > maxdatabw_) maxdatabw_ = databw_;
    if(databw_ < mindatabw_) mindatabw_ = databw_;
  }
}

////////////////////////////// Default web page ////////////////////////////
#include <iomanip>

void testStorageManager::defaultWebPage(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception)
{
  *out << "<html>"                                                   << endl;
  *out << "<head>"                                                   << endl;
  *out << "<link type=\"text/css\" rel=\"stylesheet\"";
  *out << " href=\"/" <<  getApplicationDescriptor()->getURN()
       << "/styles.css\"/>"                   << endl;
  *out << "<title>" << getApplicationDescriptor()->getClassName() << " instance "
       << getApplicationDescriptor()->getInstance()
       << "</title>"     << endl;
    *out << "<table border=\"0\" width=\"100%\">"                      << endl;
    *out << "<tr>"                                                     << endl;
    *out << "  <td align=\"left\">"                                    << endl;
    *out << "    <img"                                                 << endl;
    *out << "     align=\"middle\""                                    << endl;
    *out << "     src=\"/daq/evb/examples/fu/images/fu64x64.gif\""     << endl;
    *out << "     alt=\"main\""                                        << endl;
    *out << "     width=\"64\""                                        << endl;
    *out << "     height=\"64\""                                       << endl;
    *out << "     border=\"\"/>"                                       << endl;
    *out << "    <b>"                                                  << endl;
    *out << getApplicationDescriptor()->getClassName() << " instance "
         << getApplicationDescriptor()->getInstance()                  << endl;
    *out << "    </b>"                                                 << endl;
    *out << "  </td>"                                                  << endl;
    *out << "  <td width=\"32\">"                                      << endl;
    *out << "    <a href=\"/urn:xdaq-application:lid=3\">"             << endl;
    *out << "      <img"                                               << endl;
    *out << "       align=\"middle\""                                  << endl;
    *out << "       src=\"/daq/xdaq/hyperdaq/images/HyperDAQ.jpg\""    << endl;
    *out << "       alt=\"HyperDAQ\""                                  << endl;
    *out << "       width=\"32\""                                      << endl;
    *out << "       height=\"32\""                                      << endl;
    *out << "       border=\"\"/>"                                     << endl;
    *out << "    </a>"                                                 << endl;
    *out << "  </td>"                                                  << endl;
    *out << "  <td width=\"32\">"                                      << endl;
    *out << "  </td>"                                                  << endl;
    *out << "  <td width=\"32\">"                                      << endl;
    *out << "    <a href=\"/" << getApplicationDescriptor()->getURN()
         << "/debug\">"                   << endl;
    *out << "      <img"                                               << endl;
    *out << "       align=\"middle\""                                  << endl;
    *out << "       src=\"/daq/evb/bu/images/debug32x32.gif\""         << endl;
    *out << "       alt=\"debug\""                                     << endl;
    *out << "       width=\"32\""                                      << endl;
    *out << "       height=\"32\""                                     << endl;
    *out << "       border=\"\"/>"                                     << endl;
    *out << "    </a>"                                                 << endl;
    *out << "  </td>"                                                  << endl;
    *out << "</tr>"                                                    << endl;
    *out << "</table>"                                                 << endl;

  *out << "<hr/>"                                                    << endl;
  *out << "<table>"                                                  << endl;
  *out << "<tr valign=\"top\">"                                      << endl;
  *out << "  <td>"                                                   << endl;

  *out << "<table frame=\"void\" rules=\"groups\" class=\"states\">" << std::endl;
  *out << "<colgroup> <colgroup align=\"rigth\">"                    << std::endl;
    *out << "  <tr>"                                                   << endl;
    *out << "    <th colspan=2>"                                       << endl;
    *out << "      " << "Memory Pool Usage"                            << endl;
    *out << "    </th>"                                                << endl;
    *out << "  </tr>"                                                  << endl;

        *out << "<tr>" << std::endl;
        *out << "<th >" << std::endl;
        *out << "Parameter" << std::endl;
        *out << "</th>" << std::endl;
        *out << "<th>" << std::endl;
        *out << "Value" << std::endl;
        *out << "</th>" << std::endl;
        *out << "</tr>" << std::endl;
        *out << "<tr>" << std::endl;
          *out << "<td >" << std::endl;
          *out << "Frames Received" << std::endl;
          *out << "</td>" << std::endl;
          *out << "<td align=right>" << std::endl;
          *out << framecounter_ << std::endl;
          *out << "</td>" << std::endl;
        *out << "  </tr>" << endl;
/*  Cannot get events yet from fragment collector
        *out << "<tr>" << std::endl;
          *out << "<td >" << std::endl;
          *out << "Events Received" << std::endl;
          *out << "</td>" << std::endl;
          *out << "<td align=right>" << std::endl;
          *out << eventcounter_ << std::endl;
          *out << "</td>" << std::endl;
        *out << "  </tr>" << endl;
*/
/*   can cause a crash as pool_ is not set until a frame is received!
        *out << "<tr>" << std::endl;
          *out << "<td >" << std::endl;
          *out << "Max Pool Memory (Bytes)" << std::endl;
          *out << "</td>" << std::endl;
          *out << "<td align=right>" << std::endl;
          *out << pool_->getMemoryUsage().getCommitted() << std::endl;
          *out << "</td>" << std::endl;
        *out << "  </tr>" << endl;
        *out << "<tr>" << std::endl;
          *out << "<td >" << std::endl;
          *out << "Memory Used (Bytes)" << std::endl;
          *out << "</td>" << std::endl;
          *out << "<td align=right>" << std::endl;
          *out << pool_->getMemoryUsage().getUsed() << std::endl;
          *out << "</td>" << std::endl;
        *out << "  </tr>" << endl;
        *out << "<tr>" << std::endl;
          *out << "<td >" << std::endl;
          *out << "Percent Memory Used (%)" << std::endl;
          *out << "</td>" << std::endl;
          *out << "<td align=right>" << std::endl;
          double memused = (double)pool_->getMemoryUsage().getUsed();
          double memmax = (double)pool_->getMemoryUsage().getCommitted();
          double percentused = 0.0;
          if(pool_->getMemoryUsage().getCommitted() != 0)
            percentused = 100.0*(memused/memmax);
          *out << std::fixed << std::showpoint
               << std::setw(6) << std::setprecision(2) << percentused << std::endl;
          *out << "</td>" << std::endl;
        *out << "  </tr>" << endl;
*/
// performance statistics
    *out << "  <tr>"                                                   << endl;
    *out << "    <th colspan=2>"                                       << endl;
    *out << "      " << "Performance for last " << samples_ << " frames"<< endl;
    *out << "    </th>"                                                << endl;
    *out << "  </tr>"                                                  << endl;
        *out << "<tr>" << std::endl;
          *out << "<td >" << std::endl;
          *out << "Bandwidth (MB/s)" << std::endl;
          *out << "</td>" << std::endl;
          *out << "<td align=right>" << std::endl;
          *out << databw_ << std::endl;
          *out << "</td>" << std::endl;
        *out << "  </tr>" << endl;
        *out << "<tr>" << std::endl;
          *out << "<td >" << std::endl;
          *out << "Rate (Frames/s)" << std::endl;
          *out << "</td>" << std::endl;
          *out << "<td align=right>" << std::endl;
          *out << datarate_ << std::endl;
          *out << "</td>" << std::endl;
        *out << "  </tr>" << endl;
        *out << "<tr>" << std::endl;
          *out << "<td >" << std::endl;
          *out << "Latency (us/frame)" << std::endl;
          *out << "</td>" << std::endl;
          *out << "<td align=right>" << std::endl;
          *out << datalatency_ << std::endl;
          *out << "</td>" << std::endl;
        *out << "  </tr>" << endl;
        *out << "<tr>" << std::endl;
          *out << "<td >" << std::endl;
          *out << "Maximum Bandwidth (MB/s)" << std::endl;
          *out << "</td>" << std::endl;
          *out << "<td align=right>" << std::endl;
          *out << maxdatabw_ << std::endl;
          *out << "</td>" << std::endl;
        *out << "  </tr>" << endl;
        *out << "<tr>" << std::endl;
          *out << "<td >" << std::endl;
          *out << "Minimum Bandwidth (MB/s)" << std::endl;
          *out << "</td>" << std::endl;
          *out << "<td align=right>" << std::endl;
          *out << mindatabw_ << std::endl;
          *out << "</td>" << std::endl;
        *out << "  </tr>" << endl;
// mean performance statistics for whole run
    *out << "  <tr>"                                                   << endl;
    *out << "    <th colspan=2>"                                       << endl;
    *out << "      " << "Mean Performance for " << totalsamples_ << " frames, duration "
         << duration_ << " seconds" << endl;
    *out << "    </th>"                                                << endl;
    *out << "  </tr>"                                                  << endl;
        *out << "<tr>" << std::endl;
          *out << "<td >" << std::endl;
          *out << "Bandwidth (MB/s)" << std::endl;
          *out << "</td>" << std::endl;
          *out << "<td align=right>" << std::endl;
          *out << meandatabw_ << std::endl;
          *out << "</td>" << std::endl;
        *out << "  </tr>" << endl;
        *out << "<tr>" << std::endl;
          *out << "<td >" << std::endl;
          *out << "Rate (Frames/s)" << std::endl;
          *out << "</td>" << std::endl;
          *out << "<td align=right>" << std::endl;
          *out << meandatarate_ << std::endl;
          *out << "</td>" << std::endl;
        *out << "  </tr>" << endl;
        *out << "<tr>" << std::endl;
          *out << "<td >" << std::endl;
          *out << "Latency (us/frame)" << std::endl;
          *out << "</td>" << std::endl;
          *out << "<td align=right>" << std::endl;
          *out << meandatalatency_ << std::endl;
          *out << "</td>" << std::endl;
        *out << "  </tr>" << endl;

  *out << "</table>" << std::endl;

  *out << "  </td>"                                                  << endl;
  *out << "</table>"                                                 << endl;

  *out << "</body>"                                                  << endl;
  *out << "</html>"                                                  << endl;
}


/**
 * Provides factory method for the instantiation of SM applications
 */

extern "C" xdaq::Application * instantiate_testStorageManager(xdaq::ApplicationStub * stub )
{
        std::cout << "Going to construct a testStorageManager instance " << endl;
        return new testStorageManager(stub);
}

