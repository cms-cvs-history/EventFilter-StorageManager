// -*- c++ -*-
// $Id: RegistrationInfoBase.h,v 1.1.2.3 2009/04/01 18:44:55 paterno Exp $

#ifndef REGISTRATIONINFOBASE_H
#define REGISTRATIONINFOBASE_H

#include <string>

#include "EventFilter/StorageManager/interface/EnquingPolicyTag.h"
#include "EventFilter/StorageManager/interface/QueueID.h"

namespace stor {

  class EventDistributor;

  /**
   * Defines the common interface for event and DQM consumer
   * registration info objects.
   *
   * $Author: paterno $
   * $Revision: 1.1.2.3 $
   * $Date: 2009/04/01 18:44:55 $
   */

  class RegistrationInfoBase
  {

  public:

    /**
       The virtual destructor allows polymorphic
       containment-by-reference.
    */
    virtual ~RegistrationInfoBase();

    /**
       Register the consumer represented by this registration with the
       specified EventDistributor.
    */
    void registerMe(EventDistributor* dist);

    /**
     * Returns the ID of the queue corresponding to this consumer
     * registration.
     */
   QueueID queueId() const;

    /**
     * Returns the enquing policy requested by the consumer
     * registration.
     */
    enquing_policy::PolicyTag queuePolicy() const;

    /**
       Returns the name supplied by the consumer.
     */
    std::string consumerName() const;

    /**
       Returns the header retry interval specified by the consumer.
     */
    unsigned int headerRetryInterval() const;

    /**
       Return the maximum event request rate (in Hz).
    */
    double maxEventRequestRate() const;

  private:
    virtual void do_registerMe(EventDistributor*) = 0;
    virtual QueueID do_queueId() const = 0;
    virtual std::string do_consumerName() const = 0;
    virtual unsigned int do_headerRetryInterval() const = 0;
    virtual double do_maxEventRequestRate() const = 0;
  };

  inline
  void RegistrationInfoBase::registerMe(EventDistributor* dist)
  {
    do_registerMe(dist);
  }

  inline
  QueueID RegistrationInfoBase::queueId() const
  {
    return do_queueId();
  }

  inline
  enquing_policy::PolicyTag RegistrationInfoBase::queuePolicy() const
  {
    return do_queueId().policy();
  }

  inline
  std::string RegistrationInfoBase::consumerName() const
  {
    return do_consumerName();
  }

  inline
  unsigned int RegistrationInfoBase::headerRetryInterval() const
  {
    return do_headerRetryInterval();
  }

  inline
  double RegistrationInfoBase::maxEventRequestRate() const
  {
    return do_maxEventRequestRate();
  }

} // namespace stor

#endif
