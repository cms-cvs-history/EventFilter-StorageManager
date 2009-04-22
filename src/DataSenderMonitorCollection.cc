// $Id: DataSenderMonitorCollection.cc,v 1.1.2.6 2009/04/09 17:00:35 mommsen Exp $

#include <string>
#include <sstream>
#include <iomanip>

#include "EventFilter/StorageManager/interface/Exception.h"
#include "EventFilter/StorageManager/interface/DataSenderMonitorCollection.h"

using namespace stor;


DataSenderMonitorCollection::DataSenderMonitorCollection(xdaq::Application *app) :
MonitorCollection(app)
{
}


void DataSenderMonitorCollection::addFragmentSample(I2OChain const& i2oChain)
{
  // focus only on INIT and event fragments, for now
  if (i2oChain.messageCode() != Header::INIT &&
      i2oChain.messageCode() != Header::EVENT) {return;}
  if (i2oChain.fragmentCount() != 1) {return;}

  // fetch basic data from the I2OChain
  double fragmentSize = static_cast<double>(i2oChain.totalDataSize());

  // look up the monitoring records that we need
  RBRecordPtr rbRecordPtr;
  FURecordPtr fuRecordPtr;
  OutModRecordPtr topLevelOutModPtr, rbSpecificOutModPtr, fuSpecificOutModPtr;
  {
    boost::mutex::scoped_lock sl(_collectionsMutex);
    getAllNeededPointers(i2oChain, rbRecordPtr, fuRecordPtr,
                         topLevelOutModPtr, rbSpecificOutModPtr,
                         fuSpecificOutModPtr);
  }

  // accumulate the data of interest
  topLevelOutModPtr->fragmentSize.addSample(fragmentSize);
  rbSpecificOutModPtr->fragmentSize.addSample(fragmentSize);
  fuSpecificOutModPtr->fragmentSize.addSample(fragmentSize);
}


void DataSenderMonitorCollection::addInitSample(I2OChain const& i2oChain)
{
  // sanity checks
  if (i2oChain.messageCode() != Header::INIT) {return;}
  if (! i2oChain.complete()) {return;}

  // fetch basic data from the I2OChain
  std::string outModName = i2oChain.outputModuleLabel();
  unsigned int msgSize = i2oChain.totalDataSize();

  // look up the monitoring records that we need
  RBRecordPtr rbRecordPtr;
  FURecordPtr fuRecordPtr;
  OutModRecordPtr topLevelOutModPtr, rbSpecificOutModPtr, fuSpecificOutModPtr;
  {
    boost::mutex::scoped_lock sl(_collectionsMutex);
    getAllNeededPointers(i2oChain, rbRecordPtr, fuRecordPtr,
                         topLevelOutModPtr, rbSpecificOutModPtr,
                         fuSpecificOutModPtr);
  }

  // accumulate the data of interest
  topLevelOutModPtr->name = outModName;
  topLevelOutModPtr->initMsgSize = msgSize;

  ++rbRecordPtr->initMsgCount;
  rbSpecificOutModPtr->name = outModName;
  rbSpecificOutModPtr->initMsgSize = msgSize;

  ++fuRecordPtr->initMsgCount;
  fuSpecificOutModPtr->name = outModName;
  fuSpecificOutModPtr->initMsgSize = msgSize;
}


void DataSenderMonitorCollection::addEventSample(I2OChain const& i2oChain)
{
  // sanity checks
  if (i2oChain.messageCode() != Header::EVENT) {return;}
  if (! i2oChain.complete()) {return;}

  // fetch basic data from the I2OChain
  double eventSize = static_cast<double>(i2oChain.totalDataSize());
  unsigned int runNumber = i2oChain.runNumber();
  unsigned int eventNumber = i2oChain.eventNumber();

  // look up the monitoring records that we need
  RBRecordPtr rbRecordPtr;
  FURecordPtr fuRecordPtr;
  OutModRecordPtr topLevelOutModPtr, rbSpecificOutModPtr, fuSpecificOutModPtr;
  {
    boost::mutex::scoped_lock sl(_collectionsMutex);
    getAllNeededPointers(i2oChain, rbRecordPtr, fuRecordPtr,
                         topLevelOutModPtr, rbSpecificOutModPtr,
                         fuSpecificOutModPtr);
  }

  // accumulate the data of interest
  topLevelOutModPtr->runNumber = runNumber;
  topLevelOutModPtr->eventSize.addSample(eventSize);

  rbRecordPtr->lastEventNumber = eventNumber;
  rbRecordPtr->eventSize.addSample(eventSize);
  rbSpecificOutModPtr->runNumber = i2oChain.runNumber();
  rbSpecificOutModPtr->eventSize.addSample(eventSize);

  fuRecordPtr->lastEventNumber = eventNumber;
  fuRecordPtr->eventSize.addSample(eventSize);
  fuSpecificOutModPtr->runNumber = i2oChain.runNumber();
  fuSpecificOutModPtr->eventSize.addSample(eventSize);
}


DataSenderMonitorCollection::OutputModuleResultsList
DataSenderMonitorCollection::getTopLevelOutputModuleResults() const
{
  boost::mutex::scoped_lock sl(_collectionsMutex);
  OutputModuleResultsList resultsList;

  OutputModuleRecordMap::const_iterator omMapIter;
  OutputModuleRecordMap::const_iterator omMapEnd = _outputModuleMap.end();
  for (omMapIter = _outputModuleMap.begin(); omMapIter != omMapEnd; ++omMapIter)
    {
      OutModRecordPtr outModRecordPtr = omMapIter->second;
      OutputModuleResult result;
      result.name = outModRecordPtr->name;
      result.id = outModRecordPtr->id;
      outModRecordPtr->eventSize.getStats(result.eventStats);
      resultsList.push_back(result);
    }

  return resultsList;
}


void DataSenderMonitorCollection::do_calculateStatistics()
{
  boost::mutex::scoped_lock sl(_collectionsMutex);

  std::map<ResourceBrokerKey, RBRecordPtr>::const_iterator rbMapIter;
  std::map<ResourceBrokerKey, RBRecordPtr>::const_iterator rbMapEnd =
    _resourceBrokerMap.end();
  for (rbMapIter=_resourceBrokerMap.begin(); rbMapIter!=rbMapEnd; ++rbMapIter)
    {
      RBRecordPtr rbRecordPtr = rbMapIter->second;
      calcStatsForOutputModules(rbRecordPtr->outputModuleMap);

      std::map<FilterUnitKey, FURecordPtr>::const_iterator fuMapIter;
      std::map<FilterUnitKey, FURecordPtr>::const_iterator fuMapEnd =
        rbRecordPtr->filterUnitMap.end();        
      for (fuMapIter = rbRecordPtr->filterUnitMap.begin();
           fuMapIter != fuMapEnd; ++fuMapIter)
        {
          FURecordPtr fuRecordPtr = fuMapIter->second;
          calcStatsForOutputModules(fuRecordPtr->outputModuleMap);
        }
    }

  calcStatsForOutputModules(_outputModuleMap);
}


void DataSenderMonitorCollection::do_updateInfoSpace()
{
}


void DataSenderMonitorCollection::do_reset()
{
  boost::mutex::scoped_lock sl(_collectionsMutex);

  _resourceBrokerMap.clear();
  _outputModuleMap.clear();
}


typedef DataSenderMonitorCollection DSMC;

bool DSMC::getAllNeededPointers(I2OChain const& i2oChain,
                                DSMC::RBRecordPtr& rbRecordPtr,
                                DSMC::FURecordPtr& fuRecordPtr,
                                DSMC::OutModRecordPtr& topLevelOutModPtr,
                                DSMC::OutModRecordPtr& rbSpecificOutModPtr,
                                DSMC::OutModRecordPtr& fuSpecificOutModPtr)
{
  ResourceBrokerKey rbKey(i2oChain);
  if (! rbKey.isValid) {return false;}
  FilterUnitKey fuKey(i2oChain);
  if (! fuKey.isValid) {return false;}
  OutputModuleKey outModKey = i2oChain.outputModuleId();

  topLevelOutModPtr = getOutputModuleRecord(_outputModuleMap, outModKey);

  rbRecordPtr = getResourceBrokerRecord(rbKey);
  rbSpecificOutModPtr = getOutputModuleRecord(rbRecordPtr->outputModuleMap,
                                              outModKey);

  fuRecordPtr = getFilterUnitRecord(rbRecordPtr, fuKey);
  fuSpecificOutModPtr = getOutputModuleRecord(fuRecordPtr->outputModuleMap,
                                              outModKey);

  return true;
}

DSMC::RBRecordPtr
DSMC::getResourceBrokerRecord(DSMC::ResourceBrokerKey const& rbKey)
{
  RBRecordPtr rbRecordPtr;
  std::map<ResourceBrokerKey, RBRecordPtr>::const_iterator rbMapIter;
  rbMapIter = _resourceBrokerMap.find(rbKey);
  if (rbMapIter == _resourceBrokerMap.end())
    {
      rbRecordPtr.reset(new ResourceBrokerRecord(rbKey));
      _resourceBrokerMap[rbKey] = rbRecordPtr;
    }
  else
    {
      rbRecordPtr = rbMapIter->second;
    }
  return rbRecordPtr;
}


DSMC::FURecordPtr
DSMC::getFilterUnitRecord(DSMC::RBRecordPtr& rbRecordPtr,
                          DSMC::FilterUnitKey const& fuKey)
{
  FURecordPtr fuRecordPtr;
  std::map<FilterUnitKey, FURecordPtr>::const_iterator fuMapIter;
  fuMapIter = rbRecordPtr->filterUnitMap.find(fuKey);
  if (fuMapIter == rbRecordPtr->filterUnitMap.end())
    {
      fuRecordPtr.reset(new FilterUnitRecord(fuKey));
      rbRecordPtr->filterUnitMap[fuKey] = fuRecordPtr;
    }
  else
    {
      fuRecordPtr = fuMapIter->second;
    }
  return fuRecordPtr;
}


DSMC::OutModRecordPtr
DSMC::getOutputModuleRecord(OutputModuleRecordMap& outModMap,
                            DSMC::OutputModuleKey const& outModKey)
{
  OutModRecordPtr outModRecordPtr;
  OutputModuleRecordMap::const_iterator omMapIter;
  omMapIter = outModMap.find(outModKey);
  if (omMapIter == outModMap.end())
    {
      outModRecordPtr.reset(new OutputModuleRecord());

      outModRecordPtr->name = "Unknown";
      outModRecordPtr->id = outModKey;
      outModRecordPtr->initMsgSize = 0;
      outModRecordPtr->runNumber = 0;

      outModMap[outModKey] = outModRecordPtr;
    }
  else
    {
      outModRecordPtr = omMapIter->second;
    }
  return outModRecordPtr;
}


void DSMC::calcStatsForOutputModules(DSMC::OutputModuleRecordMap& outputModuleMap)
{
  OutputModuleRecordMap::const_iterator omMapIter;
  OutputModuleRecordMap::const_iterator omMapEnd = outputModuleMap.end();
  for (omMapIter = outputModuleMap.begin(); omMapIter != omMapEnd; ++omMapIter)
    {
      OutModRecordPtr outModRecordPtr = omMapIter->second;

      outModRecordPtr->fragmentSize.calculateStatistics();
      outModRecordPtr->eventSize.calculateStatistics();
    }
}


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -