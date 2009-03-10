// -*- c++ -*-
// $Id: RegistrationInfoBase.h,v 1.1.2.1 2009/03/09 20:30:58 biery Exp $

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
   * $Author: mommsen $
   * $Revision: 1.1.2.26 $
   * $Date: 2009/03/10 15:32:59 $
   */

  class RegistrationInfoBase
  {

  public:

    virtual ~RegistrationInfoBase() {}

    /**
     * Returns the ID of the queue corresponding to this consumer
     * registration.
     */
    virtual QueueID queueId() const = 0;

    /**
     * Returns the enquing policy requested by the consumer registration.
     */
    virtual enquing_policy::PolicyTag queuePolicy() const = 0;

    /**
     * Registers this registration with the event distributor.
     */
    virtual void registerMe(EventDistributor*) = 0;

  };

} // namespace stor

#endif
