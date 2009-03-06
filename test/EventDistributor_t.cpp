#include "Utilities/Testing/interface/CppUnit_testdriver.icpp"
#include "cppunit/extensions/HelperMacros.h"

#include "EventFilter/StorageManager/interface/EventDistributor.h"
#include "EventFilter/StorageManager/interface/EventStreamConfigurationInfo.h"
#include "EventFilter/StorageManager/interface/I2OChain.h"

#include "EventFilter/StorageManager/test/TestHelper.h"

#include "DataFormats/Common/interface/HLTenums.h"
#include "FWCore/ParameterSet/interface/PythonProcessDesc.h"


/////////////////////////////////////////////////////////////
//
// This test exercises the EventDistributor class
//
/////////////////////////////////////////////////////////////

using namespace stor;

using stor::testhelper::allocate_frame_with_basic_header;
using stor::testhelper::allocate_frame_with_init_msg;
using stor::testhelper::allocate_frame_with_event_msg;
using stor::testhelper::set_trigger_bit;
using stor::testhelper::clear_trigger_bits;

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

  boost::shared_ptr<InitMsgCollection> _initMsgCollection;
  boost::shared_ptr<EventDistributor> _eventDistributor;
};


void testEventDistributor::testInitMessages()
{
  if (_eventDistributor.get() == 0)
    {
      _initMsgCollection.reset(new InitMsgCollection());
      _eventDistributor.reset(new EventDistributor(_initMsgCollection));
    }

  CPPUNIT_ASSERT(_eventDistributor->configuredStreamCount() == 0);
  CPPUNIT_ASSERT(_eventDistributor->initializedStreamCount() == 0);
  CPPUNIT_ASSERT(_initMsgCollection->size() == 0);

  // *** specify configuration ***

  EventDistributor::StreamConfList cfgList =
    parseStreamConfig(getSampleStreamConfig());
  _eventDistributor->registerEventStreams(cfgList);

  CPPUNIT_ASSERT(_eventDistributor->configuredStreamCount() == 4);
  CPPUNIT_ASSERT(_eventDistributor->initializedStreamCount() == 0);
  CPPUNIT_ASSERT(_initMsgCollection->size() == 0);

  // *** first INIT message ***

  Reference* ref = allocate_frame_with_init_msg("out4DQM");
  stor::I2OChain initMsgFrag(ref);
  CPPUNIT_ASSERT(initMsgFrag.messageCode() == Header::INIT);

  _eventDistributor->addEventToRelevantQueues(initMsgFrag);

  CPPUNIT_ASSERT(_eventDistributor->configuredStreamCount() == 4);
  CPPUNIT_ASSERT(_eventDistributor->initializedStreamCount() == 1);
  CPPUNIT_ASSERT(_initMsgCollection->size() == 1);

  // *** second INIT message ***

  ref = allocate_frame_with_init_msg("HLTDEBUG");
  stor::I2OChain initMsgFrag2(ref);
  CPPUNIT_ASSERT(initMsgFrag2.messageCode() == Header::INIT);

  _eventDistributor->addEventToRelevantQueues(initMsgFrag2);

  CPPUNIT_ASSERT(_eventDistributor->configuredStreamCount() == 4);
  CPPUNIT_ASSERT(_eventDistributor->initializedStreamCount() == 2);
  CPPUNIT_ASSERT(_initMsgCollection->size() == 2);

  // *** third INIT message ***

  ref = allocate_frame_with_init_msg("CALIB");
  stor::I2OChain initMsgFrag3(ref);
  CPPUNIT_ASSERT(initMsgFrag3.messageCode() == Header::INIT);

  _eventDistributor->addEventToRelevantQueues(initMsgFrag3);

  CPPUNIT_ASSERT(_eventDistributor->configuredStreamCount() == 4);
  CPPUNIT_ASSERT(_eventDistributor->initializedStreamCount() == 3);
  CPPUNIT_ASSERT(_initMsgCollection->size() == 3);

  // *** duplicate INIT message ***

  ref = allocate_frame_with_init_msg("CALIB");
  stor::I2OChain initMsgFrag4(ref);
  CPPUNIT_ASSERT(initMsgFrag4.messageCode() == Header::INIT);

  _eventDistributor->addEventToRelevantQueues(initMsgFrag4);

  CPPUNIT_ASSERT(_eventDistributor->configuredStreamCount() == 4);
  CPPUNIT_ASSERT(_eventDistributor->initializedStreamCount() == 3);
  CPPUNIT_ASSERT(_initMsgCollection->size() == 3);

  // *** bogus INIT message ***

  ref = allocate_frame_with_init_msg("BOGUS");
  stor::I2OChain initMsgFrag5(ref);
  CPPUNIT_ASSERT(initMsgFrag5.messageCode() == Header::INIT);

  _eventDistributor->addEventToRelevantQueues(initMsgFrag5);

  CPPUNIT_ASSERT(_eventDistributor->configuredStreamCount() == 4);
  CPPUNIT_ASSERT(_eventDistributor->initializedStreamCount() == 3);
  CPPUNIT_ASSERT(_initMsgCollection->size() == 4);

  // *** cleanup ***

  _initMsgCollection->clear();
  _eventDistributor->clearEventStreams();

  CPPUNIT_ASSERT(_eventDistributor->configuredStreamCount() == 0);
  CPPUNIT_ASSERT(_eventDistributor->initializedStreamCount() == 0);
  CPPUNIT_ASSERT(_initMsgCollection->size() == 0);
}


void testEventDistributor::testEventSelection()
{
  if (_eventDistributor.get() == 0)
    {
      _initMsgCollection.reset(new InitMsgCollection());
      _eventDistributor.reset(new EventDistributor(_initMsgCollection));
    }

  CPPUNIT_ASSERT(_eventDistributor->configuredStreamCount() == 0);
  CPPUNIT_ASSERT(_eventDistributor->initializedStreamCount() == 0);
  CPPUNIT_ASSERT(_initMsgCollection->size() == 0);

  // *** specify configuration ***

  EventDistributor::StreamConfList cfgList =
    parseStreamConfig(getSampleStreamConfig());
  _eventDistributor->registerEventStreams(cfgList);

  CPPUNIT_ASSERT(_eventDistributor->configuredStreamCount() == 4);
  CPPUNIT_ASSERT(_eventDistributor->initializedStreamCount() == 0);
  CPPUNIT_ASSERT(_initMsgCollection->size() == 0);

  // *** first INIT message ***

  Reference* ref = allocate_frame_with_init_msg("HLTDEBUG");
  stor::I2OChain initMsgFrag(ref);
  CPPUNIT_ASSERT(initMsgFrag.messageCode() == Header::INIT);

  _eventDistributor->addEventToRelevantQueues(initMsgFrag);

  CPPUNIT_ASSERT(_eventDistributor->configuredStreamCount() == 4);
  CPPUNIT_ASSERT(_eventDistributor->initializedStreamCount() == 1);
  CPPUNIT_ASSERT(_initMsgCollection->size() == 1);

  CPPUNIT_ASSERT(_eventDistributor->configuredStreamCount() == 4);
  CPPUNIT_ASSERT(_eventDistributor->initializedStreamCount() == 1);
  CPPUNIT_ASSERT(_initMsgCollection->size() == 1);

  // *** HLT trigger bit tests ***
  std::vector<unsigned char> hltBits;
  CPPUNIT_ASSERT(hltBits.size() == 0);

  set_trigger_bit(hltBits, 8, edm::hlt::Ready);

  CPPUNIT_ASSERT(hltBits.size() == 3);
  CPPUNIT_ASSERT(hltBits[2] == 0);

  // *** first event message (should pass) ***

  uint32 eventNumber = 1;
  uint32 hltBitCount = 9;
  clear_trigger_bits(hltBits);
  set_trigger_bit(hltBits, 0, edm::hlt::Pass);

  ref = allocate_frame_with_event_msg("HLTDEBUG", hltBits, hltBitCount,
                                      eventNumber);
  stor::I2OChain eventMsgFrag(ref);
  CPPUNIT_ASSERT(eventMsgFrag.messageCode() == Header::EVENT);

  CPPUNIT_ASSERT(!eventMsgFrag.isTaggedForAnyEventStream());
  CPPUNIT_ASSERT(!eventMsgFrag.isTaggedForAnyEventConsumer());
  CPPUNIT_ASSERT(!eventMsgFrag.isTaggedForAnyDQMEventConsumer());

  _eventDistributor->addEventToRelevantQueues(eventMsgFrag);

  CPPUNIT_ASSERT(_eventDistributor->configuredStreamCount() == 4);
  CPPUNIT_ASSERT(_eventDistributor->initializedStreamCount() == 1);
  CPPUNIT_ASSERT(_initMsgCollection->size() == 1);

  CPPUNIT_ASSERT(eventMsgFrag.isTaggedForAnyEventStream());
  CPPUNIT_ASSERT(!eventMsgFrag.isTaggedForAnyEventConsumer());
  CPPUNIT_ASSERT(!eventMsgFrag.isTaggedForAnyDQMEventConsumer());

  std::vector<StreamID> streamIdList = eventMsgFrag.getEventStreamTags();
  CPPUNIT_ASSERT(streamIdList.size() == 1);
  CPPUNIT_ASSERT(streamIdList[0] == 2);

  // *** second event message (should not pass) ***

  clear_trigger_bits(hltBits);
  set_trigger_bit(hltBits, 0, edm::hlt::Fail);
  set_trigger_bit(hltBits, 2, edm::hlt::Exception);
  set_trigger_bit(hltBits, 4, edm::hlt::Pass);

  ++eventNumber;
  ref = allocate_frame_with_event_msg("HLTDEBUG", hltBits, hltBitCount,
                                      eventNumber);
  stor::I2OChain eventMsgFrag2(ref);
  CPPUNIT_ASSERT(eventMsgFrag2.messageCode() == Header::EVENT);

  CPPUNIT_ASSERT(!eventMsgFrag2.isTaggedForAnyEventStream());
  CPPUNIT_ASSERT(!eventMsgFrag2.isTaggedForAnyEventConsumer());
  CPPUNIT_ASSERT(!eventMsgFrag2.isTaggedForAnyDQMEventConsumer());

  _eventDistributor->addEventToRelevantQueues(eventMsgFrag2);

  CPPUNIT_ASSERT(_eventDistributor->configuredStreamCount() == 4);
  CPPUNIT_ASSERT(_eventDistributor->initializedStreamCount() == 1);
  CPPUNIT_ASSERT(_initMsgCollection->size() == 1);

  CPPUNIT_ASSERT(!eventMsgFrag2.isTaggedForAnyEventStream());
  CPPUNIT_ASSERT(!eventMsgFrag2.isTaggedForAnyEventConsumer());
  CPPUNIT_ASSERT(!eventMsgFrag2.isTaggedForAnyDQMEventConsumer());

  // *** third event message (should not pass) ***

  clear_trigger_bits(hltBits);
  set_trigger_bit(hltBits, 0, edm::hlt::Pass);

  ++eventNumber;
  ref = allocate_frame_with_event_msg("BOGUS", hltBits, hltBitCount,
                                      eventNumber);
  stor::I2OChain eventMsgFrag3(ref);
  CPPUNIT_ASSERT(eventMsgFrag3.messageCode() == Header::EVENT);

  CPPUNIT_ASSERT(!eventMsgFrag3.isTaggedForAnyEventStream());
  CPPUNIT_ASSERT(!eventMsgFrag3.isTaggedForAnyEventConsumer());
  CPPUNIT_ASSERT(!eventMsgFrag3.isTaggedForAnyDQMEventConsumer());

  _eventDistributor->addEventToRelevantQueues(eventMsgFrag3);

  CPPUNIT_ASSERT(_eventDistributor->configuredStreamCount() == 4);
  CPPUNIT_ASSERT(_eventDistributor->initializedStreamCount() == 1);
  CPPUNIT_ASSERT(_initMsgCollection->size() == 1);

  CPPUNIT_ASSERT(!eventMsgFrag3.isTaggedForAnyEventStream());
  CPPUNIT_ASSERT(!eventMsgFrag3.isTaggedForAnyEventConsumer());
  CPPUNIT_ASSERT(!eventMsgFrag3.isTaggedForAnyDQMEventConsumer());

  // *** cleanup ***

  _initMsgCollection->clear();
  _eventDistributor->clearEventStreams();

  CPPUNIT_ASSERT(_eventDistributor->configuredStreamCount() == 0);
  CPPUNIT_ASSERT(_eventDistributor->initializedStreamCount() == 0);
  CPPUNIT_ASSERT(_initMsgCollection->size() == 0);
}



EventDistributor::StreamConfList testEventDistributor::parseStreamConfig(std::string cfgString)
{
  EventDistributor::StreamConfList streamCfgList;
  PythonProcessDesc py_pdesc(cfgString.c_str());
  boost::shared_ptr<edm::ProcessDesc> pdesc = py_pdesc.processDesc();
  boost::shared_ptr<edm::ParameterSet> smPSet = pdesc->getProcessPSet();

  // loop over each end path
  size_t streamId = 0;
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
          cfgInfo.setStreamId(++streamId);
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
