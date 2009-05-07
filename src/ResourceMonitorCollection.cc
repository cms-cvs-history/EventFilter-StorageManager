// $Id: ResourceMonitorCollection.cc,v 1.1.2.10 2009/04/09 17:00:35 mommsen Exp $

#include <string>
#include <sstream>
#include <iomanip>
#include <sys/statfs.h>

#include "EventFilter/StorageManager/interface/Exception.h"
#include "EventFilter/StorageManager/interface/ResourceMonitorCollection.h"

using namespace stor;

ResourceMonitorCollection::ResourceMonitorCollection(xdaq::Application *app) :
MonitorCollection(app),
_pool(0)
{
  putItemsIntoInfoSpace();
}


void ResourceMonitorCollection::configureDisks(DiskWritingParams const& dwParams)
{
  boost::mutex::scoped_lock sl(_diskUsageListMutex);

  _highWaterMark = dwParams._highWaterMark;

  int nLogicalDisk = dwParams._nLogicalDisk;
  unsigned int nD = nLogicalDisk ? nLogicalDisk : 1;
  _diskUsageList.clear();
  _diskUsageList.reserve(nD);

  for (unsigned int i=0; i<nD; ++i) {

    DiskUsagePtr diskUsage(new DiskUsage);
    diskUsage->pathName = dwParams._filePath;
    if(nLogicalDisk>0) {
      std::ostringstream oss;
      oss << "/" << std::setfill('0') << std::setw(2) << i; 
      diskUsage->pathName += oss.str();
    }

    diskUsage->diskSize = 0;
    struct statfs64 buf;
    int retVal = statfs64(diskUsage->pathName.c_str(), &buf);
    if(retVal==0) {
      unsigned int blksize = buf.f_bsize;
      diskUsage->diskSize = buf.f_blocks * blksize / 1024 / 1024 /1024;
    }
    _diskUsageList.push_back(diskUsage);
  }
}

void ResourceMonitorCollection::setMemoryPoolPointer(toolbox::mem::Pool* pool)
{
  if ( ! _pool)
    _pool = pool;
}


void ResourceMonitorCollection::getStats(Stats& stats) const
{
  getDiskStats(stats);

  _poolUsage.getStats(stats.poolUsageStats);
  _numberOfCopyWorkers.getStats(stats.numberOfCopyWorkersStats);
  _numberOfInjectWorkers.getStats(stats.numberOfInjectWorkersStats);
}


void  ResourceMonitorCollection::getDiskStats(Stats& stats) const
{
  boost::mutex::scoped_lock sl(_diskUsageListMutex);

  stats.diskUsageStatsList.clear();
  stats.diskUsageStatsList.reserve(_diskUsageList.size());
  for ( DiskUsagePtrList::const_iterator it = _diskUsageList.begin(),
          itEnd = _diskUsageList.end();
        it != itEnd;
        ++it)
  {
    DiskUsageStatsPtr diskUsageStats(new DiskUsageStats);
    (*it)->absDiskUsage.getStats(diskUsageStats->absDiskUsageStats);
    (*it)->relDiskUsage.getStats(diskUsageStats->relDiskUsageStats);
    diskUsageStats->diskSize = (*it)->diskSize;
    diskUsageStats->pathName = (*it)->pathName;
    diskUsageStats->warningColor = (*it)->warningColor;
    stats.diskUsageStatsList.push_back(diskUsageStats);
  }
}


void ResourceMonitorCollection::do_calculateStatistics()
{
  calcPoolUsage();
  calcDiskUsage();
  calcNumberOfWorkers();
}


void ResourceMonitorCollection::calcPoolUsage()
{
  if (_pool)
  {
    try {
      _pool->lock();
      _poolUsage.addSample( _pool->getMemoryUsage().getUsed() );
      _pool->unlock();
    }
    catch (...)
    {
      _pool->unlock();
    }
  }
  _poolUsage.calculateStatistics();
}


void ResourceMonitorCollection::calcDiskUsage()
{
  boost::mutex::scoped_lock sl(_diskUsageListMutex);

  for ( DiskUsagePtrList::iterator it = _diskUsageList.begin(),
          itEnd = _diskUsageList.end();
        it != itEnd;
        ++it)
  {
    struct statfs64 buf;
    int retVal = statfs64((*it)->pathName.c_str(), &buf);
    if(retVal==0) {
      unsigned int blksize = buf.f_bsize;
      double absused = 
        (*it)->diskSize -
        buf.f_bavail  * blksize / 1024 / 1024 /1024;
      double relused = (100 * (absused / (*it)->diskSize)); 
      (*it)->absDiskUsage.addSample(absused);
      (*it)->absDiskUsage.calculateStatistics();
      (*it)->relDiskUsage.addSample(relused);
      (*it)->relDiskUsage.calculateStatistics();
      if (relused > _highWaterMark*100)
      {
        (*it)->warningColor = "#EF5A10";
        // TODO: add sentinel warning
      }
      else
      {
        (*it)->warningColor = "#FFFFFF";
      }
    }
  }
}


void ResourceMonitorCollection::calcNumberOfWorkers()
{
  _numberOfCopyWorkers.addSample( getProcessCount("CopyWorker.pl") );
  _numberOfInjectWorkers.addSample( getProcessCount("InjectWorker.pl") );

  _numberOfCopyWorkers.calculateStatistics();
  _numberOfInjectWorkers.calculateStatistics();
}


void ResourceMonitorCollection::do_updateInfoSpace()
{
  //nothing to do
}


void ResourceMonitorCollection::do_reset()
{
  _poolUsage.reset();
  _numberOfCopyWorkers.reset();
  _numberOfInjectWorkers.reset();

  boost::mutex::scoped_lock sl(_diskUsageListMutex);
  for ( DiskUsagePtrList::const_iterator it = _diskUsageList.begin(),
          itEnd = _diskUsageList.end();
        it != itEnd;
        ++it)
  {
    (*it)->absDiskUsage.reset();
    (*it)->relDiskUsage.reset();
    (*it)->warningColor = "#FFFFFF";
  }
}


unsigned int ResourceMonitorCollection::getProcessCount(std::string processName)
{

  int count = -1;
  char buf[128];
  std::string command = "ps -C " + processName + " --no-header | wc -l";

  FILE *fp = popen(command.c_str(), "r");
  if ( fp )
  {
    if ( fgets(buf, sizeof(buf), fp) )
    {
      count = strtol(buf, '\0', 10);
    }
    pclose( fp );
  }
  return count;
}


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
