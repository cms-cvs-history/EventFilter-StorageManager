// $Id: StatisticsReporter.h,v 1.1.2.3 2009/01/30 10:49:40 mommsen Exp $

#ifndef StorageManager_StatisticsReporter_h
#define StorageManager_StatisticsReporter_h

#include "toolbox/lang/Class.h"
#include "toolbox/task/WaitingWorkLoop.h"
#include "xdaq/Application.h"

#include "EventFilter/StorageManager/interface/FragmentMonitorCollection.h"
#include "EventFilter/StorageManager/interface/RunMonitorCollection.h"


namespace stor {

  /**
   * Singleton to keep track of all monitoring and statistics issues
   *
   * This class also starts the monitoring workloop to update the 
   * statistics for all MonitorCollections.
   *
   * $Author: mommsen $
   * $Revision: 1.1.2.3 $
   * $Date: 2009/01/30 10:49:40 $
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


  private:

    //Prevent copying of the StatisticsReporter
    StatisticsReporter(StatisticsReporter const&);
    StatisticsReporter& operator=(StatisticsReporter const&);

    bool monitorAction(toolbox::task::WorkLoop*);

    xdaq::Application* _app;
    RunMonitorCollection _runMonCollection;
    FragmentMonitorCollection _fragMonCollection;
    toolbox::task::WorkLoop* _monitorWL;      
    bool _doMonitoring;

  };
  
} // namespace stor

#endif // StorageManager_StatisticsReporter_h 


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
