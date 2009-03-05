#include "Utilities/Testing/interface/CppUnit_testdriver.icpp"
#include "cppunit/extensions/HelperMacros.h"

#include "EventFilter/StorageManager/interface/EventDistributor.h"
#include "EventFilter/StorageManager/interface/EventStreamConfigurationInfo.h"
#include "EventFilter/StorageManager/interface/I2OChain.h"

#include "EventFilter/StorageManager/test/TestHelper.h"

#include "IOPool/Streamer/interface/InitMsgBuilder.h"
#include "IOPool/Streamer/interface/EventMsgBuilder.h"

#include "FWCore/ParameterSet/interface/PythonProcessDesc.h"

#include <iostream>
#include "zlib.h"


/////////////////////////////////////////////////////////////
//
// This test exercises the EventDistributor class
//
/////////////////////////////////////////////////////////////

using namespace stor;

using stor::testhelper::allocate_frame_with_basic_header;

class testEventDistributor : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(testEventDistributor);
  CPPUNIT_TEST(testInitMessages);
  CPPUNIT_TEST(testEventSelection);

  CPPUNIT_TEST_SUITE_END();

public:
  void testInitMessages();
  void testEventSelection();

private:
  EventDistributor::StreamConfList parseStreamConfig(std::string cfgString);
  std::string getSampleStreamConfig();

  EventDistributor _eventDistributor;
};

void testEventDistributor::testInitMessages()
{
  CPPUNIT_ASSERT(_eventDistributor.configuredStreamCount() == 0);
  CPPUNIT_ASSERT(_eventDistributor.initializedStreamCount() == 0);

  EventDistributor::StreamConfList cfgList =
    parseStreamConfig(getSampleStreamConfig());
  _eventDistributor.registerEventStreams(cfgList);

  CPPUNIT_ASSERT(_eventDistributor.configuredStreamCount() == 4);
  CPPUNIT_ASSERT(_eventDistributor.initializedStreamCount() == 0);

  char psetid[] = "1234567890123456";
  Strings hlt_names;
  Strings hlt_selections;
  Strings l1_names;

  hlt_names.push_back("a");  hlt_names.push_back("b");
  hlt_names.push_back("c");  hlt_names.push_back("d");
  hlt_names.push_back("e");  hlt_names.push_back("f");
  hlt_names.push_back("g");  hlt_names.push_back("h");
  hlt_names.push_back("i");

  hlt_selections.push_back("a");
  hlt_selections.push_back("c");
  hlt_selections.push_back("e");
  hlt_selections.push_back("g");
  hlt_selections.push_back("i");

  l1_names.push_back("t10");  l1_names.push_back("t11");

  char reltag[]="CMSSW_3_0_0_pre7";
  std::string processName = "HLT";
  std::string outputModuleLabel = "out4DQM";

  uLong crc = crc32(0L, Z_NULL, 0);
  Bytef* crcbuf = (Bytef*) outputModuleLabel.data();
  unsigned int outputModuleId = crc32(crc,crcbuf,outputModuleLabel.length());

  unsigned int value1 = 0xa5a5d2d2;
  unsigned int value2 = 0xb4b4e1e1;
  unsigned int value3 = 0xc3c3f0f0;

  Reference* ref = allocate_frame_with_basic_header(I2O_SM_PREAMBLE, 0, 1);
  I2O_SM_PREAMBLE_MESSAGE_FRAME *smMsg =
    (I2O_SM_PREAMBLE_MESSAGE_FRAME*) ref->getDataLocation();
  smMsg->hltTid = value1;
  smMsg->rbBufferID = 2;
  smMsg->outModID = outputModuleId;
  smMsg->fuProcID = value2;
  smMsg->fuGUID = value3;

  InitMsgBuilder
    initBuilder(smMsg->dataPtr(), smMsg->dataSize, 100,
                Version(7,(const uint8*)psetid), (const char*) reltag,
                processName.c_str(), outputModuleLabel.c_str(),
                outputModuleId, hlt_names, hlt_selections, l1_names);

  stor::I2OChain initMsgFrag(ref);
  CPPUNIT_ASSERT(initMsgFrag.messageCode() == Header::INIT);

  _eventDistributor.addEventToRelevantQueues(initMsgFrag);

  CPPUNIT_ASSERT(_eventDistributor.configuredStreamCount() == 4);
  CPPUNIT_ASSERT(_eventDistributor.initializedStreamCount() == 1);

  _eventDistributor.clearEventStreams();

  CPPUNIT_ASSERT(_eventDistributor.configuredStreamCount() == 0);
  CPPUNIT_ASSERT(_eventDistributor.initializedStreamCount() == 0);
}

void testEventDistributor::testEventSelection()
{
  CPPUNIT_ASSERT(_eventDistributor.configuredStreamCount() == 0);
  CPPUNIT_ASSERT(_eventDistributor.initializedStreamCount() == 0);

  EventDistributor::StreamConfList cfgList =
    parseStreamConfig(getSampleStreamConfig());
  _eventDistributor.registerEventStreams(cfgList);

  CPPUNIT_ASSERT(_eventDistributor.configuredStreamCount() == 4);
  CPPUNIT_ASSERT(_eventDistributor.initializedStreamCount() == 0);

  _eventDistributor.clearEventStreams();

  CPPUNIT_ASSERT(_eventDistributor.configuredStreamCount() == 0);
  CPPUNIT_ASSERT(_eventDistributor.initializedStreamCount() == 0);
}



EventDistributor::StreamConfList testEventDistributor::parseStreamConfig(std::string cfgString)
{
  EventDistributor::StreamConfList streamCfgList;
  PythonProcessDesc py_pdesc(cfgString.c_str());
  boost::shared_ptr<edm::ProcessDesc> pdesc = py_pdesc.processDesc();
  boost::shared_ptr<edm::ParameterSet> smPSet = pdesc->getProcessPSet();

  // loop over each end path
  std::vector<std::string> allEndPaths = 
    smPSet->getParameter<std::vector<std::string> >("@end_paths");
  for(std::vector<std::string>::iterator endPathIter = allEndPaths.begin();
      endPathIter != allEndPaths.end(); ++endPathIter) {

    // loop over each element in the end path list (not sure why...)
    std::vector<std::string> anEndPath =
      smPSet->getParameter<std::vector<std::string> >((*endPathIter));
    for(std::vector<std::string>::iterator ep2Iter = anEndPath.begin();
        ep2Iter != anEndPath.end(); ++ep2Iter) {

      // fetch the end path parameter set
      edm::ParameterSet endPathPSet =
        smPSet->getParameter<edm::ParameterSet>((*ep2Iter));
      if (! endPathPSet.empty()) {
        std::string mod_type =
          endPathPSet.getParameter<std::string> ("@module_type");
        if (mod_type == "EventStreamFileWriter" ||
            mod_type == "ErrorStreamFileWriter" ||
            mod_type == "FRDStreamFileWriter") {

          std::string streamLabel =
            endPathPSet.getParameter<std::string> ("streamLabel");
          long long maxFileSize =
            1048576 * (long long) endPathPSet.getParameter<int> ("maxSize");
          EventStreamConfigurationInfo::FilterList requestedEvents =
            edm::EventSelector::getEventSelectionVString(endPathPSet);
          std::string requestedOMLabel =
            endPathPSet.getUntrackedParameter<std::string>("SelectHLTOutput",
                                                           std::string());
          bool useCompression =
            endPathPSet.getUntrackedParameter<bool>("use_compression", true);
          unsigned int compressionLevel =
            endPathPSet.getUntrackedParameter<int>("compression_level", 1);
          unsigned int maxEventSize =
            endPathPSet.getUntrackedParameter<int>("max_event_size", 7000000);

          EventStreamConfigurationInfo cfgInfo(streamLabel,
                                               maxFileSize,
                                               requestedEvents,
                                               requestedOMLabel,
                                               useCompression,
                                               compressionLevel,
                                               maxEventSize);
          streamCfgList.push_back(cfgInfo);
        }
      }
    }
  }

  return streamCfgList;
}

std::string testEventDistributor::getSampleStreamConfig()
{
  // should we hard-code this or get it from a file?

  std::stringstream msg;
  msg << "import FWCore.ParameterSet.Config as cms" << std::endl;

  msg << "process = cms.Process(\"SM\")" << std::endl;
  msg << "process.source = cms.Source(\"FragmentInput\")" << std::endl;

  msg << "process.out1 = cms.OutputModule(\"EventStreamFileWriter\"," << std::endl;
  msg << "                                streamLabel = cms.string('A')," << std::endl;
  msg << "                                maxSize = cms.int32(20)," << std::endl;
  msg << "                                SelectHLTOutput = cms.untracked.string('out4DQM')" << std::endl;
  msg << "                                )" << std::endl;

  msg << "process.out2 = cms.OutputModule(\"EventStreamFileWriter\"," << std::endl;
  msg << "                                streamLabel = cms.string('B')," << std::endl;
  msg << "                                maxSize = cms.int32(40)," << std::endl;
  msg << "                                SelectEvents = cms.untracked.PSet( SelectEvents = " << std::endl;
  msg << "                                    cms.vstring('a', 'b', 'c', 'd') )," << std::endl;
  msg << "                                SelectHLTOutput = cms.untracked.string('HLTDEBUG')" << std::endl;
  msg << "                                )" << std::endl;

  msg << "process.out3 = cms.OutputModule(\"EventStreamFileWriter\"," << std::endl;
  msg << "                                streamLabel = cms.string('C')," << std::endl;
  msg << "                                maxSize = cms.int32(60)," << std::endl;
  msg << "                                SelectEvents = cms.untracked.PSet( SelectEvents = " << std::endl;
  msg << "                                    cms.vstring('e') )," << std::endl;
  msg << "                                SelectHLTOutput = cms.untracked.string('CALIB')" << std::endl;
  msg << "                                )" << std::endl;

  msg << "process.out4 = cms.OutputModule(\"ErrorStreamFileWriter\"," << std::endl;
  msg << "                                streamLabel = cms.string('Error')," << std::endl;
  msg << "                                maxSize = cms.int32(1)" << std::endl;
  msg << "                                )" << std::endl;

  msg << "process.end1 = cms.EndPath(process.out1)" << std::endl;
  msg << "process.end2 = cms.EndPath(process.out2)" << std::endl;
  msg << "process.end3 = cms.EndPath(process.out3)" << std::endl;
  msg << "process.end4 = cms.EndPath(process.out4)" << std::endl;

  return msg.str();
}


// This macro writes the 'main' for this test.
CPPUNIT_TEST_SUITE_REGISTRATION(testEventDistributor);


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
