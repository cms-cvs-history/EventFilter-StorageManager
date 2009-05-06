// -*- c++ -*-
// $Id: ConsumerMonitorCollection.h,v 1.1.2.4 2009/05/06 10:05:46 dshpakov Exp $

#ifndef CONSUMERMONITORCOLLECTION
#define CONSUMERMONITORCOLLECTION

#include "EventFilter/StorageManager/interface/ConsumerID.h"
#include "EventFilter/StorageManager/interface/MonitoredQuantity.h"
#include "EventFilter/StorageManager/interface/MonitorCollection.h"

#include <boost/thread/mutex.hpp>
#include <boost/shared_ptr.hpp>

#include <map>

namespace stor
{

  class ConsumerMonitorCollection: public MonitorCollection
  {

  public:

    explicit ConsumerMonitorCollection( xdaq::Application* );

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

    // Prevent copying:
    ConsumerMonitorCollection( const ConsumerMonitorCollection& );
    ConsumerMonitorCollection& operator = ( const ConsumerMonitorCollection& );

    virtual void do_calculateStatistics();
    virtual void do_reset();
    virtual void do_updateInfoSpace();

    typedef std::map< ConsumerID, boost::shared_ptr<MonitoredQuantity> > ConsStatMap;

    ConsStatMap _qmap; // queued
    ConsStatMap _smap; // served

    mutable boost::mutex _mutex;

  };

} // namespace stor

#endif
