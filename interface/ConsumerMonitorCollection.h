// -*- c++ -*-
// $Id: $

#ifndef CONSUMERMONITORCOLLECTION
#define CONSUMERMONITORCOLLECTION

#include "EventFilter/StorageManager/interface/ConsumerID.h"

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
    bool getQueued( ConsumerID cid, unsigned int& result );

    /**
       Get served data size. Return false if consumer ID not found.
    */
    bool getServed( ConsumerID cid, unsigned int& result );

    /**
       Reset sizes to zero leaving consumers in
    */
    void resetCounters();

    /**
       Clear all consumer data
    */
    void clearConsumers();

  private:

    typedef std::map<ConsumerID,unsigned int> ConsStatMap;

    ConsStatMap _qmap; // queued
    ConsStatMap _smap; // served

  };

} // namespace stor

#endif
