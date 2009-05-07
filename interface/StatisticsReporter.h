// $Id: StatisticsReporter.h,v 1.1.2.15 2009/05/06 10:05:46 dshpakov Exp $

#ifndef StorageManager_StatisticsReporter_h
#define StorageManager_StatisticsReporter_h

#include "toolbox/lang/Class.h"
#include "toolbox/task/WaitingWorkLoop.h"
#include "xdaq/Application.h"
#include "xdata/String.h"

#include "EventFilter/StorageManager/interface/DataSenderMonitorCollection.h"
#include "EventFilter/StorageManager/interface/DQMEventMonitorCollection.h"
#include "EventFilter/StorageManager/interface/FilesMonitorCollection.h"
#include "EventFilter/StorageManager/interface/FragmentMonitorCollection.h"
#include "EventFilter/StorageManager/interface/ResourceMonitorCollection.h"
#include "EventFilter/StorageManager/interface/RunMonitorCollection.h"
#include "EventFilter/StorageManager/interface/StreamsMonitorCollection.h"
#include "EventFilter/StorageManager/interface/ConsumerMonitorCollection.h"

#include "boost/shared_ptr.hpp"
#include "boost/thread/mutex.hpp"

#include <string>

namespace stor {

  /**
   * Singleton to keep track of all monitoring and statistics issues
   *
   * This class also starts the monitoring workloop to update the 
   * statistics for all MonitorCollections.
   *
   * $Author: dshpakov $
   * $Revision: 1.1.2.15 $
   * $Date: 2009/05/06 10:05:46 $
   */
  
  class StatisticsReporter : public toolbox::lang::Class
  {
  public:
    
    explicit StatisticsReporter(xdaq::Application*);
    
    ~StatisticsReporter();


    const RunMonitorCollection& getRunMonitorCollection() const
    { return _runMonCollection; }

    RunMonitorCollection& getRunMonitorCollection()
    { return _runMonCollection; }


    const FragmentMonitorCollection& getFragmentMonitorCollection() const
    { return _fragMonCollection; }

    FragmentMonitorCollection& getFragmentMonitorCollection()
    { return _fragMonCollection; }


    const FilesMonitorCollection& getFilesMonitorCollection() const
    { return _filesMonCollection; }

    FilesMonitorCollection& getFilesMonitorCollection()
    { return _filesMonCollection; }


    const StreamsMonitorCollection& getStreamsMonitorCollection() const
    { return _streamsMonCollection; }

    StreamsMonitorCollection& getStreamsMonitorCollection()
    { return _streamsMonCollection; }


    const DataSenderMonitorCollection& getDataSenderMonitorCollection() const
    { return _dataSenderMonCollection; }

    DataSenderMonitorCollection& getDataSenderMonitorCollection()
    { return _dataSenderMonCollection; }


    const DQMEventMonitorCollection& getDQMEventMonitorCollection() const
    { return _dqmEventMonCollection; }

    DQMEventMonitorCollection& getDQMEventMonitorCollection()
    { return _dqmEventMonCollection; }


    const ResourceMonitorCollection& getResourceMonitorCollection() const
    { return _resourceMonCollection; }

    ResourceMonitorCollection& getResourceMonitorCollection()
    { return _resourceMonCollection; }


    const ConsumerMonitorCollection& getConsumerMonitorCollection() const
    {
      return _consumerMonitorCollection;
    }

    ConsumerMonitorCollection& getConsumerMonitorCollection()
    {
      return _consumerMonitorCollection;
    }

    // Current state name:
    const std::string& externallyVisibleState() const;
    void setExternallyVisibleState( const std::string& );

    /**
     * Create and start the monitoring workloop
     */
    void startWorkLoop(std::string workloopName);

    /**
     * Reset all monitored quantities
     */
    void reset();


  private:

    //Prevent copying of the StatisticsReporter
    StatisticsReporter(StatisticsReporter const&);
    StatisticsReporter& operator=(StatisticsReporter const&);

    bool monitorAction(toolbox::task::WorkLoop*);

    xdaq::Application* _app;
    RunMonitorCollection _runMonCollection;
    FragmentMonitorCollection _fragMonCollection;
    FilesMonitorCollection _filesMonCollection;
    StreamsMonitorCollection _streamsMonCollection;
    DataSenderMonitorCollection _dataSenderMonCollection;
    DQMEventMonitorCollection _dqmEventMonCollection;
    ResourceMonitorCollection _resourceMonCollection;
    ConsumerMonitorCollection _consumerMonitorCollection;
    toolbox::task::WorkLoop* _monitorWL;      
    bool _doMonitoring;

    std::string _externallyVisibleState;
    mutable boost::mutex _state_name_lock;

    // State name for infospace updates:
    xdata::String _xdaq_state_name;
    void reportStateName();

    // Unused status string from old SM
    xdata::String _progressMarker;

  };

  typedef boost::shared_ptr<StatisticsReporter> StatisticsReporterPtr;
  
} // namespace stor

#endif // StorageManager_StatisticsReporter_h 


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
