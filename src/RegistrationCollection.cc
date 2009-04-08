// $Id: $

#include "EventFilter/StorageManager/interface/RegistrationCollection.h"

using namespace stor;

RegistrationCollection::RegistrationCollection()
{
  boost::mutex::scoped_lock sl( _lock );
  _nextConsumerID = ConsumerID(1);
  _registrationAllowed = false;
}

RegistrationCollection::~RegistrationCollection() {}

ConsumerID RegistrationCollection::getConsumerID()
{
  boost::mutex::scoped_lock sl( _lock );
  _nextConsumerID++;
  return _nextConsumerID;
}

bool
RegistrationCollection::addRegistrationInfo( ConsumerID cid, ConsRegPtr ri )
{
  boost::mutex::scoped_lock sl( _lock );
  if( _registrationAllowed )
    {
      _consumers[ cid ] = ri;
      return true;
    }
  else
    {
      return false;
    }
}

ConsRegPtr RegistrationCollection::getConsumer( ConsumerID cid ) const
{
  boost::mutex::scoped_lock sl( _lock );
  ConsRegMap::const_iterator it = _consumers.find( cid );
  if( it != _consumers.end() )
    {
      return it->second;
    }
  else
    {
      return ConsRegPtr();
    }
}

void RegistrationCollection::enableConsumerRegistration()
{
  boost::mutex::scoped_lock sl( _lock );
  _registrationAllowed = true;
}

void RegistrationCollection::disableConsumerRegistration()
{
  boost::mutex::scoped_lock sl( _lock );
  _registrationAllowed = false;
}
