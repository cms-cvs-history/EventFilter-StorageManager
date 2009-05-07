// $Id: ResourceMonitorCollection.h,v 1.1.2.9 2009/04/09 17:00:58 mommsen Exp $

#ifndef StorageManager_ResourceMonitorCollection_h
#define StorageManager_ResourceMonitorCollection_h

#include <vector>
#include <string>

#include <boost/thread/mutex.hpp>
#include <boost/shared_ptr.hpp>

#include "toolbox/mem/Pool.h"

#include "EventFilter/StorageManager/interface/Configuration.h"
#include "EventFilter/StorageManager/interface/MonitorCollection.h"


namespace stor {

  /**
   * A collection of MonitoredQuantities related to resource usages
   *
   * $Author: mommsen $
   * $Revision: 1.1.2.9 $
   * $Date: 2009/04/09 17:00:58 $
   */
  
  class ResourceMonitorCollection : public MonitorCollection
  {
  public:

    struct DiskUsageStats
    {
      MonitoredQuantity::Stats absDiskUsageStats;  // absolute disk usage in GB
      MonitoredQuantity::Stats relDiskUsageStats;  // percentage of disk space occupied
      size_t diskSize;                             // absolute size of disk in GB
      std::string pathName;                        // path of the disk
      std::string warningColor;                    // HTML color code for disk usage warning
    };
    typedef boost::shared_ptr<DiskUsageStats> DiskUsageStatsPtr;
    typedef std::vector<DiskUsageStatsPtr> DiskUsageStatsPtrList;

    struct Stats
    {
      DiskUsageStatsPtrList diskUsageStatsList;

      MonitoredQuantity::Stats poolUsageStats; // I2O message pool usage in bytes
      MonitoredQuantity::Stats numberOfCopyWorkersStats;
      MonitoredQuantity::Stats numberOfInjectWorkersStats;
    };


    explicit ResourceMonitorCollection(xdaq::Application*);

    /**
     * Stores the given memory pool pointer if not yet set.
     * If it is already set, the argument is ignored.
     */
    void setMemoryPoolPointer(toolbox::mem::Pool*);

    /**
     * Configures the disks used to write events
     */
    void configureDisks(DiskWritingParams const&);

    /**
     * Write all our collected statistics into the given Stats struct.
     */
    void getStats(Stats&) const;


  private:

    //Prevent copying of the ResourceMonitorCollection
    ResourceMonitorCollection(ResourceMonitorCollection const&);
    ResourceMonitorCollection& operator=(ResourceMonitorCollection const&);

    virtual void do_calculateStatistics();
    
    virtual void do_updateInfoSpace();
    
    virtual void do_reset();

    void getDiskStats(Stats&) const;
    void calcPoolUsage();
    void calcDiskUsage();
    void calcNumberOfWorkers();
    unsigned int getProcessCount(std::string processName);

    toolbox::mem::Pool* _pool;
    double _highWaterMark;     //percentage of disk full when issuing an alarm

    struct DiskUsage
    {
      MonitoredQuantity absDiskUsage;
      MonitoredQuantity relDiskUsage;
      size_t diskSize;
      std::string pathName;
      std::string warningColor;
    };
    typedef boost::shared_ptr<DiskUsage> DiskUsagePtr;
    typedef std::vector<DiskUsagePtr> DiskUsagePtrList;
    DiskUsagePtrList _diskUsageList;
    mutable boost::mutex _diskUsageListMutex;

    MonitoredQuantity _poolUsage;
    MonitoredQuantity _numberOfCopyWorkers;
    MonitoredQuantity _numberOfInjectWorkers;

  };
  
} // namespace stor

#endif // StorageManager_ResourceMonitorCollection_h 


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
