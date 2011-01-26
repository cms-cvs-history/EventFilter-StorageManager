#include <iostream>
#include <iomanip>

#include "Utilities/Testing/interface/CppUnit_testdriver.icpp"
#include "cppunit/extensions/HelperMacros.h"

#include "EventFilter/StorageManager/interface/Configuration.h"
#include "EventFilter/StorageManager/interface/DQMEventConsumerRegistrationInfo.h"
#include "EventFilter/StorageManager/interface/EventConsumerRegistrationInfo.h"

using namespace stor;

class testConsumerRegistrationInfo : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(testConsumerRegistrationInfo);
  CPPUNIT_TEST(testEventConsumerRegistrationInfo);
  CPPUNIT_TEST(testEventConsumerPSet);
  CPPUNIT_TEST(testIncompleteEventConsumerPSet);
  CPPUNIT_TEST(testIdenticalEventConsumers);
  CPPUNIT_TEST(testDQMEventConsumerRegistrationInfo);
  CPPUNIT_TEST(testIdenticalDQMEventConsumers);

  CPPUNIT_TEST_SUITE_END();

public:

  void testEventConsumerRegistrationInfo();
  void testEventConsumerPSet();
  void testIncompleteEventConsumerPSet();
  void testIdenticalEventConsumers();

  void testDQMEventConsumerRegistrationInfo();
  void testIdenticalDQMEventConsumers();

};


void testConsumerRegistrationInfo::testEventConsumerRegistrationInfo()
{
  Strings eventSelection;
  eventSelection.push_back( "DQM1" );
  eventSelection.push_back( "DQM2" );

  const std::string triggerSelection = "DQM1 || DQM2";

  QueueID qid(enquing_policy::DiscardOld, 3);

  EventConsumerRegistrationInfo ecri(
    "Test Consumer",
    "localhost",
    triggerSelection,
    eventSelection,
    "hltOutputDQM",
    1,
    false,
    qid.index(),
    qid.policy(),
    boost::posix_time::seconds(10),
    boost::posix_time::milliseconds(100)
  );
  ecri.setQueueId( qid );

  CPPUNIT_ASSERT( ecri.consumerName() == "Test Consumer" );
  CPPUNIT_ASSERT( ecri.consumerId() == ConsumerID(0) );
  CPPUNIT_ASSERT( ecri.queueId() == qid );
  CPPUNIT_ASSERT( ecri.queueSize() == 3 );
  CPPUNIT_ASSERT( ecri.queuePolicy() == enquing_policy::DiscardOld );
  CPPUNIT_ASSERT( ecri.secondsToStale() == boost::posix_time::seconds(10) );
  CPPUNIT_ASSERT( ecri.triggerSelection() == triggerSelection );
  CPPUNIT_ASSERT( ecri.eventSelection() == eventSelection );
  CPPUNIT_ASSERT( ecri.outputModuleLabel() == "hltOutputDQM" );
  CPPUNIT_ASSERT( ecri.uniqueEvents() == false );
  CPPUNIT_ASSERT( ecri.remoteHost() == "localhost" );
  CPPUNIT_ASSERT( ecri.minEventRequestInterval() == boost::posix_time::milliseconds(100) );
  CPPUNIT_ASSERT( fabs(stor::utils::duration_to_seconds(ecri.minEventRequestInterval()) - 0.1) < 0.0001);

  edm::ParameterSet pset = ecri.getPSet();
  CPPUNIT_ASSERT( pset.getUntrackedParameter<std::string>("SelectHLTOutput") == "hltOutputDQM" );
  CPPUNIT_ASSERT( pset.getUntrackedParameter<std::string>("TriggerSelector") == triggerSelection );
  CPPUNIT_ASSERT( pset.getParameter<Strings>("TrackedEventSelection") == eventSelection );
  CPPUNIT_ASSERT( pset.getUntrackedParameter<bool>("uniqueEvents") == false );
  CPPUNIT_ASSERT( pset.getUntrackedParameter<unsigned int>("prescale") == 1 );
  CPPUNIT_ASSERT( pset.getUntrackedParameter<int>("queueSize") == 3 );
  CPPUNIT_ASSERT( pset.getUntrackedParameter<double>("consumerTimeOut") == 10 );
  CPPUNIT_ASSERT( pset.getUntrackedParameter<std::string>("queuePolicy") == "DiscardOld" );
  CPPUNIT_ASSERT( pset.getUntrackedParameter<double>("maxEventRequestRate") == 10 );
}


void testConsumerRegistrationInfo::testEventConsumerPSet()
{
  Strings eventSelection;
  eventSelection.push_back( "DQM" );
  eventSelection.push_back( "HLT" );

  const std::string triggerSelection = "HLT || DQM";

  edm::ParameterSet origPSet;
  origPSet.addUntrackedParameter<std::string>("SelectHLTOutput", "hltOutputDQM");
  origPSet.addUntrackedParameter<std::string>("TriggerSelector", triggerSelection);
  origPSet.addParameter<Strings>("TrackedEventSelection", eventSelection);
  origPSet.addUntrackedParameter<bool>("uniqueEvents", true);
  origPSet.addUntrackedParameter<unsigned int>("prescale", 5);
  origPSet.addUntrackedParameter<int>("queueSize", 10);
  origPSet.addUntrackedParameter<double>("consumerTimeOut", 33);
  origPSet.addUntrackedParameter<std::string>("queuePolicy", "DiscardNew");
  origPSet.addUntrackedParameter<double>("maxEventRequestRate", 25);

  EventConsumerRegistrationInfo ecri(
    "Test Consumer",
    "localhost",
    origPSet
  );

  edm::ParameterSet ecriPSet = ecri.getPSet();
  CPPUNIT_ASSERT( isTransientEqual(origPSet, ecriPSet) );
}


void testConsumerRegistrationInfo::testIncompleteEventConsumerPSet()
{
  edm::ParameterSet origPSet;
  origPSet.addUntrackedParameter<std::string>("SelectHLTOutput", "hltOutputDQM");
  
  EventConsumerRegistrationInfo ecri(
    "Test Consumer",
    "localhost",
    origPSet
  );

  CPPUNIT_ASSERT( ecri.queueSize() == 0 );
  CPPUNIT_ASSERT( ecri.queuePolicy() == enquing_policy::Max );
  CPPUNIT_ASSERT( ecri.secondsToStale() == boost::posix_time::seconds(0) );
  CPPUNIT_ASSERT( ecri.minEventRequestInterval() == boost::posix_time::not_a_date_time );

  edm::ParameterSet ecriPSet = ecri.getPSet();
  CPPUNIT_ASSERT( ! isTransientEqual(origPSet, ecriPSet) );
  CPPUNIT_ASSERT( ecriPSet.getUntrackedParameter<std::string>("SelectHLTOutput") == "hltOutputDQM" );
  CPPUNIT_ASSERT( ecriPSet.getUntrackedParameter<std::string>("TriggerSelector") == "" );
  CPPUNIT_ASSERT( ecriPSet.getParameter<Strings>("TrackedEventSelection") == Strings() );
  CPPUNIT_ASSERT( ecriPSet.getUntrackedParameter<bool>("uniqueEvents") == false );
  CPPUNIT_ASSERT( ecriPSet.getUntrackedParameter<unsigned int>("prescale") == 1 );
  CPPUNIT_ASSERT( ! ecriPSet.exists("queueSize") );
  CPPUNIT_ASSERT( ! ecriPSet.exists("consumerTimeOut") );
  CPPUNIT_ASSERT( ! ecriPSet.exists("queuePolicy") );
  CPPUNIT_ASSERT( ! ecriPSet.exists("maxEventRequestRate") );

  EventServingParams defaults;
  defaults._activeConsumerTimeout =  boost::posix_time::seconds(12);
  defaults._consumerQueueSize = 22;
  defaults._consumerQueuePolicy = "DiscardOld";

  EventConsumerRegistrationInfo ecriDefaults(
    "Test Consumer",
    "localhost",
    origPSet,
    defaults
  );

  CPPUNIT_ASSERT( ecriDefaults.queueSize() == defaults._consumerQueueSize );
  CPPUNIT_ASSERT( ecriDefaults.queuePolicy() == enquing_policy::DiscardOld );
  CPPUNIT_ASSERT( ecriDefaults.secondsToStale() == defaults._activeConsumerTimeout );
  CPPUNIT_ASSERT( ecriDefaults.minEventRequestInterval() == boost::posix_time::not_a_date_time );

  edm::ParameterSet ecriDefaultsPSet = ecriDefaults.getPSet();
  CPPUNIT_ASSERT( ! isTransientEqual(origPSet, ecriDefaultsPSet) );
  CPPUNIT_ASSERT( ! isTransientEqual(ecriPSet, ecriDefaultsPSet) );
  CPPUNIT_ASSERT( ecriDefaultsPSet.getUntrackedParameter<std::string>("SelectHLTOutput") == "hltOutputDQM" );
  CPPUNIT_ASSERT( ecriDefaultsPSet.getUntrackedParameter<std::string>("TriggerSelector") == "" );
  CPPUNIT_ASSERT( ecriDefaultsPSet.getParameter<Strings>("TrackedEventSelection") == Strings() );
  CPPUNIT_ASSERT( ecriDefaultsPSet.getUntrackedParameter<bool>("uniqueEvents") == false );
  CPPUNIT_ASSERT( ecriDefaultsPSet.getUntrackedParameter<unsigned int>("prescale") == 1 );
  CPPUNIT_ASSERT( ecriDefaultsPSet.getUntrackedParameter<int>("queueSize") == 22 );
  CPPUNIT_ASSERT( ecriDefaultsPSet.getUntrackedParameter<double>("consumerTimeOut") == 12 );
  CPPUNIT_ASSERT( ecriDefaultsPSet.getUntrackedParameter<std::string>("queuePolicy") == "DiscardOld" );
  CPPUNIT_ASSERT( ! ecriDefaultsPSet.exists("maxEventRequestRate") );
}


void testConsumerRegistrationInfo::testIdenticalEventConsumers()
{
  Strings eventSelection;
  eventSelection.push_back( "DQM1" );
  eventSelection.push_back( "DQM2" );

  const std::string triggerSelection = "DQM1 || DQM2";

  QueueID qid(enquing_policy::DiscardOld, 3);

  EventConsumerRegistrationInfo ecri1(
    "Consumer A",
    "localhost",
    triggerSelection,
    eventSelection,
    "hltOutputDQM",
    1,
    false,
    qid.index(),
    qid.policy(),
    boost::posix_time::seconds(10)
  );
  ecri1.setQueueId( qid );

  EventConsumerRegistrationInfo ecri2(
    "Consumer B",
    "remotehost",
    triggerSelection,
    eventSelection,
    "hltOutputDQM",
    1,
    false,
    qid.index(),
    qid.policy(),
    boost::posix_time::seconds(10)
  );
  ecri2.setQueueId( qid );

  const std::string triggerSelection2 = "DQM1";

  EventConsumerRegistrationInfo ecri3(
    "Consumer C",
    "farawayhost",
    triggerSelection2,
    eventSelection,
    "hltOutputDQM",
    1,
    false,
    qid.index(),
    qid.policy(),
    boost::posix_time::seconds(10)
  );
  ecri3.setQueueId( qid );

  EventConsumerRegistrationInfo ecri4(
    "Consumer D",
    "inanothergalaxyhost",
    triggerSelection2,
    eventSelection,
    "hltOutputDQM",
    1,
    true, // unique events
    qid.index(),
    qid.policy(),
    boost::posix_time::seconds(10)
  );
  ecri4.setQueueId( qid );

  EventConsumerRegistrationInfo ecri5(
    "Consumer D",
    "inanotheruniversehost",
    triggerSelection2,
    eventSelection,
    "hltOutputDQM",
    10, //different prescale
    true, // unique events
    qid.index(),
    qid.policy(),
    boost::posix_time::seconds(10)
  );
  ecri5.setQueueId( qid );

  CPPUNIT_ASSERT( ecri1 == ecri2 );
  CPPUNIT_ASSERT( ecri1 != ecri3 );
  CPPUNIT_ASSERT( ecri2 != ecri3 );
  CPPUNIT_ASSERT( ecri3 != ecri4 );
  CPPUNIT_ASSERT( ecri4 != ecri5 );
}


void testConsumerRegistrationInfo::testDQMEventConsumerRegistrationInfo()
{
  QueueID qid(stor::enquing_policy::DiscardNew, 2);
  DQMEventConsumerRegistrationInfo ecri(
    "Histo Consumer",
    "localhost",
    "*",
    qid.index(),
    qid.policy(),
    boost::posix_time::seconds(1024)
  );
  ecri.setQueueId( qid );
  
  CPPUNIT_ASSERT( ecri.consumerName() == "Histo Consumer" );
  CPPUNIT_ASSERT( ecri.consumerId() == ConsumerID(0) );
  CPPUNIT_ASSERT( ecri.queueId() == qid );
  CPPUNIT_ASSERT( ecri.queueSize() == 2 );
  CPPUNIT_ASSERT( ecri.queuePolicy() == enquing_policy::DiscardNew );
  CPPUNIT_ASSERT( ecri.secondsToStale() == boost::posix_time::seconds(1024) );
  CPPUNIT_ASSERT( ecri.topLevelFolderName() == "*" );
  CPPUNIT_ASSERT( ecri.remoteHost() == "localhost" );
}


void testConsumerRegistrationInfo::testIdenticalDQMEventConsumers()
{
  QueueID qid(stor::enquing_policy::DiscardNew, 2);
  DQMEventConsumerRegistrationInfo ecri1(
    "Histo Consumer 1",
    "localhost",
    "*",
    qid.index(),
    qid.policy(),
    boost::posix_time::seconds(1024)
  );
  ecri1.setQueueId( qid );

  DQMEventConsumerRegistrationInfo ecri2(
    "Histo Consumer 2",
    "remotehost",
    "*",
    qid.index(),
    qid.policy(),
    boost::posix_time::seconds(1024)
  );
  ecri2.setQueueId( qid );

  DQMEventConsumerRegistrationInfo ecri3(
    "Histo Consumer 3",
    "farawayhost",
    "HCAL",
    qid.index(),
    qid.policy(),
    boost::posix_time::seconds(1024)
  );
  ecri3.setQueueId( qid );

  DQMEventConsumerRegistrationInfo ecri4(
    "Histo Consumer 3",
    "farawayhost",
    "HCAL",
    qid.index(),
    qid.policy(),
    boost::posix_time::seconds(10)
  );
  ecri4.setQueueId( qid );
 
  CPPUNIT_ASSERT( ecri1 == ecri2 );
  CPPUNIT_ASSERT( ecri1 != ecri3 );
  CPPUNIT_ASSERT( ecri2 != ecri3 );
  CPPUNIT_ASSERT( ecri3 != ecri4 );
}


// This macro writes the 'main' for this test.
CPPUNIT_TEST_SUITE_REGISTRATION(testConsumerRegistrationInfo);


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
