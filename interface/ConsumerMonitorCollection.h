// -*- c++ -*-
// $Id: ConsumerMonitorCollection.h,v 1.1.2.3 2009/05/06 09:23:26 dshpakov Exp $

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

    typedef std::map< ConsumerID, boost::shared_ptr<MonitoredQuantity> > ConsStatMap;

    ConsStatMap _qmap; // queued
    ConsStatMap _smap; // served

    mutable boost::mutex _mutex;

  };

} // namespace stor

#endif
