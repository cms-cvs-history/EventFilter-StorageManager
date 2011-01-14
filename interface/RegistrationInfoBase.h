// $Id: RegistrationInfoBase.h,v 1.6.2.1 2011/01/13 13:28:41 mommsen Exp $
/// @file: RegistrationInfoBase.h 

#ifndef StorageManager_RegistrationInfoBase_h
#define StorageManager_RegistrationInfoBase_h

#include <string>

#include <boost/shared_ptr.hpp>

#include "EventFilter/StorageManager/interface/ConsumerID.h"
#include "EventFilter/StorageManager/interface/EnquingPolicyTag.h"
#include "EventFilter/StorageManager/interface/QueueID.h"
#include "EventFilter/StorageManager/interface/Utils.h"

namespace stor {

  class EventDistributor;

  /**
   * Defines the common interface for event and DQM consumer
   * registration info objects.
   *
   * $Author: mommsen $
   * $Revision: 1.6.2.1 $
   * $Date: 2011/01/13 13:28:41 $
   */

  class RegistrationInfoBase
  {

  public:

    /**
       The virtual destructor allows polymorphic
       containment-by-reference.
    */
    virtual ~RegistrationInfoBase() {};

    /**
       Mark time when consumer last contacted us
    */
    inline void consumerContact()
    { _lastConsumerContact = utils::getCurrentTime(); }

    /**
       Return whether or not *this is a valid registration
    */
    bool isValid();

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
       Set the consumer ID.
     */
    void setQueueId(QueueID const& id);

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
       Returns the hostname of the consumer.
     */
    std::string remoteHost() const;

    /**
       Returns the ID given to this consumer.
     */
    ConsumerID consumerId() const;

    /**
       Set the consumer ID.
     */
    void setConsumerId(const ConsumerID& id);

    /**
       Returns the queue Size
     */
    int queueSize() const;

    /**
       Returns the time until the consumer becomes stale
     */
    utils::duration_t secondsToStale() const;

    /**
       Returns if the consumer is stale at the given point in time
     */
    bool isStale(const utils::time_point_t& now) const
    { return ( now > _lastConsumerContact + secondsToStale() ); }


  private:
    virtual void do_registerMe(EventDistributor*) = 0;
    virtual QueueID do_queueId() const = 0;
    virtual void do_setQueueId(QueueID const& id) = 0;
    virtual std::string do_consumerName() const = 0;
    virtual std::string do_remoteHost() const = 0;
    virtual ConsumerID do_consumerId() const = 0;
    virtual void do_setConsumerId(ConsumerID const& id) = 0;
    virtual int do_queueSize() const = 0;
    virtual enquing_policy::PolicyTag do_queuePolicy() const = 0;
    virtual utils::duration_t do_secondsToStale() const = 0;

    utils::time_point_t _lastConsumerContact;
  };

  typedef boost::shared_ptr<stor::RegistrationInfoBase> RegPtr;

  inline
  bool RegistrationInfoBase::isValid()
  {
    return consumerId().isValid();
  }

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
  void RegistrationInfoBase::setQueueId(QueueID const& id)
  {
    do_setQueueId(id);
  }

  inline
  enquing_policy::PolicyTag RegistrationInfoBase::queuePolicy() const
  {
    return do_queuePolicy();
  }

  inline
  std::string RegistrationInfoBase::consumerName() const
  {
    return do_consumerName();
  }

  inline
  std::string RegistrationInfoBase::remoteHost() const
  {
    return do_remoteHost();
  }

  inline
  ConsumerID RegistrationInfoBase::consumerId() const
  {
    return do_consumerId();
  }

  inline
  void RegistrationInfoBase::setConsumerId(ConsumerID const& id)
  {
    do_setConsumerId(id);
  }

  inline
  int RegistrationInfoBase::queueSize() const
  {
    return do_queueSize();
  }

  inline
  utils::duration_t RegistrationInfoBase::secondsToStale() const
  {
    return do_secondsToStale();
  }

} // namespace stor

#endif // StorageManager_RegistrationInfoBase_h


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
