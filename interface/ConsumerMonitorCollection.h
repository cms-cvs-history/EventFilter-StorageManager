// -*- c++ -*-
// $Id: ConsumerMonitorCollection.h,v 1.1.2.2 2009/04/29 11:51:26 dshpakov Exp $

#ifndef CONSUMERMONITORCOLLECTION
#define CONSUMERMONITORCOLLECTION

#include "EventFilter/StorageManager/interface/ConsumerID.h"
#include "EventFilter/StorageManager/interface/MonitoredQuantity.h"

#include <boost/thread/mutex.hpp>
#include <boost/shared_ptr.hpp>

#include <map>

namespace stor
{

  class ConsumerMonitorCollection
  {

  public:

    ConsumerMonitorCollection() {}
    ~ConsumerMonitorCollection() {}

    /**
       Add queued sample
    */
    void addQueuedEventSample( ConsumerID cid, unsigned int data_size );

    /**
       Add served sample
    */
    void addServedEventSample( ConsumerID cid, unsigned int data_size );

    /**
       Get queued data size. Return false if consumer ID not found.
    */
    bool getQueued( ConsumerID cid, MonitoredQuantity::Stats& result );

    /**
       Get served data size. Return false if consumer ID not found.
    */
    bool getServed( ConsumerID cid, MonitoredQuantity::Stats& result );

    /**
       Reset sizes to zero leaving consumers in
    */
    void resetCounters();

    /**
       Clear all consumer data
    */
    void clearConsumers();

  private:

    // Stolen from other MonitorCollection's (to prevent copying):
    ConsumerMonitorCollection( const ConsumerMonitorCollection& );
    ConsumerMonitorCollection& operator = ( const ConsumerMonitorCollection& );

    typedef std::map< ConsumerID, boost::shared_ptr<MonitoredQuantity> > ConsStatMap;

    ConsStatMap _qmap; // queued
    ConsStatMap _smap; // served

    mutable boost::mutex _mutex;

  };

} // namespace stor

#endif
