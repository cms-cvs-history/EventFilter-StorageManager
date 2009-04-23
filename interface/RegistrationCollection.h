// -*- c++ -*-
// $Id: RegistrationCollection.h,v 1.1.2.5 2009/04/21 21:23:05 biery Exp $

#ifndef REGISTRATIONCOLLECTION_H
#define REGISTRATIONCOLLECTION_H

#include "EventFilter/StorageManager/interface/ConsumerID.h"
#include "EventFilter/StorageManager/interface/RegistrationInfoBase.h"
#include "EventFilter/StorageManager/interface/EventConsumerRegistrationInfo.h"
#include "EventFilter/StorageManager/interface/ConsumerUtils.h"

#include <vector>
#include <boost/thread/mutex.hpp>
#include <boost/shared_ptr.hpp>

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
       Add registration info. Return false if no registration is allowed.
    */
    bool addRegistrationInfo( ConsumerID,
			      boost::shared_ptr<RegistrationInfoBase> );

    /**
       Get consumer registrations.
    */
    typedef std::vector<stor::ConsRegPtr> ConsumerRegistrations;
    void getEventConsumers( ConsumerRegistrations& );

    /**
       Enable registration.
    */
    void enableConsumerRegistration();

    /**
       Disable registration.
    */
    void disableConsumerRegistration();

    /**
       Clear registrations.
    */
    void clearRegistrations();

    /**
       Test if registration is allowed.
    */
    bool registrationIsAllowed() const;

    /**
       Test if consumer is a proxy.
    */
    bool isProxy( ConsumerID ) const;

  private:

    mutable boost::mutex _lock;

    ConsumerID _nextConsumerID;

    bool _registrationAllowed;

    ConsumerRegistrations _consumers;

  };

} // namespace stor

#endif
