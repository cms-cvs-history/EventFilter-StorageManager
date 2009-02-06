#include "Utilities/Testing/interface/CppUnit_testdriver.icpp"
#include "cppunit/extensions/HelperMacros.h"

#include <math.h>
#include <stdlib.h>

#include "EventFilter/StorageManager/interface/MonitoredQuantity.h"

/////////////////////////////////////////////////////////////
//
// This test exercises the MonitoredQuantity class
//
/////////////////////////////////////////////////////////////

using namespace stor;


class testMonitoredQuantity : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(testMonitoredQuantity);
  CPPUNIT_TEST(testEmpty);
  CPPUNIT_TEST(testFull);
  CPPUNIT_TEST(testRecent);
  CPPUNIT_TEST(testDisable);

  CPPUNIT_TEST_SUITE_END();

public:
  testMonitoredQuantity();
  void testEmpty();
  void testFull();
  void testRecent();
  void testDisable();

private:

  void accumulateSamples
  (
    unsigned int sampleCount,
    double &squareSum
  );

  void testResults
  (
    MonitoredQuantity::DataSetType type,
    unsigned int cycleCount,
    unsigned int sampleCount,
    double squareSum
  );


  MonitoredQuantity _quantity;

  const double _multiplier;
};

testMonitoredQuantity::testMonitoredQuantity() :
  _multiplier(drand48()*100)
{
  //Only 2 bins deep history for testing
  _quantity.setNewTimeWindowForRecentResults(2);

  srand48(static_cast<long int>(_quantity.getCurrentTime()));
}

void testMonitoredQuantity::accumulateSamples
(
  unsigned int sampleCount,
  double &squareSum
)
{
  assert(!isnan(squareSum));
  for (
    unsigned int i = 1;
    i <= sampleCount;
    ++i
  )
  {
    _quantity.addSample(i*_multiplier);
    squareSum += pow(i*_multiplier,2);
  }
  _quantity.calculateStatistics();
}


void testMonitoredQuantity::testResults
(
  MonitoredQuantity::DataSetType type,
  unsigned int cycleCount,
  unsigned int sampleCount,
  double squareSum
)
{
  // we don't expect an exact agreement due to rounding precision
  const double smallValue = 1e-5;

  CPPUNIT_ASSERT(_quantity.getSampleCount(type) == cycleCount * sampleCount);
  
  CPPUNIT_ASSERT(
    fabs(
      _quantity.getValueSum(type) -
      cycleCount * static_cast<double>(sampleCount)*(sampleCount+1)/2 * _multiplier
    ) < smallValue);

  CPPUNIT_ASSERT(
    fabs(
      _quantity.getValueAverage(type) -
      ((cycleCount) ? static_cast<double>(sampleCount+1)/2 * _multiplier : 0)
    ) < smallValue);

  CPPUNIT_ASSERT(_quantity.getValueMin(type) == 
    (cycleCount) ? _multiplier : 1e+9);

  CPPUNIT_ASSERT(_quantity.getValueMax(type) == 
    (cycleCount) ? static_cast<double>(sampleCount)*_multiplier : 1e-9);

  if (_quantity.getDuration(type) > 0)
  {
    CPPUNIT_ASSERT(
      fabs(
        _quantity.getSampleRate(type) -
        cycleCount*sampleCount/_quantity.getDuration(type)
      ) < smallValue);    

    CPPUNIT_ASSERT(
      fabs(
        _quantity.getSampleLatency(type) -
        1e6*_quantity.getDuration(type)/(cycleCount*sampleCount)
      ) < smallValue);    

    CPPUNIT_ASSERT(
      fabs(
        _quantity.getValueRate(type) -
        _quantity.getValueSum(type)/_quantity.getDuration(type)
      ) < smallValue);
  }

  if (sampleCount > 0)
  {
    unsigned long numEntries = cycleCount * sampleCount;
    double rmsSquared = pow(_quantity.getValueRMS(type), 2);
    double expectedSquare = squareSum/numEntries 
      - pow(_quantity.getValueAverage(type),2);

    double difference = rmsSquared - expectedSquare;
    std::cerr << "\n type " << type;
    std::cerr << "\n square sum " << squareSum;
    std::cerr << "\n cycle count " << cycleCount;
    std::cerr << "\n sample count " << sampleCount;
    std::cerr << "\n RMS:        " << _quantity.getValueRMS(type);
    std::cerr << "\n expected square: " << expectedSquare;
    std::cerr << "\n difference: " << difference;

    std::cerr << '\n';

    CPPUNIT_ASSERT(fabs(difference) < smallValue);
//     CPPUNIT_ASSERT(
//       fabs(
//            _quantity.getValueRMS(type) -
//            sqrt((squareSum/(cycleCount*sampleCount))-pow(_quantity.getValueAverage(type),2))
//            ) < smallValue);
  }
}


void testMonitoredQuantity::testEmpty()
{
  _quantity.reset();

  _quantity.calculateStatistics();

  testResults(MonitoredQuantity::FULL, 0, 0, 0);
  testResults(MonitoredQuantity::RECENT, 0, 0, 0);
}


void testMonitoredQuantity::testFull()
{
  int sampleCount = 100;
  double squareSum = 0.0;

  _quantity.reset();

  accumulateSamples(sampleCount, squareSum);

  testResults(MonitoredQuantity::FULL, 1, sampleCount, squareSum);
}


void testMonitoredQuantity::testRecent()
{
  int sampleCount = 50;
  double squareSum=0.0, totalSquareSum=0.0;

  _quantity.reset();

  accumulateSamples(sampleCount, squareSum);
  // reset square sum as buffer is only 2 deep
  totalSquareSum = squareSum;
  squareSum = 0;
  accumulateSamples(sampleCount, squareSum);
  accumulateSamples(sampleCount, squareSum);
  totalSquareSum += squareSum;

  testResults(MonitoredQuantity::FULL, 3, sampleCount, totalSquareSum);
  testResults(MonitoredQuantity::RECENT, 2, sampleCount, squareSum);
}


void testMonitoredQuantity::testDisable()
{
  int sampleCount = 50;
  double squareSum(0.0), dummySquareSum(0.0);

  _quantity.reset();

  accumulateSamples(sampleCount, squareSum);
  // disable the quantity, no changes expected
  _quantity.disable();
  accumulateSamples(sampleCount, dummySquareSum);

  testResults(MonitoredQuantity::FULL, 1, sampleCount, squareSum);
  testResults(MonitoredQuantity::RECENT, 1, sampleCount, squareSum);

  // Reenable quantity. This resets everything.
  _quantity.enable();
  squareSum = 0;
  accumulateSamples(sampleCount, squareSum);

  testResults(MonitoredQuantity::FULL, 1, sampleCount, squareSum);
  testResults(MonitoredQuantity::RECENT, 1, sampleCount, squareSum);
}




// This macro writes the 'main' for this test.
CPPUNIT_TEST_SUITE_REGISTRATION(testMonitoredQuantity);


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
