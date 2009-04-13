// -*- c++ -*-
// $Id: $

/**
   Free helper functions for handling consumer header and event
   requests and responses
 */

#ifndef CONSUMERUTILS_H
#define CONSUMERUTILS_H

#include "EventFilter/StorageManager/interface/ConsumerID.h"
#include "EventFilter/StorageManager/interface/EventConsumerRegistrationInfo.h"
#include "EventFilter/StorageManager/interface/InitMsgCollection.h"
#include "EventFilter/StorageManager/interface/I2OChain.h"

#include <boost/shared_ptr.hpp>

namespace xgi
{
  class Input;
  class Output;
}

namespace stor
{

  typedef boost::shared_ptr<stor::EventConsumerRegistrationInfo> ConsRegPtr;

  /**
     Parse consumer registration request:
  */
  ConsRegPtr parseEventConsumerRegistration( xgi::Input* in,
					     utils::duration_t secondsToStale );

  /**
     Send ID to consumer:
  */
  void writeEventConsumerRegistration( xgi::Output*, ConsumerID );

  /**
     Tell consumer we're not ready:
  */
  void writeNotReady( xgi::Output* );

  /**
     Send empty buffer to consumer:
  */
  void writeEmptyBuffer( xgi::Output* );

  /**
     Write HTTP headers:
  */
  void writeHTTPHeaders( xgi::Output* );

  /**
     Extract consumer ID from header request:
  */
  ConsumerID getConsumerID( xgi::Input* );

  /**
     Send header to consumer:
  */
  void writeConsumerHeader( xgi::Output*, InitMsgSharedPtr );

  /**
     Send event to consumer:
  */
  void writeConsumerEvent( xgi::Output*, const I2OChain& );

}

#endif
