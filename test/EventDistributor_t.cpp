#include "Utilities/Testing/interface/CppUnit_testdriver.icpp"
#include "cppunit/extensions/HelperMacros.h"

#include "EventFilter/StorageManager/interface/EnquingPolicyTag.h"
#include "EventFilter/StorageManager/interface/EventDistributor.h"
#include "EventFilter/StorageManager/interface/EventStreamConfigurationInfo.h"
#include "EventFilter/StorageManager/interface/I2OChain.h"
#include "EventFilter/StorageManager/interface/QueueID.h"

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
using stor::testhelper::allocate_frame_with_error_msg;
using stor::testhelper::allocate_frame_with_dqm_msg;
using stor::testhelper::set_trigger_bit;
using stor::testhelper::clear_trigger_bits;

class testEventDistributor : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(testEventDistributor);
  CPPUNIT_TEST(testInitMessages);
  CPPUNIT_TEST(testStreamSelection);
  CPPUNIT_TEST(testConsumerSelection);
  CPPUNIT_TEST( testDQMMessages );

  CPPUNIT_TEST_SUITE_END();

public:
  void testInitMessages();
  void testStreamSelection();
  void testConsumerSelection();
  void testDQMMessages();

private:
  void parseStreamConfigs(std::string cfgString,
                          EventDistributor::EvtStrConfList& eventConfigs,
                          EventDistributor::ErrStrConfList& errorConfigs);
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
  CPPUNIT_ASSERT(_eventDistributor->configuredConsumerCount() == 0);
  CPPUNIT_ASSERT(_eventDistributor->initializedConsumerCount() == 0);

  // *** specify configuration ***

  EventDistributor::EvtStrConfList evtCfgList;
  EventDistributor::ErrStrConfList errCfgList;
  parseStreamConfigs(getSampleStreamConfig(), evtCfgList, errCfgList);
  _eventDistributor->registerEventStreams(evtCfgList);
  _eventDistributor->registerErrorStreams(errCfgList);

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
  CPPUNIT_ASSERT(_eventDistributor->configuredConsumerCount() == 0);
  CPPUNIT_ASSERT(_eventDistributor->initializedConsumerCount() == 0);

  // *** cleanup ***

  _initMsgCollection->clear();
  _eventDistributor->clearStreams();

  CPPUNIT_ASSERT(_eventDistributor->configuredStreamCount() == 0);
  CPPUNIT_ASSERT(_eventDistributor->initializedStreamCount() == 0);
  CPPUNIT_ASSERT(_initMsgCollection->size() == 0);
  CPPUNIT_ASSERT(_eventDistributor->configuredConsumerCount() == 0);
  CPPUNIT_ASSERT(_eventDistributor->initializedConsumerCount() == 0);
}


void testEventDistributor::testStreamSelection()
{
  if (_eventDistributor.get() == 0)
    {
      _initMsgCollection.reset(new InitMsgCollection());
      _eventDistributor.reset(new EventDistributor(_initMsgCollection));
    }

  CPPUNIT_ASSERT(_eventDistributor->configuredStreamCount() == 0);
  CPPUNIT_ASSERT(_eventDistributor->initializedStreamCount() == 0);
  CPPUNIT_ASSERT(_initMsgCollection->size() == 0);
  CPPUNIT_ASSERT(_eventDistributor->configuredConsumerCount() == 0);
  CPPUNIT_ASSERT(_eventDistributor->initializedConsumerCount() == 0);

  // *** specify configuration ***

  EventDistributor::EvtStrConfList evtCfgList;
  EventDistributor::ErrStrConfList errCfgList;
  parseStreamConfigs(getSampleStreamConfig(), evtCfgList, errCfgList);
  _eventDistributor->registerEventStreams(evtCfgList);
  _eventDistributor->registerErrorStreams(errCfgList);

  CPPUNIT_ASSERT(_eventDistributor->configuredStreamCount() == 4);
  CPPUNIT_ASSERT(_eventDistributor->initializedStreamCount() == 0);
  CPPUNIT_ASSERT(_initMsgCollection->size() == 0);

  // *** INIT message ***

  Reference* ref = allocate_frame_with_init_msg("HLTDEBUG");
  stor::I2OChain initMsgFrag(ref);
  CPPUNIT_ASSERT(initMsgFrag.messageCode() == Header::INIT);

  _eventDistributor->addEventToRelevantQueues(initMsgFrag);

  CPPUNIT_ASSERT(_eventDistributor->configuredStreamCount() == 4);
  CPPUNIT_ASSERT(_eventDistributor->initializedStreamCount() == 1);
  CPPUNIT_ASSERT(_initMsgCollection->size() == 1);

  // *** HLT trigger bit tests ***
  std::vector<unsigned char> hltBits;
  CPPUNIT_ASSERT(hltBits.size() == 0);

  set_trigger_bit(hltBits, 8, edm::hlt::Ready);

  CPPUNIT_ASSERT(hltBits.size() == 3);
  CPPUNIT_ASSERT(hltBits[2] == 0);

  set_trigger_bit(hltBits, 3, edm::hlt::Fail);
  set_trigger_bit(hltBits, 5, edm::hlt::Pass);
  set_trigger_bit(hltBits, 8, edm::hlt::Exception);
  CPPUNIT_ASSERT(hltBits[0] == 0x80);
  CPPUNIT_ASSERT(hltBits[1] == 0x4);
  CPPUNIT_ASSERT(hltBits[2] == 0x3);

  // *** first event message (should pass) ***

  uint32 eventNumber = 1;
  uint32 hltBitCount = 9;
  clear_trigger_bits(hltBits);
  set_trigger_bit(hltBits, 0, edm::hlt::Pass);

  ref = allocate_frame_with_event_msg("HLTDEBUG", hltBits, hltBitCount,
                                      eventNumber);
  stor::I2OChain eventMsgFrag(ref);
  CPPUNIT_ASSERT(eventMsgFrag.messageCode() == Header::EVENT);

  CPPUNIT_ASSERT(!eventMsgFrag.isTaggedForAnyStream());
  CPPUNIT_ASSERT(!eventMsgFrag.isTaggedForAnyEventConsumer());
  CPPUNIT_ASSERT(!eventMsgFrag.isTaggedForAnyDQMEventConsumer());

  _eventDistributor->addEventToRelevantQueues(eventMsgFrag);

  CPPUNIT_ASSERT(_eventDistributor->configuredStreamCount() == 4);
  CPPUNIT_ASSERT(_eventDistributor->initializedStreamCount() == 1);
  CPPUNIT_ASSERT(_initMsgCollection->size() == 1);

  CPPUNIT_ASSERT(eventMsgFrag.isTaggedForAnyStream());
  CPPUNIT_ASSERT(!eventMsgFrag.isTaggedForAnyEventConsumer());
  CPPUNIT_ASSERT(!eventMsgFrag.isTaggedForAnyDQMEventConsumer());

  std::vector<StreamID> streamIdList = eventMsgFrag.getStreamTags();
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

  CPPUNIT_ASSERT(!eventMsgFrag2.isTaggedForAnyStream());
  CPPUNIT_ASSERT(!eventMsgFrag2.isTaggedForAnyEventConsumer());
  CPPUNIT_ASSERT(!eventMsgFrag2.isTaggedForAnyDQMEventConsumer());

  _eventDistributor->addEventToRelevantQueues(eventMsgFrag2);

  CPPUNIT_ASSERT(_eventDistributor->configuredStreamCount() == 4);
  CPPUNIT_ASSERT(_eventDistributor->initializedStreamCount() == 1);
  CPPUNIT_ASSERT(_initMsgCollection->size() == 1);

  CPPUNIT_ASSERT(!eventMsgFrag2.isTaggedForAnyStream());
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

  CPPUNIT_ASSERT(!eventMsgFrag3.isTaggedForAnyStream());
  CPPUNIT_ASSERT(!eventMsgFrag3.isTaggedForAnyEventConsumer());
  CPPUNIT_ASSERT(!eventMsgFrag3.isTaggedForAnyDQMEventConsumer());

  _eventDistributor->addEventToRelevantQueues(eventMsgFrag3);

  CPPUNIT_ASSERT(_eventDistributor->configuredStreamCount() == 4);
  CPPUNIT_ASSERT(_eventDistributor->initializedStreamCount() == 1);
  CPPUNIT_ASSERT(_initMsgCollection->size() == 1);

  CPPUNIT_ASSERT(!eventMsgFrag3.isTaggedForAnyStream());
  CPPUNIT_ASSERT(!eventMsgFrag3.isTaggedForAnyEventConsumer());
  CPPUNIT_ASSERT(!eventMsgFrag3.isTaggedForAnyDQMEventConsumer());

  // *** first error message (should pass) ***

  ++eventNumber;
  ref = allocate_frame_with_error_msg(eventNumber);
  stor::I2OChain errorMsgFrag(ref);
  CPPUNIT_ASSERT(errorMsgFrag.messageCode() == Header::ERROR_EVENT);

  CPPUNIT_ASSERT(!errorMsgFrag.isTaggedForAnyStream());
  CPPUNIT_ASSERT(!errorMsgFrag.isTaggedForAnyEventConsumer());
  CPPUNIT_ASSERT(!errorMsgFrag.isTaggedForAnyDQMEventConsumer());

  _eventDistributor->addEventToRelevantQueues(errorMsgFrag);

  CPPUNIT_ASSERT(_eventDistributor->configuredStreamCount() == 4);
  CPPUNIT_ASSERT(_eventDistributor->initializedStreamCount() == 1);
  CPPUNIT_ASSERT(_initMsgCollection->size() == 1);

  CPPUNIT_ASSERT(errorMsgFrag.isTaggedForAnyStream());
  CPPUNIT_ASSERT(!errorMsgFrag.isTaggedForAnyEventConsumer());
  CPPUNIT_ASSERT(!errorMsgFrag.isTaggedForAnyDQMEventConsumer());

  streamIdList = errorMsgFrag.getStreamTags();
  CPPUNIT_ASSERT(streamIdList.size() == 1);
  CPPUNIT_ASSERT(streamIdList[0] == 4);

  CPPUNIT_ASSERT(_eventDistributor->configuredConsumerCount() == 0);
  CPPUNIT_ASSERT(_eventDistributor->initializedConsumerCount() == 0);

  // *** cleanup ***

  _initMsgCollection->clear();
  _eventDistributor->clearStreams();

  CPPUNIT_ASSERT(_eventDistributor->configuredStreamCount() == 0);
  CPPUNIT_ASSERT(_eventDistributor->initializedStreamCount() == 0);
  CPPUNIT_ASSERT(_initMsgCollection->size() == 0);
  CPPUNIT_ASSERT(_eventDistributor->configuredConsumerCount() == 0);
  CPPUNIT_ASSERT(_eventDistributor->initializedConsumerCount() == 0);
}


void testEventDistributor::testConsumerSelection()
{
  if (_eventDistributor.get() == 0)
    {
      _initMsgCollection.reset(new InitMsgCollection());
      _eventDistributor.reset(new EventDistributor(_initMsgCollection));
    }

  CPPUNIT_ASSERT(_eventDistributor->configuredConsumerCount() == 0);
  CPPUNIT_ASSERT(_eventDistributor->initializedConsumerCount() == 0);
  CPPUNIT_ASSERT(_initMsgCollection->size() == 0);
  CPPUNIT_ASSERT(_eventDistributor->configuredStreamCount() == 0);
  CPPUNIT_ASSERT(_eventDistributor->initializedStreamCount() == 0);

  // *** first consumer ***

  Strings selections;
  boost::shared_ptr<EventConsumerRegistrationInfo> consInfo;

  {
    selections.clear();
    selections.push_back("a");
    selections.push_back("b");
    consInfo.reset(new EventConsumerRegistrationInfo(
        "http://cmswn1340.fnal.gov:52985/urn:xdaq-application:lid=29",
        5, 5, "Test Consumer", 5, 10, selections, "out4DQM", 120,
        stor::enquing_policy::DiscardOld));
    QueueID queueId(enquing_policy::DiscardOld, 1);
    consInfo->setQueueId(queueId);
    _eventDistributor->registerEventConsumer(&(*consInfo));
    
    CPPUNIT_ASSERT(_eventDistributor->configuredConsumerCount() == 1);
    CPPUNIT_ASSERT(_eventDistributor->initializedConsumerCount() == 0);
    CPPUNIT_ASSERT(_initMsgCollection->size() == 0);
  }

  // *** INIT message ***

  Reference* ref = allocate_frame_with_init_msg("out4DQM");
  stor::I2OChain initMsgFrag(ref);
  CPPUNIT_ASSERT(initMsgFrag.messageCode() == Header::INIT);

  _eventDistributor->addEventToRelevantQueues(initMsgFrag);

  CPPUNIT_ASSERT(_eventDistributor->configuredConsumerCount() == 1);
  CPPUNIT_ASSERT(_eventDistributor->initializedConsumerCount() == 1);
  CPPUNIT_ASSERT(_initMsgCollection->size() == 1);

  // *** second consumer ***
  {
    selections.clear();
    selections.push_back("c");
    selections.push_back("d");
    consInfo.reset(new EventConsumerRegistrationInfo(
        "http://cmswn1340.fnal.gov:52985/urn:xdaq-application:lid=29",
        5, 5, "Test Consumer", 5, 10, selections, "out4DQM", 120,
        stor::enquing_policy::DiscardOld));
    QueueID queueId(enquing_policy::DiscardNew, 2);
    consInfo->setQueueId(queueId);
    _eventDistributor->registerEventConsumer(&(*consInfo));
    
    CPPUNIT_ASSERT(_eventDistributor->configuredConsumerCount() == 2);
    CPPUNIT_ASSERT(_eventDistributor->initializedConsumerCount() == 2);
    CPPUNIT_ASSERT(_initMsgCollection->size() == 1);
  }

  // *** first event message (should pass both consumers) ***

  std::vector<unsigned char> hltBits;
  set_trigger_bit(hltBits, 8, edm::hlt::Ready);

  uint32 eventNumber = 1;
  uint32 hltBitCount = 9;
  clear_trigger_bits(hltBits);
  set_trigger_bit(hltBits, 0, edm::hlt::Pass);
  set_trigger_bit(hltBits, 2, edm::hlt::Pass);

  ref = allocate_frame_with_event_msg("out4DQM", hltBits, hltBitCount,
                                      eventNumber);
  stor::I2OChain eventMsgFrag(ref);
  CPPUNIT_ASSERT(eventMsgFrag.messageCode() == Header::EVENT);

  CPPUNIT_ASSERT(!eventMsgFrag.isTaggedForAnyStream());
  CPPUNIT_ASSERT(!eventMsgFrag.isTaggedForAnyEventConsumer());
  CPPUNIT_ASSERT(!eventMsgFrag.isTaggedForAnyDQMEventConsumer());

  _eventDistributor->addEventToRelevantQueues(eventMsgFrag);

  CPPUNIT_ASSERT(!eventMsgFrag.isTaggedForAnyStream());
  CPPUNIT_ASSERT(eventMsgFrag.isTaggedForAnyEventConsumer());
  CPPUNIT_ASSERT(!eventMsgFrag.isTaggedForAnyDQMEventConsumer());

  std::vector<QueueID> queueIdList = eventMsgFrag.getEventConsumerTags();
  CPPUNIT_ASSERT(queueIdList.size() == 2);
  CPPUNIT_ASSERT(queueIdList[0].index()  == 1);
  CPPUNIT_ASSERT(queueIdList[0].policy() == enquing_policy::DiscardOld);
  CPPUNIT_ASSERT(queueIdList[1].index() == 2);
  CPPUNIT_ASSERT(queueIdList[1].policy() == enquing_policy::DiscardNew);

  // *** third consumer ***
  {
    selections.clear();
    selections.push_back("c");
    selections.push_back("a");
    consInfo.reset(new EventConsumerRegistrationInfo(
        "http://cmswn1340.fnal.gov:52985/urn:xdaq-application:lid=29",
        5, 5, "Test Consumer", 5, 10, selections, "out4DQM", 120,
        stor::enquing_policy::DiscardOld));
    QueueID queueId(enquing_policy::DiscardOld, 3);
    consInfo->setQueueId(queueId);
    consInfo->registerMe(&(*_eventDistributor));
    
    CPPUNIT_ASSERT(_eventDistributor->configuredConsumerCount() == 3);
    CPPUNIT_ASSERT(_eventDistributor->initializedConsumerCount() == 3);
    CPPUNIT_ASSERT(_initMsgCollection->size() == 1);
  }
  // *** fourth consumer ***
  {
    selections.clear();
    selections.push_back("b");
    selections.push_back("d");
    consInfo.reset(new EventConsumerRegistrationInfo(
        "http://cmswn1340.fnal.gov:52985/urn:xdaq-application:lid=29",
        5, 5, "Test Consumer", 5, 10, selections, "out4DQM", 120,
        stor::enquing_policy::DiscardOld));
    QueueID queueId(enquing_policy::DiscardNew, 4);
    consInfo->setQueueId(queueId);
    consInfo->registerMe(&(*_eventDistributor));
    
    CPPUNIT_ASSERT(_eventDistributor->configuredConsumerCount() == 4);
    CPPUNIT_ASSERT(_eventDistributor->initializedConsumerCount() == 4);
    CPPUNIT_ASSERT(_initMsgCollection->size() == 1);
  }
  
  // *** second event message (should not pass) ***
  
  clear_trigger_bits(hltBits);
  set_trigger_bit(hltBits, 0, edm::hlt::Fail);
  set_trigger_bit(hltBits, 2, edm::hlt::Exception);
  set_trigger_bit(hltBits, 4, edm::hlt::Pass);

  ++eventNumber;
  ref = allocate_frame_with_event_msg("out4DQM", hltBits, hltBitCount,
                                      eventNumber);
  stor::I2OChain eventMsgFrag2(ref);
  CPPUNIT_ASSERT(eventMsgFrag2.messageCode() == Header::EVENT);

  CPPUNIT_ASSERT(!eventMsgFrag2.isTaggedForAnyStream());
  CPPUNIT_ASSERT(!eventMsgFrag2.isTaggedForAnyEventConsumer());
  CPPUNIT_ASSERT(!eventMsgFrag2.isTaggedForAnyDQMEventConsumer());

  _eventDistributor->addEventToRelevantQueues(eventMsgFrag2);

  CPPUNIT_ASSERT(!eventMsgFrag2.isTaggedForAnyStream());
  CPPUNIT_ASSERT(!eventMsgFrag2.isTaggedForAnyEventConsumer());
  CPPUNIT_ASSERT(!eventMsgFrag2.isTaggedForAnyDQMEventConsumer());

  // *** third event message (should pass) ***

  clear_trigger_bits(hltBits);
  set_trigger_bit(hltBits, 0, edm::hlt::Pass);
  set_trigger_bit(hltBits, 3, edm::hlt::Pass);
  set_trigger_bit(hltBits, 4, edm::hlt::Pass);
  set_trigger_bit(hltBits, 5, edm::hlt::Pass);
  set_trigger_bit(hltBits, 6, edm::hlt::Pass);
  set_trigger_bit(hltBits, 7, edm::hlt::Pass);

  ++eventNumber;
  ref = allocate_frame_with_event_msg("out4DQM", hltBits, hltBitCount,
                                      eventNumber);
  stor::I2OChain eventMsgFrag3(ref);
  CPPUNIT_ASSERT(eventMsgFrag3.messageCode() == Header::EVENT);

  CPPUNIT_ASSERT(!eventMsgFrag3.isTaggedForAnyStream());
  CPPUNIT_ASSERT(!eventMsgFrag3.isTaggedForAnyEventConsumer());
  CPPUNIT_ASSERT(!eventMsgFrag3.isTaggedForAnyDQMEventConsumer());

  _eventDistributor->addEventToRelevantQueues(eventMsgFrag3);

  CPPUNIT_ASSERT(!eventMsgFrag3.isTaggedForAnyStream());
  CPPUNIT_ASSERT(eventMsgFrag3.isTaggedForAnyEventConsumer());
  CPPUNIT_ASSERT(!eventMsgFrag3.isTaggedForAnyDQMEventConsumer());

  queueIdList = eventMsgFrag3.getEventConsumerTags();
  CPPUNIT_ASSERT(queueIdList.size() == 4);
  CPPUNIT_ASSERT(queueIdList[0].index() == 1);
  CPPUNIT_ASSERT(queueIdList[0].policy() == enquing_policy::DiscardOld);
  CPPUNIT_ASSERT(queueIdList[1].index() == 2);
  CPPUNIT_ASSERT(queueIdList[1].policy() == enquing_policy::DiscardNew);
  CPPUNIT_ASSERT(queueIdList[2].index() == 3);
  CPPUNIT_ASSERT(queueIdList[2].policy() == enquing_policy::DiscardOld);
  CPPUNIT_ASSERT(queueIdList[3].index() == 4);
  CPPUNIT_ASSERT(queueIdList[3].policy() == enquing_policy::DiscardNew);

  // *** fourth event message (should not pass) ***

  clear_trigger_bits(hltBits);
  set_trigger_bit(hltBits, 0, edm::hlt::Pass);
  set_trigger_bit(hltBits, 1, edm::hlt::Pass);
  set_trigger_bit(hltBits, 2, edm::hlt::Pass);
  set_trigger_bit(hltBits, 3, edm::hlt::Pass);

  ++eventNumber;
  ref = allocate_frame_with_event_msg("BOGUS", hltBits, hltBitCount,
                                      eventNumber);
  stor::I2OChain eventMsgFrag4(ref);
  CPPUNIT_ASSERT(eventMsgFrag4.messageCode() == Header::EVENT);

  CPPUNIT_ASSERT(!eventMsgFrag4.isTaggedForAnyStream());
  CPPUNIT_ASSERT(!eventMsgFrag4.isTaggedForAnyEventConsumer());
  CPPUNIT_ASSERT(!eventMsgFrag4.isTaggedForAnyDQMEventConsumer());

  _eventDistributor->addEventToRelevantQueues(eventMsgFrag4);

  CPPUNIT_ASSERT(!eventMsgFrag4.isTaggedForAnyStream());
  CPPUNIT_ASSERT(!eventMsgFrag4.isTaggedForAnyEventConsumer());
  CPPUNIT_ASSERT(!eventMsgFrag4.isTaggedForAnyDQMEventConsumer());

  // *** first error message (should not pass) ***

  ++eventNumber;
  ref = allocate_frame_with_error_msg(eventNumber);
  stor::I2OChain errorMsgFrag(ref);
  CPPUNIT_ASSERT(errorMsgFrag.messageCode() == Header::ERROR_EVENT);

  CPPUNIT_ASSERT(!errorMsgFrag.isTaggedForAnyStream());
  CPPUNIT_ASSERT(!errorMsgFrag.isTaggedForAnyEventConsumer());
  CPPUNIT_ASSERT(!errorMsgFrag.isTaggedForAnyDQMEventConsumer());

  _eventDistributor->addEventToRelevantQueues(errorMsgFrag);

  CPPUNIT_ASSERT(!errorMsgFrag.isTaggedForAnyStream());
  CPPUNIT_ASSERT(!errorMsgFrag.isTaggedForAnyEventConsumer());
  CPPUNIT_ASSERT(!errorMsgFrag.isTaggedForAnyDQMEventConsumer());

  // *** cleanup ***

  CPPUNIT_ASSERT(_eventDistributor->configuredConsumerCount() == 4);
  CPPUNIT_ASSERT(_eventDistributor->initializedConsumerCount() == 4);
  CPPUNIT_ASSERT(_initMsgCollection->size() == 1);
  CPPUNIT_ASSERT(_eventDistributor->configuredStreamCount() == 0);
  CPPUNIT_ASSERT(_eventDistributor->initializedStreamCount() == 0);

  _initMsgCollection->clear();
  _eventDistributor->clearStreams();
  _eventDistributor->clearConsumers();

  CPPUNIT_ASSERT(_eventDistributor->configuredConsumerCount() == 0);
  CPPUNIT_ASSERT(_eventDistributor->initializedConsumerCount() == 0);
  CPPUNIT_ASSERT(_initMsgCollection->size() == 0);
  CPPUNIT_ASSERT(_eventDistributor->configuredStreamCount() == 0);
  CPPUNIT_ASSERT(_eventDistributor->initializedStreamCount() == 0);
}


void testEventDistributor::
parseStreamConfigs(std::string cfgString,
                   EventDistributor::EvtStrConfList& evtCfgList,
                   EventDistributor::ErrStrConfList& errCfgList)
{
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
        if (mod_type == "EventStreamFileWriter") {

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
          evtCfgList.push_back(cfgInfo);
        }
        else if (mod_type == "ErrorStreamFileWriter" ||
                 mod_type == "FRDStreamFileWriter") {

          std::string streamLabel =
            endPathPSet.getParameter<std::string> ("streamLabel");
          long long maxFileSize =
            1048576 * (long long) endPathPSet.getParameter<int> ("maxSize");

          ErrorStreamConfigurationInfo cfgInfo(streamLabel,
                                               maxFileSize);
          cfgInfo.setStreamId(++streamId);
          errCfgList.push_back(cfgInfo);
        }
      }
    }
  }
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


void testEventDistributor::testDQMMessages()
{

  //
  //// Copied and pasted from methods for other message types: ////
  //

  if (_eventDistributor.get() == 0)
    {
      _initMsgCollection.reset(new InitMsgCollection());
      _eventDistributor.reset(new EventDistributor(_initMsgCollection));
    }

  EventDistributor::EvtStrConfList evtCfgList;
  EventDistributor::ErrStrConfList errCfgList;
  parseStreamConfigs(getSampleStreamConfig(), evtCfgList, errCfgList);
  _eventDistributor->registerEventStreams(evtCfgList);
  _eventDistributor->registerErrorStreams(errCfgList);

  //
  //// DQM-specific stuff: ////
  //

  std::string url = "http://localhost:43210/urn:xdaq-application:lid=77";
  enquing_policy::PolicyTag policy = stor::enquing_policy::DiscardOld;

  // Consumer 1:
  boost::shared_ptr<DQMEventConsumerRegistrationInfo> ri1;
  ri1.reset( new DQMEventConsumerRegistrationInfo( url,
                                                   "DQM Consumer 1",
                                                   10, 10, "HCAL",
                                                   policy, 10 ) );
  QueueID qid1( enquing_policy::DiscardOld, 1 );
  ri1->setQueueId( qid1 );
  _eventDistributor->registerDQMEventConsumer( &( *ri1 ) );

  // Consumer 2:
  boost::shared_ptr<DQMEventConsumerRegistrationInfo> ri2;
  ri2.reset( new DQMEventConsumerRegistrationInfo( url,
                                                   "DQM Consumer 2",
                                                   10, 10, "ECAL",
                                                   policy, 10 ) );
  QueueID qid2( enquing_policy::DiscardOld, 2 );
  ri2->setQueueId( qid2 );
  _eventDistributor->registerDQMEventConsumer( &( *ri2 ) );

  // HCAL event:
  Reference* ref1 = allocate_frame_with_dqm_msg( 1111, "HCAL" );
  stor::I2OChain frag1( ref1 );
  CPPUNIT_ASSERT( frag1.messageCode() == Header::DQM_EVENT );
  _eventDistributor->addEventToRelevantQueues( frag1 );
  CPPUNIT_ASSERT( frag1.isTaggedForAnyDQMEventConsumer() );
  CPPUNIT_ASSERT( frag1.getDQMEventConsumerTags().size() == 1 );

  // ECAL event:
  Reference* ref2 = allocate_frame_with_dqm_msg( 2222, "ECAL" );
  stor::I2OChain frag2( ref2 );
  CPPUNIT_ASSERT( frag2.messageCode() == Header::DQM_EVENT );
  _eventDistributor->addEventToRelevantQueues( frag2 );
  CPPUNIT_ASSERT( frag2.isTaggedForAnyDQMEventConsumer() );
  
  CPPUNIT_ASSERT( frag2.getDQMEventConsumerTags().size() == 1 );

  // Unknown event:
  Reference* ref3 = allocate_frame_with_dqm_msg( 3333, "idontbelonghere" );
  stor::I2OChain frag3( ref3 );
  CPPUNIT_ASSERT( frag3.messageCode() == Header::DQM_EVENT );
  _eventDistributor->addEventToRelevantQueues( frag3 );

  CPPUNIT_ASSERT( !frag3.isTaggedForAnyDQMEventConsumer() );
  CPPUNIT_ASSERT( frag3.getDQMEventConsumerTags().size() == 0 );

}

// This macro writes the 'main' for this test.
CPPUNIT_TEST_SUITE_REGISTRATION(testEventDistributor);


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
