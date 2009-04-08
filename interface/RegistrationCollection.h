// -*- c++ -*-
// $Id: $

#ifndef REGISTRATIONCOLLECTION_H
#define REGISTRATIONCOLLECTION_H

#include "EventFilter/StorageManager/interface/ConsumerID.h"
#include "EventFilter/StorageManager/interface/EventConsumerRegistrationInfo.h"

#include <map>
#include <boost/thread/mutex.hpp>

namespace stor
{

  class RegistrationCollection
  {

  public:

    RegistrationCollection();

    ~RegistrationCollection();

    /**
       Return next available consumer ID or 0 if no registration is
       allowed.
    */
    ConsumerID getConsumerID();

    /**
       Add consumer registration. Return false if no registration is allowed.
    */
    bool addRegistrationInfo( ConsumerID, stor::ConsRegPtr );

    /**
       Get consumer registration. Return null if not there.
    */
    stor::ConsRegPtr getConsumer( ConsumerID ) const;

    /**
       Enable registration.
    */
    void enableConsumerRegistration();

    /**
       Disable registration.
    */
    void disableConsumerRegistration();

  private:

    mutable boost::mutex _lock;

    ConsumerID _nextConsumerID;

    bool _registrationAllowed;

    typedef std::map<ConsumerID,stor::ConsRegPtr>
    ConsRegMap;
    ConsRegMap _consumers;

  };

} // namespace stor

#endif
