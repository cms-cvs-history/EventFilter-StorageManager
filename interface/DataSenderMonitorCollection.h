// $Id: DataSenderMonitorCollection.h,v 1.1.2.6 2009/04/09 17:00:59 mommsen Exp $

#ifndef StorageManager_DataSenderMonitorCollection_h
#define StorageManager_DataSenderMonitorCollection_h

#include <map>

#include <boost/thread/mutex.hpp>
#include <boost/shared_ptr.hpp>

#include "EventFilter/StorageManager/interface/I2OChain.h"
#include "EventFilter/StorageManager/interface/MonitorCollection.h"
#include "IOPool/Streamer/interface/MsgHeader.h"


namespace stor {

  /**
   * A collection of MonitoredQuantities to track received fragments
   * and events by their source (resource broker, filter unit, etc.)
   *
   * $Author: mommsen $
   * $Revision: 1.1.2.6 $
   * $Date: 2009/04/09 17:00:59 $
   */
  
  class DataSenderMonitorCollection : public MonitorCollection
  {
  public:

    /**
     * Key that is used to identify resource brokers.
     */
    struct ResourceBrokerKey
    {
      bool isValid;
      std::string hltURL;
      unsigned int hltTid;
      unsigned int hltInstance;
      unsigned int hltLocalId;
      std::string hltClassName;

      explicit ResourceBrokerKey(I2OChain const& i2oChain)
      {
        if (i2oChain.messageCode() != Header::INVALID)
          {
            isValid = true;
            hltURL = i2oChain.hltURL();
            hltTid = i2oChain.hltTid();
            hltInstance = i2oChain.hltInstance();
            hltLocalId = i2oChain.hltLocalId();
            hltClassName = i2oChain.hltClassName();
          }
        else
          {
            isValid = false;
          }
      }

      bool operator<(ResourceBrokerKey const& other) const
      {
        if (isValid != other.isValid) return isValid < other.isValid;
        if (hltURL != other.hltURL) return hltURL < other.hltURL;
        if (hltTid != other.hltTid) return hltTid < other.hltTid;
        if (hltInstance != other.hltInstance) return hltInstance < other.hltInstance;
        if (hltLocalId != other.hltLocalId) return hltLocalId < other.hltLocalId;
        return hltClassName < other.hltClassName;
      }
    };

    /**
     * Key that is used to identify filter units.
     */
    struct FilterUnitKey
    {
      bool isValid;
      unsigned int fuProcessId;
      unsigned int fuGuid;

      explicit FilterUnitKey(I2OChain const& i2oChain)
      {
        if (i2oChain.messageCode() != Header::INVALID)
          {
            isValid = true;
            fuProcessId = i2oChain.fuProcessId();
            fuGuid = i2oChain.fuGuid();
          }
        else
          {
            isValid = false;
          }
      }

      bool operator<(FilterUnitKey const& other) const
      {
        if (isValid != other.isValid) return isValid < other.isValid;
        if (fuProcessId != other.fuProcessId) return fuProcessId < other.fuProcessId;
        return fuGuid < other.fuGuid;
      }
    };

    /**
     * Key that is used to identify output modules.
     */
    typedef unsigned int OutputModuleKey;


    /**
     * The set of information that is kept per output module.
     */
    struct OutputModuleRecord
    {
      std::string name;
      unsigned int id;
      unsigned int initMsgSize;
      unsigned int runNumber;
      MonitoredQuantity fragmentSize;
      MonitoredQuantity eventSize;
    };
    typedef boost::shared_ptr<OutputModuleRecord> OutModRecordPtr;
    typedef std::map<OutputModuleKey, OutModRecordPtr> OutputModuleRecordMap;

    /**
     * The set of information that is kept per filter unit.
     */
    struct FilterUnitRecord
    {
      FilterUnitKey key;
      OutputModuleRecordMap outputModuleMap;
      MonitoredQuantity eventSize;
      unsigned int initMsgCount;
      unsigned int lastEventNumber;

      explicit FilterUnitRecord(FilterUnitKey fuKey) :
        key(fuKey),
        initMsgCount(0)
      {}
    };
    typedef boost::shared_ptr<FilterUnitRecord> FURecordPtr;

    /**
     * The set of information that is kept per resource broker.
     */
    struct ResourceBrokerRecord
    {
      ResourceBrokerKey key;
      std::map<FilterUnitKey, FURecordPtr> filterUnitMap;
      OutputModuleRecordMap outputModuleMap;
      MonitoredQuantity eventSize;
      unsigned int initMsgCount;
      unsigned int lastEventNumber;

      explicit ResourceBrokerRecord(ResourceBrokerKey rbKey) :
        key(rbKey),
        initMsgCount(0)
      {}
    };
    typedef boost::shared_ptr<ResourceBrokerRecord> RBRecordPtr;


    /**
     * Results for a given output module.
     */
    struct OutputModuleResult
    {
      std::string name;
      unsigned int id;
      MonitoredQuantity::Stats eventStats;
    };
    typedef std::vector<OutputModuleResult> OutputModuleResultsList;


    /**
     * Constructor.
     */
    explicit DataSenderMonitorCollection(xdaq::Application*);

    /**
     * Adds the specified fragment to the monitor collection.
     */
    void addFragmentSample(I2OChain const&);

    /**
     * Adds the specified (complete) INIT message to the monitor collection.
     */
    void addInitSample(I2OChain const&);

    /**
     * Adds the specified (complete) Event to the monitor collection.
     */
    void addEventSample(I2OChain const&);

    /**
     * Fetches the top-level output module statistics.
     */
    OutputModuleResultsList getTopLevelOutputModuleResults() const;

  private:

    //Prevent copying of the DataSenderMonitorCollection
    DataSenderMonitorCollection(DataSenderMonitorCollection const&);
    DataSenderMonitorCollection& operator=(DataSenderMonitorCollection const&);

    virtual void do_calculateStatistics();

    virtual void do_updateInfoSpace();

    virtual void do_reset();

    bool getAllNeededPointers(I2OChain const& i2oChain,
                              RBRecordPtr& rbRecordPtr,
                              FURecordPtr& fuRecordPtr,
                              OutModRecordPtr& topLevelOutModPtr,
                              OutModRecordPtr& rbSpecificOutModPtr,
                              OutModRecordPtr& fuSpecificOutModPtr);

    RBRecordPtr getResourceBrokerRecord(ResourceBrokerKey const&);

    FURecordPtr getFilterUnitRecord(RBRecordPtr&, FilterUnitKey const&);

    OutModRecordPtr getOutputModuleRecord(OutputModuleRecordMap&,
                                          OutputModuleKey const&);

    void calcStatsForOutputModules(OutputModuleRecordMap& outputModuleMap);

    std::map<ResourceBrokerKey, RBRecordPtr> _resourceBrokerMap;
    OutputModuleRecordMap _outputModuleMap;

    mutable boost::mutex _collectionsMutex;

  };
  
} // namespace stor

#endif // StorageManager_DataSenderMonitorCollection_h 


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -