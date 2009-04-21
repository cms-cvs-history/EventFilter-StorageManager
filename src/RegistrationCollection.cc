// $Id: RegistrationCollection.cc,v 1.1.2.4 2009/04/16 10:42:21 dshpakov Exp $

#include "EventFilter/StorageManager/interface/RegistrationCollection.h"

#include <boost/pointer_cast.hpp>

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

  if( !_registrationAllowed )
    {
      return ConsumerID(0);
    }

  _nextConsumerID++;
  return _nextConsumerID;

}

bool
RegistrationCollection::addRegistrationInfo( ConsumerID cid,
					     boost::shared_ptr<RegistrationInfoBase> ri )
{
  boost::mutex::scoped_lock sl( _lock );
  if( _registrationAllowed )
    {
      _consumers.push_back( boost::dynamic_pointer_cast<EventConsumerRegistrationInfo>( ri ) );
      return true;
    }
  else
    {
      return false;
    }
}

void RegistrationCollection::getEventConsumers( ConsumerRegistrations& crs )
{
  boost::mutex::scoped_lock sl( _lock );
  for( ConsumerRegistrations::const_iterator it = _consumers.begin();
       it != _consumers.end(); ++it )
    {
      crs.push_back( *it );
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

bool RegistrationCollection::registrationIsAllowed()
{
  boost::mutex::scoped_lock sl( _lock );
  return _registrationAllowed;
}

bool RegistrationCollection::isProxy( ConsumerID cid ) const
{
  // Ugly. Should probably switch to a map.
  for( ConsumerRegistrations::const_iterator it = _consumers.begin();
       it != _consumers.end(); ++it )
    {
      if( (*it)->consumerID() == cid )
	{
	  if( (*it)->isProxyServer() )
	    {
	      return true;
	    }
	}
    }
  return false;
}
