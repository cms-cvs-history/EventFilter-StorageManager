// -*- c++ -*-
// $Id: RegistrationInfoBase.h,v 1.1.2.1 2009/03/10 20:39:44 biery Exp $

#ifndef REGISTRATIONINFOBASE_H
#define REGISTRATIONINFOBASE_H

#include "EventFilter/StorageManager/interface/EnquingPolicyTag.h"
#include "EventFilter/StorageManager/interface/QueueID.h"

namespace stor {

  class EventDistributor;

  /**
   * Defines the common interface for event and DQM consumer
   * registration info objects.
   *
   * $Author: biery $
   * $Revision: 1.1.2.1 $
   * $Date: 2009/03/10 20:39:44 $
   */

  class RegistrationInfoBase
  {

  public:

    virtual ~RegistrationInfoBase() {}

    /**
     * Returns the ID of the queue corresponding to this consumer
     * registration.
     */
    virtual const QueueID& queueId() const = 0;

    /**
     * Returns the enquing policy requested by the consumer registration.
     */
    virtual const enquing_policy::PolicyTag& queuePolicy() const = 0;

    /**
     * Registers this registration with the event distributor.
     */
    virtual void registerMe(EventDistributor*) = 0;

  };

} // namespace stor

#endif
