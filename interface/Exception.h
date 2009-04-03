// $Id: Exception.h,v 1.1.2.9 2009/03/27 01:26:58 biery Exp $

#ifndef _storagemanager_Exception_h_
#define _storagemanager_Exception_h_


#include "xcept/Exception.h"

// The following macro is defined in newer xdaq versions
#ifndef XCEPT_DEFINE_EXCEPTION
#define XCEPT_DEFINE_EXCEPTION(NAMESPACE1, EXCEPTION_NAME) \
namespace NAMESPACE1 { \
namespace exception { \
class EXCEPTION_NAME: public xcept::Exception \
{\
        public: \
        EXCEPTION_NAME( std::string name, std::string message, std::string module, int line, std::string function ): \
                xcept::Exception(name, message, module, line, function) \
        {} \
        EXCEPTION_NAME( std::string name, std::string message, std::string module, int line, std::string function, xcept::Exception & e ): \
                xcept::Exception(name, message, module, line, function,e) \
        {} \
}; \
} \
}
#endif



/**
 * Generic exception raised by the storage manager
 */
XCEPT_DEFINE_EXCEPTION(stor, Exception)

/**
 * Exception raised in case of a SOAP error
 */
XCEPT_DEFINE_EXCEPTION(stor, SoapMessage)

/**
 * Exception raised in case of a finite state machine error
 */
XCEPT_DEFINE_EXCEPTION(stor, StateMachine)

/**
 * Exception raised in case of a monitoring error
 */
XCEPT_DEFINE_EXCEPTION(stor, Monitoring)

/**
 * Exception raised in case of problems accessing the info space
 */
XCEPT_DEFINE_EXCEPTION(stor, Infospace)

/**
 * Exception raised in case of missuse of I2OChain
 */
XCEPT_DEFINE_EXCEPTION(stor, I2OChain)

/**
 * Exception raised in case of asking for information from the wrong I2O message type
 */
XCEPT_DEFINE_EXCEPTION(stor, WrongI2OMessageType)

/**
 * Exception raised in case of requesting information from a faulty or incomplete init message
 */
XCEPT_DEFINE_EXCEPTION(stor, IncompleteInitMessage)

/**
 * Exception raised in case of requesting information from a faulty or incomplete event message
 */
XCEPT_DEFINE_EXCEPTION(stor, IncompleteEventMessage)

/**
 * Exception raised when the SM is unable to determine which resource
 * broker should received a discard message for a particular I2O message.
 */
XCEPT_DEFINE_EXCEPTION(stor, RBLookupFailed)

/**
 * Exception raised when a non-existant queue is requested.
 */
XCEPT_DEFINE_EXCEPTION(stor, UnknownQueueId)

/**
 * Exception raised when a non-existant stream is requested.
 */
XCEPT_DEFINE_EXCEPTION(stor, UnknownStreamId)

/**
 * Exception raised in case of a fragment processing error
 */
XCEPT_DEFINE_EXCEPTION(stor, FragmentProcessing)

/**
 * Exception raised in case of a disk writing error
 */
XCEPT_DEFINE_EXCEPTION(stor, DiskWriting)

/**
 * Consumer registration exception
 */
XCEPT_DEFINE_EXCEPTION( stor, ConsumerRegistration )

#endif // _storagemanager_Exception_h_


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
