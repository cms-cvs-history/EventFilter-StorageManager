// $Id: StatisticsReporter.h,v 1.1.2.3 2009/03/20 10:25:06 mommsen Exp $

#ifndef StorageManager_StatisticsReporter_h
#define StorageManager_StatisticsReporter_h

#include "toolbox/lang/Class.h"
#include "toolbox/task/WaitingWorkLoop.h"
#include "xdaq/Application.h"

#include "EventFilter/StorageManager/interface/FilesMonitorCollection.h"
#include "EventFilter/StorageManager/interface/FragmentMonitorCollection.h"
#include "EventFilter/StorageManager/interface/RunMonitorCollection.h"

#include <string>

namespace stor {

  /**
   * Singleton to keep track of all monitoring and statistics issues
   *
   * This class also starts the monitoring workloop to update the 
   * statistics for all MonitorCollections.
   *
   * $Author: mommsen $
   * $Revision: 1.1.2.3 $
   * $Date: 2009/03/20 10:25:06 $
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


    // Current state name:
    const std::string& currentStateName() const { return _currentStateName; }
    void setCurrentStateName( const std::string& n ) { _currentStateName = n; }

    /**
     * Create and start the monitoring workloop
     */
    void startWorkLoop();


  private:

    //Prevent copying of the StatisticsReporter
    StatisticsReporter(StatisticsReporter const&);
    StatisticsReporter& operator=(StatisticsReporter const&);

    bool monitorAction(toolbox::task::WorkLoop*);

    xdaq::Application* _app;
    RunMonitorCollection _runMonCollection;
    FragmentMonitorCollection _fragMonCollection;
    FilesMonitorCollection _filesMonCollection;
    toolbox::task::WorkLoop* _monitorWL;      
    bool _doMonitoring;

    std::string _currentStateName;

  };
  
} // namespace stor

#endif // StorageManager_StatisticsReporter_h 


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
