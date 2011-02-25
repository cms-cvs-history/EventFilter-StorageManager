// $Id: RegistrationInfoBase.h,v 1.6.2.7 2011/02/24 13:37:13 mommsen Exp $
/// @file: RegistrationInfoBase.h 

#ifndef EventFilter_StorageManager_RegistrationInfoBase_h
#define EventFilter_StorageManager_RegistrationInfoBase_h

#include <string>

#include <boost/shared_ptr.hpp>

#include "EventFilter/StorageManager/interface/Configuration.h"
#include "EventFilter/StorageManager/interface/ConsumerID.h"
#include "EventFilter/StorageManager/interface/EnquingPolicyTag.h"
#include "EventFilter/StorageManager/interface/QueueID.h"
#include "EventFilter/StorageManager/interface/Utils.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"


namespace stor {

  class EventDistributor;

  /**
   * Defines the common interface for event and DQM consumer
   * registration info objects.
   *
   * $Author: mommsen $
   * $Revision: 1.6.2.7 $
   * $Date: 2011/02/24 13:37:13 $
   */

  class RegistrationInfoBase
  {

  public:
    
    RegistrationInfoBase
    (
      const std::string& consumerName,
      const std::string& remoteHost,
      const int& queueSize,
      const enquing_policy::PolicyTag& queuePolicy,
      const utils::duration_t& secondsToStale
    );

    RegistrationInfoBase
    (
      const edm::ParameterSet& pset,
      const std::string& remoteHost,
      const EventServingParams& eventServingParams,
      const bool useEventServingParams
    );

    /**
       The virtual destructor allows polymorphic
       containment-by-reference.
    */
    virtual ~RegistrationInfoBase() {};

    /**
       Mark time when consumer last contacted us
    */
    void consumerContact();

    /**
       Register the consumer represented by this registration with the
       specified EventDistributor.
    */
    void registerMe(EventDistributor* dist);

    /**
       Returns a formatted string which contains the information about the event type.
     */
    void eventType(std::ostream&) const;

    /**
      Return the ParameterSet containing the consumer registration infos
    */
    edm::ParameterSet getPSet() const;

    /**
      Print queue information into ostream
     */
    void queueInfo(std::ostream&) const;

    // Accessors
    bool isValid() const { return _consumerId.isValid(); }
    const QueueID& queueId() const { return _queueId; }
    const enquing_policy::PolicyTag& queuePolicy() const { return _queuePolicy; }
    const std::string& consumerName() const { return _consumerName; }
    const std::string& remoteHost() const { return _remoteHost; }
    const std::string& sourceURL() const { return _sourceURL; }
    const ConsumerID& consumerId() const { return _consumerId; }
    const int& queueSize() const { return _queueSize; }
    const int& maxConnectTries() const { return _maxConnectTries; }
    const int& connectTrySleepTime() const { return _connectTrySleepTime; }
    const int& retryInterval() const { return _retryInterval; }
    const utils::duration_t& secondsToStale() const { return _secondsToStale; }
    bool isStale(const utils::time_point_t&) const;
    double lastContactSecondsAgo(const utils::time_point_t&) const;

    // Setters
    void setQueueId(const QueueID& id) { _queueId = id; }
    void setSourceURL(const std::string& url) { _sourceURL = url; }
    void setConsumerId(const ConsumerID& id) { _consumerId = id; }

    // Comparison:
    virtual bool operator<(const RegistrationInfoBase&) const;
    virtual bool operator==(const RegistrationInfoBase&) const;
    virtual bool operator!=(const RegistrationInfoBase&) const;


  protected:

    virtual void do_registerMe(EventDistributor*) = 0;
    virtual void do_eventType(std::ostream&) const = 0;
    virtual void do_appendToPSet(edm::ParameterSet&) const = 0;


  private:

    const std::string                _remoteHost;
    std::string                      _consumerName;
    std::string                      _sourceURL;
    int                              _queueSize;
    enquing_policy::PolicyTag        _queuePolicy;
    utils::duration_t                _secondsToStale;
    int                              _maxConnectTries;
    int                              _connectTrySleepTime;
    int                              _retryInterval;
    QueueID                          _queueId;
    ConsumerID                       _consumerId;
    utils::time_point_t              _lastConsumerContact;
  };

  typedef boost::shared_ptr<RegistrationInfoBase> RegPtr;


  inline
  void RegistrationInfoBase::consumerContact()
  {
    _lastConsumerContact = utils::getCurrentTime();
  }

  inline
  void RegistrationInfoBase::registerMe(EventDistributor* dist)
  {
    do_registerMe(dist);
  }

  inline
  void RegistrationInfoBase::eventType(std::ostream& os) const
  {
    do_eventType(os);
  }

  inline
  bool RegistrationInfoBase::isStale(const utils::time_point_t& now) const
  {
    return ( now > _lastConsumerContact + secondsToStale() );
  }

  inline
  double RegistrationInfoBase::lastContactSecondsAgo(const utils::time_point_t& now) const
  {
    return utils::duration_to_seconds( now - _lastConsumerContact );
  }

  std::ostream& operator<<(std::ostream& os, 
                           RegistrationInfoBase const& ri);

} // namespace stor

#endif // EventFilter_StorageManager_RegistrationInfoBase_h


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
